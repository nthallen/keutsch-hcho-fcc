#ifndef SUBBUS_H_INCLUDED
#define SUBBUS_H_INCLUDED

#include <stdint.h>

#define BOARD_REV                   "V10:178:HCHO FCC Rev A V1.0"
#define SUBBUS_FAIL_RESERVED        0xF000
#define SUBBUS_INTA_ADDR            0x0001
#define SUBBUS_BDID_ADDR            0x0002
#define SUBBUS_FAIL_ADDR            0x0004
#define SUBBUS_SWITCHES_ADDR        0x0005

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

#endif
