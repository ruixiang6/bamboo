#ifndef __HAL_GPIO_H
#define __HAL_GPIO_H

#define GPIO_RF_MODULE		24
#define GPIO_RF_PA			25
#define GPIO_BD				26
#define GPIO_ETH			27
#define GPIO_USB_RST		28
#define GPIO_LED			23

void hal_gpio_init(bool_t init_flag);

void hal_gpio_output(uint8_t pin, bool_t state);

bool_t hal_gpio_input(uint8_t pin);


#endif
