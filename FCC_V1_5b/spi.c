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
static volatile bool SPI_ADC_txfr_complete = false;
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

static void SPI_ADC_CLOCK_init(void) {
  _pm_enable_bus_clock(PM_BUS_APBC, SERCOM0);
  _gclk_enable_channel(SERCOM0_GCLK_ID_CORE, CONF_GCLK_SERCOM0_CORE_SRC);
}

static void complete_cb_SPI_ADC(const struct spi_m_async_descriptor *const io_descr) {
  SPI_ADC_txfr_complete = true;
}

void spi_init(void) {
  SPI_ADC_CLOCK_init();
  spi_m_async_init(&SPI_ADC, SERCOM0);
  SPI_ADC_PORT_init();
	spi_m_async_get_io_descriptor(&SPI_ADC, &SPI_ADC_io);
	spi_m_async_register_callback(&SPI_ADC, SPI_M_ASYNC_CB_XFER, (FUNC_PTR)complete_cb_SPI_ADC);
	spi_m_async_enable(&SPI_ADC);
}

static uint16_t spi_read_data;
static uint16_t spi_write_data;

static void start_spi_transfer(uint8_t pin, uint16_t value) {
  chip_select(pin);
  spi_write_data = value;
  SPI_ADC_txfr_complete = false;
  spi_m_async_transfer(&SPI_ADC, (uint8_t const *)&spi_write_data,
        (uint8_t *const)&spi_read_data, 2);
}

enum cs0_state_t {cs0_init, cs0_init_tx, cs0_wana0, cs0_ana0_tx, cs0_wana1, cs0_ana1_tx};
static enum cs0_state_t cs0_state = cs0_init;
#define CONVERT_AIN0 0x818Du
#define CONVERT_AIN2 0xD18Du
#define CONVERT_TEMP 0x819Du
#define MISO PA07

static void poll_cs0(void) {
  switch (cs0_state) {
    case cs0_init:
      // initiate write to cs0 to convert ana0
      start_spi_transfer(CS0, CONVERT_AIN0);
      cs0_state = cs0_init_tx;
      break;
    case cs0_init_tx:
      chip_deselect(CS0);
      cs0_state = cs0_wana0;
      break;
    case cs0_wana0:
      chip_select(CS0);
      if (gpio_get_pin_level(MISO)) {
        chip_deselect(CS0);
        return;
      }
      start_spi_transfer(CS0, CONVERT_AIN2);
      cs0_state = cs0_ana0_tx;
      break;
    case cs0_ana0_tx:
      chip_deselect(CS0);
      subbus_cache_write(0x10, spi_read_data);
      cs0_state = cs0_wana1;
      break;
    case cs0_wana1:
      chip_select(CS0);
      if (gpio_get_pin_level(MISO)) {
        chip_deselect(CS0);
        return;
      }
      start_spi_transfer(CS0, CONVERT_AIN0);
      cs0_state = cs0_ana1_tx;
      break;
    case cs0_ana1_tx:
      chip_deselect(CS0);
      subbus_cache_write(0x11, spi_read_data);
      cs0_state = cs0_wana0;
      break;
    default:
      assert(false, __FILE__, __LINE__);
  }
}

static void poll_cs1(void) {}
static void poll_dac(void) {}

enum spi_state_t {spi_cs0, spi_cs1, spi_dac };
static enum spi_state_t spi_state = spi_cs0;

void poll_spi(void) {
  enum spi_state_t input_state = spi_state;
  while (SPI_ADC_txfr_complete) {
    switch (spi_state) {
      case spi_cs0:
        poll_cs0();
        if (SPI_ADC_txfr_complete) {
          spi_state = spi_cs1;
        }
        break;
      case spi_cs1:
        poll_cs1();
        if (SPI_ADC_txfr_complete) {
          spi_state = spi_dac;
        }
        break;
      case spi_dac:
        poll_dac();
        if (SPI_ADC_txfr_complete) {
          spi_state = spi_cs0;
        }
        break;
      default:
        assert(false, __FILE__, __LINE__);
    }
    if (spi_state == input_state) break;
  }
}
