#include <platform.h>
#include "Control_IO.h"

void ControlIO_Init(void)
{
	hal_gpio_output(GPIO_BD, 1);	
	hal_gpio_output(GPIO_ETH, 1);
}