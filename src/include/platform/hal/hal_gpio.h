#ifndef __HAL_GPIO_H
#define __HAL_GPIO_H

void hal_gpio_init(bool_t init_flag);

void hal_gpio_output(uint8_t pin, bool_t state);

bool_t hal_gpio_input(uint8_t pin);


#endif
