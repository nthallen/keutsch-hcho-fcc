/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */

#include "driver_init.h"
#include <peripheral_clk_config.h>
#include <utils.h>
#include <hal_init.h>
#include <hpl_gclk_base.h>
#include <hpl_pm_base.h>
#include <hpl_rtc_base.h>

struct spi_m_async_descriptor SPI_ADC;

void SPI_ADC_PORT_init(void)
{

	// Set pin direction to input
	gpio_set_pin_direction(PA04, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(PA04,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(PA04, PINMUX_PA04D_SERCOM0_PAD0);

	// Set pin direction to output
	gpio_set_pin_direction(PA05, GPIO_DIRECTION_OUT);

	gpio_set_pin_level(PA05,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	gpio_set_pin_function(PA05, PINMUX_PA05D_SERCOM0_PAD1);

	// Set pin direction to output
	gpio_set_pin_direction(PA07, GPIO_DIRECTION_OUT);

	gpio_set_pin_level(PA07,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	gpio_set_pin_function(PA07, PINMUX_PA07D_SERCOM0_PAD3);
}

void SPI_ADC_CLOCK_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM0);
	_gclk_enable_channel(SERCOM0_GCLK_ID_CORE, CONF_GCLK_SERCOM0_CORE_SRC);
}

void SPI_ADC_init(void)
{
	SPI_ADC_CLOCK_init();
	spi_m_async_init(&SPI_ADC, SERCOM0);
	SPI_ADC_PORT_init();
}

void system_init(void)
{
	init_mcu();

	SPI_ADC_init();
	USART_CTRL_init();

	// TIMER_0_init();
}
