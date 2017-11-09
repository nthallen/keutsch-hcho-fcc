#include "subbus.h"
#include "control.h"

static uint16_t fail_reg = 0;

/************************************************************************/
/* Interrupt Handling                                                   */
/************************************************************************/
#if SUBBUS_INTERRUPTS

typedef struct {
  uint16_t address;
  uint16_t bitmask;
} board_intr_t;

static const board_intr_t bd_intrs[] = {
};

#define N_INTERRUPTS (sizeof(bd_intrs)/sizeof(board_intr_t))

typedef struct {
  uint16_t intr_id;
  int active;
} interrupt_t;

static interrupt_t interrupts[N_INTERRUPTS];

volatile uint8_t subbus_intr_req = 0;

void init_interrupts(void) {
  int i;
  for ( i = 0; i < N_INTERRUPTS; i++) {
    interrupts[i].active = 0;
  }
}

/**
 * Associates the interrupt at the specified base address with the
 * specified interrupt ID. Does not check to see if the interrupt
 * is already active, since that should be handled in the driver.
 * The only possible error is that the specified address is not
 * in our defined list, so ENOENT.
 * @return non-zero on success. Will return zero if board not listed in table,
 * or no acknowledge detected.
 */
int intr_attach(int id, uint16_t addr) {
  int i;
  for ( i = 0; i < N_INTERRUPTS; i++ ) {
    if ( bd_intrs[i].address == addr ) {
      interrupts[i].active = 1;
      interrupts[i].intr_id = id;
      return subbus_write( addr, 0x20 );
    }
  }
  return 0;
}

/**
 * @return non-zero on success. Will return zero if board is not listed in the table
 * or no acknowledge is detected.
 */
int intr_detach( uint16_t addr ) {
  int i;
  for ( i = 0; i < N_INTERRUPTS; i++ ) {
    if ( bd_intrs[i].address == addr ) {
      interrupts[i].active = 0;
      return subbus_write(addr, 0);
      return 1;
    }
  }
  return 0;
}

void intr_service(void) {
  uint16_t ivec;
  if ( subbus_read(SUBBUS_INTA_ADDR, &ivec) ) {
    int i;
    for ( i = 0; i < N_INTERRUPTS; i++ ) {
      if (ivec & bd_intrs[i].bitmask) {
        if (interrupts[i].active) {
          SendCodeVal('I', interrupts[i].intr_id);
          return;
        } // ### else could send an error, attempt to disable...
      }
    }
  } else SendErrorMsg("11"); // No ack on INTA
}
#endif

/************************************************************************/
/* Subbus Cache Functions                                               */
/************************************************************************/
static uint16_t subbus_cache[SUBBUS_CACHE_SIZE];
static uint16_t subbus_cache_wvalue[SUBBUS_CACHE_SIZE];
static bool subbus_cache_writable[SUBBUS_CACHE_SIZE];
static bool subbus_cache_written[SUBBUS_CACHE_SIZE];

static bool is_cache_addr(uint16_t addr, uint16_t *offset) {
  bool rv = false;
  if (addr >= SUBBUS_CACHE_BASE_ADDR) {
    if (addr < SUBBUS_CACHE_BASE_ADDR + SUBBUS_CACHE_SIZE) {
      *offset = addr-SUBBUS_CACHE_BASE_ADDR;
      rv = true;
    }
  }
  return rv;
}

/**
 * @param addr Subbus address
 * @param writable Indicate whether address is writable
 * @return non-zero on success
 */
bool subbus_cache_config(uint16_t addr, bool writable) {
  uint16_t offset;
  if (is_cache_addr(addr, &offset)) {
    subbus_cache_writable[offset] = writable;
    return true;
  }
  return false;
}

/**
 * If a value has been written to the specified address since the
 * last call to this function, the new value is written at the
 * value address.
 * @param addr The cache address
 * @param value Pointer where value may be written
 * @return true if a value has been written to this address.
 */
bool subbus_cache_iswritten(uint16_t addr, uint16_t *value) {
  uint16_t offset;
  if (is_cache_addr(addr, &offset) && subbus_cache_written[offset]) {
    *value = subbus_cache_wvalue[offset];
    subbus_cache_written[offset] = 0;
    return true;
  }
  return false;
}

/**
 * This function differs from subbus_cache_write() in that it directly
 * updates the cache value. subbus_cache_write() is specifically for
 * write originating from the control port. subbus_cache_update() is
 * used by internal functions for storing data acquired from
 * peripherals, or for storing values written from the control
 * port after verifying them.
 * @param addr The cache address
 * @param data The value to be written
 * @return true on success
 */
int subbus_cache_update(uint16_t addr, uint16_t data) {
  uint16_t offset;
  if (is_cache_addr(addr, &offset)) {
    subbus_cache[offset] = data;
    return 1;
  }
  return 0;
}

int subbus_cache_write(uint16_t addr, uint16_t data) {
  uint16_t offset;
  if (is_cache_addr(addr, &offset) && subbus_cache_writable[offset]) {
    subbus_cache_wvalue[offset] = data;
    subbus_cache_written[offset] = 1;
    return 1;
  }
  return 0;
}

int subbus_cache_read(uint16_t addr, uint16_t *data) {
  uint16_t offset;
  if (is_cache_addr(addr, &offset)) {
    *data = subbus_cache[offset];
    return 1;
  }
  return 0;
}

void subbus_reset(void) {
  int i;
  for (i = 0; i < SUBBUS_CACHE_SIZE; ++i) {
    subbus_cache[i] = 0;
    subbus_cache_wvalue[i] = 0;
    subbus_cache_writable[i] = 0;
    subbus_cache_written[i] = 0;
  }
}

/**
 * @return non-zero if EXPACK is detected.
 */
int subbus_read( uint16_t addr, uint16_t *rv ) {
  if (addr >= SUBBUS_CACHE_BASE_ADDR) {
    return subbus_cache_read(addr, rv);
  }
  switch (addr) {
    case 0:
    case SUBBUS_INTA_ADDR:
      return 0;
    case SUBBUS_BDID_ADDR:
      *rv = SUBBUS_BOARD_ID;
      return 1;
    case SUBBUS_BDID_ADDR+1:
      *rv = SUBBUS_BUILD_NUM;
      return 1;
    case SUBBUS_FAIL_ADDR:
      *rv = fail_reg;
      return 1;
    case SUBBUS_SWITCHES_ADDR:
      *rv = 0;
      return 1;
    default:
      return 0;
  }
}

int subbus_write( uint16_t addr, uint16_t data) {
  uint16_t offset;
  if (is_cache_addr(addr, &offset)) {
    return subbus_cache_write(addr, data);
  }
  return 0;
}

void set_fail(uint16_t arg) {
  fail_reg = (fail_reg & SUBBUS_FAIL_RESERVED) |
    (arg & ~SUBBUS_FAIL_RESERVED);
#if defined(SUBBUS_FAIL_ADDR)
  subbus_write(SUBBUS_FAIL_ADDR, fail_reg);
#endif
}

void set_fail_reserved(uint16_t arg) {
  fail_reg = (fail_reg & ~SUBBUS_FAIL_RESERVED) |
      (arg & SUBBUS_FAIL_RESERVED);
#if defined(SUBBUS_FAIL_ADDR)
  subbus_write(SUBBUS_FAIL_ADDR, fail_reg);
#endif
}
