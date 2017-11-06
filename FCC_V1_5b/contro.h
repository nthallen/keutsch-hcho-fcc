#ifndef CONTROL_H_INCLUDED
#define CONTROL_H_INCLUDED
#include <stdint.h>

void SendMsg(uint8_t *);			// Send String Back to Host via USB
void SendCode(uint8_t code);
void SendCodeVal(uint8_t, uint16_t);
void SendErrorMsg(uint8_t *code);

#endif