#include <atmel_start.h>
#include "usart.h"
#include "subbus.h"
#include "control.h"


int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	uart_init();
  subbus_reset();
	while (1) {
    poll_control();
    // poll_spi();
    if (subbus_intr_req)
      intr_service();
    // possibly check for watchdog features
	}
}
