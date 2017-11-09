#include "commands.h"
#include "subbus.h"
#include "spi.h"

void commands_init(void) {
  gpio_set_pin_direction(ADR0, GPIO_DIRECTION_IN);
  gpio_set_pin_pull_mode(ADR0, GPIO_PULL_OFF);
  gpio_set_pin_function(ADR0, GPIO_PIN_FUNCTION_OFF);

  gpio_set_pin_direction(ADR1, GPIO_DIRECTION_IN);
  gpio_set_pin_pull_mode(ADR1, GPIO_PULL_OFF);
  gpio_set_pin_function(ADR1, GPIO_PIN_FUNCTION_OFF);

  gpio_set_pin_direction(ADR2, GPIO_DIRECTION_IN);
  gpio_set_pin_pull_mode(ADR2, GPIO_PULL_OFF);
  gpio_set_pin_function(ADR2, GPIO_PIN_FUNCTION_OFF);

  gpio_set_pin_direction(CLOSE0, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(CLOSE0, false);
  gpio_set_pin_function(CLOSE0, GPIO_PIN_FUNCTION_OFF);

  gpio_set_pin_direction(OPEN0, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(OPEN0, false);
  gpio_set_pin_function(OPEN0, GPIO_PIN_FUNCTION_OFF);

  gpio_set_pin_direction(CLOSE1, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(CLOSE1, false);
  gpio_set_pin_function(CLOSE1, GPIO_PIN_FUNCTION_OFF);

  gpio_set_pin_direction(OPEN1, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(OPEN1, false);
  gpio_set_pin_function(OPEN1, GPIO_PIN_FUNCTION_OFF);

  gpio_set_pin_direction(CLOSE2, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(CLOSE2, false);
  gpio_set_pin_function(CLOSE2, GPIO_PIN_FUNCTION_OFF);

  gpio_set_pin_direction(OPEN2, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(OPEN2, false);
  gpio_set_pin_function(OPEN2, GPIO_PIN_FUNCTION_OFF);

  gpio_set_pin_direction(CLOSE3, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(CLOSE3, false);
  gpio_set_pin_function(CLOSE3, GPIO_PIN_FUNCTION_OFF);

  gpio_set_pin_direction(OPEN3, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(OPEN3, false);
  gpio_set_pin_function(OPEN3, GPIO_PIN_FUNCTION_OFF);

  gpio_set_pin_direction(LED, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(LED, true);
  gpio_set_pin_function(LED, GPIO_PIN_FUNCTION_OFF);

  subbus_cache_config(SUBBUS_ADDR_CMDS, true);
}

void poll_commands(void) {
  uint16_t cmd;
  if (subbus_cache_iswritten(SUBBUS_ADDR_CMDS, &cmd)) {
    switch (cmd) {
      case 32: spi_enable(true); break;
      case 33: spi_enable(false); break;
      default: break;
    }
  }
}