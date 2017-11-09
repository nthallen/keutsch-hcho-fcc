#ifndef SUBBUS_H_INCLUDED
#define SUBBUS_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>

#define BOARD_REV                   "V10:178:HCHO FCC Rev A V1.1"
#define SUBBUS_BOARD_ID             10
#define SUBBUS_BUILD_NUM            1
#define SUBBUS_FAIL_RESERVED        0xF000
#define SUBBUS_INTA_ADDR            0x0001
#define SUBBUS_BDID_ADDR            0x0002
#define SUBBUS_FAIL_ADDR            0x0004
#define SUBBUS_SWITCHES_ADDR        0x0005
#define SUBBUS_CACHE_BASE_ADDR      ((uint16_t)0x10)
#define SUBBUS_CACHE_SIZE           ((uint16_t)9)
#define SUBBUS_INTERRUPTS           0

#define SUBBUS_ADDR_CMDS 0x18

extern volatile uint8_t subbus_intr_req;
void subbus_reset(void);
void init_interrupts(void);
int intr_attach(int id, uint16_t addr);
int intr_detach( uint16_t addr );
void intr_service(void);
int subbus_read( uint16_t addr, uint16_t *rv );
int subbus_write( uint16_t addr, uint16_t data);
void set_fail(uint16_t arg);
void set_fail_reserved(uint16_t arg);
int subbus_cache_write(uint16_t addr, uint16_t data);
int subbus_cache_update(uint16_t addr, uint16_t data);
int subbus_cache_read(uint16_t addr, uint16_t *data);
bool subbus_cache_config(uint16_t addr, bool writable);
bool subbus_cache_iswritten(uint16_t addr, uint16_t *value);

#endif
