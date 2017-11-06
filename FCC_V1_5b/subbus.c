#include "subbus.h"
#include "control.h"

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

static uint16_t fail_reg = 0;
volatile uint8_t subbus_intr_req = 0;

void subbus_reset(void) {}

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

/**
 * @return non-zero if EXPACK is detected.
 */
int subbus_read( uint16_t addr, uint16_t *rv ) {
  int expack = 1;
  return expack;
}

int subbus_write( uint16_t addr, uint16_t data) {
  int expack = 1;
  return expack;
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
