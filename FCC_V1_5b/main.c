#include <atmel_start.h>
#include "usart.h"
#include "subbus.h"
#include "control.h"
#include "spi.h"
#include "commands.h"

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
  subbus_reset();
	uart_init();
  spi_init();
  commands_init();
	while (1) {
    poll_control();
    poll_spi();
    poll_commands();
#if SUBBUS_INTERRUPTS
    if (subbus_intr_req)
      intr_service();
#endif
    // possibly check for watchdog features
	}
}
