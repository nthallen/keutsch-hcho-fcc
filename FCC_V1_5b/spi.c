/************************************************************************/
/* \file spi.c                                                          */
/************************************************************************/
#include <peripheral_clk_config.h>
#include <hal_spi_m_async.h>
#include <hal_gpio.h>
#include <hpl_pm_base.h>
#include <hpl_gclk_base.h>
#include "atmel_start_pins.h"
#include "spi.h"
#include "subbus.h"

static struct spi_m_async_descriptor SPI_ADC;
static int SPI_ADC_txfr_complete = 0;
static struct io_descriptor *SPI_ADC_io;

static inline void chip_select(uint8_t pin) {
  gpio_set_pin_level(pin, false);
}
static inline void chip_deselect(uint8_t pin) {
  gpio_set_pin_level(pin, true);
}

static void SPI_ADC_PORT_init(void) {
  // PA07 SERCOM0 PAD[3] MISO
  gpio_set_pin_direction(PA07, GPIO_DIRECTION_IN);
  gpio_set_pin_pull_mode(PA07, GPIO_PULL_OFF);
  // <y> Pull configuration
  // <id> pad_pull_config
  // <GPIO_PULL_OFF"> Off
  // <GPIO_PULL_UP"> Pull-up
  // <GPIO_PULL_DOWN"> Pull-down
  gpio_set_pin_function(PA07, PINMUX_PA07D_SERCOM0_PAD3);

  // PA04 SERCOM0 PAD[0] MOSI
  gpio_set_pin_direction(PA04, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(PA04, false);
  gpio_set_pin_function(PA04, PINMUX_PA04D_SERCOM0_PAD0);

  // PA05 SERCOM0 PAD[1] SCK
  gpio_set_pin_direction(PA05, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(PA05, false);
  gpio_set_pin_function(PA05, PINMUX_PA05D_SERCOM0_PAD1);


  // PA02: CS0/
  gpio_set_pin_direction(CS0, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(CS0, true);
  gpio_set_pin_function(CS0, GPIO_PIN_FUNCTION_OFF);
  // PA06: CS1/
  gpio_set_pin_direction(CS1, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(CS1, true);
  gpio_set_pin_function(CS1, GPIO_PIN_FUNCTION_OFF);
  // PA03: DACSYNC/
  gpio_set_pin_direction(DACSYNC, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(DACSYNC, true);
  gpio_set_pin_function(DACSYNC, GPIO_PIN_FUNCTION_OFF);
}

static void SPI_ADC_CLOCK_init(void)
{
  _pm_enable_bus_clock(PM_BUS_APBC, SERCOM0);
  _gclk_enable_channel(SERCOM0_GCLK_ID_CORE, CONF_GCLK_SERCOM0_CORE_SRC);
}

static void complete_cb_SPI_ADC(const struct spi_m_async_descriptor *const io_descr) {
  SPI_ADC_txfr_complete = 1;
}

void spi_init(void) {
  SPI_ADC_CLOCK_init();
  spi_m_async_init(&SPI_ADC, SERCOM0);
  SPI_ADC_PORT_init();
	spi_m_async_get_io_descriptor(&SPI_ADC, &SPI_ADC_io);
	spi_m_async_register_callback(&SPI_ADC, SPI_M_ASYNC_CB_XFER, (FUNC_PTR)complete_cb_SPI_ADC);
	spi_m_async_enable(&SPI_ADC);
}

void poll_spi(void) {

}
