#include <platform.h>
#include <mss_gpio.h>

void hal_gpio_init(bool_t init_flag)
{
	if (init_flag)
	{
		MSS_GPIO_init();
	}
	
	//POWER
	MSS_GPIO_config(GPIO_RF_MODULE, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(GPIO_RF_PA, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(GPIO_BD, MSS_GPIO_OUTPUT_MODE);	
	MSS_GPIO_config(GPIO_ETH, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(GPIO_USB_RST, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(GPIO_LED, MSS_GPIO_OUTPUT_MODE);

	if (init_flag)
	{
		//default pin state
		MSS_GPIO_set_outputs(0);
	}
}

void hal_gpio_output(uint8_t pin, bool_t state)
{
	MSS_GPIO_set_output(pin, state);	
}

bool_t hal_gpio_input(uint8_t pin)
{
	if ((MSS_GPIO_get_inputs()&(1u<<pin)))
	{
		return PLAT_TRUE;
	}
	else
	{
		return PLAT_FALSE;
	}
}
