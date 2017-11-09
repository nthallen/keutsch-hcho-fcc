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
static volatile bool SPI_ADC_txfr_complete = true;
static struct io_descriptor *SPI_ADC_io;
static bool spi_enabled = false;

void spi_enable(bool value) {
  spi_enabled = value;
}

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

static uint8_t spi_read_data[MAX_SPI_READ_LENGTH];

static void start_spi_transfer(uint8_t pin, uint8_t const *txbuf, int length) {
  assert(length <= MAX_SPI_READ_LENGTH,__FILE__,__LINE__);
  chip_select(pin);
  SPI_ADC_txfr_complete = false;
  spi_m_async_transfer(&SPI_ADC, txbuf, spi_read_data, length);
}

/* The bytes need to be swapped on output, since the driver transmits LSB first */
static uint8_t CONVERT_AIN0[2] = { 0x81, 0x8D };
static uint8_t CONVERT_AIN2[2] = { 0xD1, 0x8D };
static uint8_t CONVERT_TEMP[2] = { 0x81, 0x9D };
#define MISO PA07

enum adc_state_t {adc_init, adc_init_tx,
           adc_ain0_wait, adc_ain0_tx,
           adc_ain2_wait, adc_ain2_tx,
           adc_temp_wait, adc_temp_tx};
typedef struct {
  enum adc_state_t state;
  uint8_t cs_pin;
  uint16_t AIN0_addr;
  uint16_t AIN2_addr;
  uint16_t TEMP_addr;
} adc_poll_def;

static adc_poll_def adc_u2 = {adc_init, CS0, 0x10, 0x11, 0x19};
static adc_poll_def adc_u3 = {adc_init, CS1, 0x12, 0x13, 0x1A};

/**
 * poll_adc() is only called when SPI_ADC_txfr_complete is non-zero 
 * @return true if we are relinquishing the SPI bus
 */
static bool poll_adc(adc_poll_def *adc) {
  uint16_t value;
  switch (adc->state) {
    case adc_init:
      start_spi_transfer(adc->cs_pin, CONVERT_AIN0, 2);
      adc->state = adc_init_tx;
      return false;
    case adc_init_tx:
      chip_deselect(adc->cs_pin);
      adc->state = adc_ain0_wait;
      return true;
    case adc_ain0_wait:
      chip_select(adc->cs_pin);
      if (gpio_get_pin_level(MISO)) {
        chip_deselect(adc->cs_pin);
        return true;
      }
      start_spi_transfer(adc->cs_pin, CONVERT_AIN2, 2);
      adc->state = adc_ain0_tx;
      return false;
    case adc_ain0_tx:
      chip_deselect(adc->cs_pin);
      value = (spi_read_data[0] << 8) + spi_read_data[1];
      subbus_cache_update(adc->AIN0_addr, value);
      adc->state = adc_ain2_wait;
      return true;
    case adc_ain2_wait:
      chip_select(adc->cs_pin);
      if (gpio_get_pin_level(MISO)) {
        chip_deselect(adc->cs_pin);
        return true;
      }
      start_spi_transfer(adc->cs_pin, CONVERT_TEMP, 2);
      adc->state = adc_ain2_tx;
      return false;
    case adc_ain2_tx:
      chip_deselect(adc->cs_pin);
      value = (spi_read_data[0] << 8) + spi_read_data[1];
      subbus_cache_update(adc->AIN2_addr, value);
      adc->state = adc_temp_wait;
      return true;
    case adc_temp_wait:
      chip_select(adc->cs_pin);
      if (gpio_get_pin_level(MISO)) {
        chip_deselect(adc->cs_pin);
        return true;
      }
      start_spi_transfer(adc->cs_pin, CONVERT_AIN0, 2);
      adc->state = adc_temp_tx;
      return false;
    case adc_temp_tx:
      chip_deselect(adc->cs_pin);
      value = (spi_read_data[0] << 8) + spi_read_data[1];
      subbus_cache_update(adc->TEMP_addr, value);
      adc->state = adc_temp_wait;
      return true;
    default:
      assert(false, __FILE__, __LINE__);
  }
  return true;
}

enum dac_state_t {dac_init, dac_tx, dac_idle};
typedef struct {
  enum dac_state_t state;
  uint8_t cs_pin;
  uint16_t addr[4];
} dac_poll_def;

static dac_poll_def dac_u5 = {dac_init, DACSYNC, {0x14, 0x15, 0x16, 0x17}};

static bool poll_dac(void) {
  return true;
}

enum spi_state_t {spi_adc_u2, spi_adc_u3, spi_dac };
static enum spi_state_t spi_state = spi_adc_u2;

void poll_spi(void) {
  enum spi_state_t input_state = spi_state;
  if (!spi_enabled) return;
  while (SPI_ADC_txfr_complete) {
    switch (spi_state) {
      case spi_adc_u2:
        if (poll_adc(&adc_u2)) {
          spi_state = spi_adc_u3;
        }
        break;
      case spi_adc_u3:
        if (poll_adc(&adc_u3)) {
          spi_state = spi_dac;
        }
        break;
      case spi_dac:
        if (poll_dac()) {
          spi_state = spi_adc_u2;
        }
        break;
      default:
        assert(false, __FILE__, __LINE__);
    }
    if (spi_state == input_state) break;
  }
}

void spi_init(void) {
  SPI_ADC_CLOCK_init();
  spi_m_async_init(&SPI_ADC, SERCOM0);
  SPI_ADC_PORT_init();
  spi_m_async_get_io_descriptor(&SPI_ADC, &SPI_ADC_io);
  spi_m_async_register_callback(&SPI_ADC, SPI_M_ASYNC_CB_XFER, (FUNC_PTR)complete_cb_SPI_ADC);
  spi_m_async_enable(&SPI_ADC);
  subbus_cache_config(dac_u5.addr[0], true);
  subbus_cache_config(dac_u5.addr[1], true);
  subbus_cache_config(dac_u5.addr[2], true);
  subbus_cache_config(dac_u5.addr[3], true);
}
