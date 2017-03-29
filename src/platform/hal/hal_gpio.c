#include <platform.h>
#include <mss_gpio.h>

void hal_gpio_init(bool_t init_flag)
{
	if (init_flag)
	{
		MSS_GPIO_init();
	}
	
	//GPIO
	MSS_GPIO_config(MSS_GPIO_0, MSS_GPIO_INPUT_MODE);//version0
	MSS_GPIO_config(MSS_GPIO_1, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_2, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_3, MSS_GPIO_INPUT_MODE); //reserve
	MSS_GPIO_config(MSS_GPIO_4, MSS_GPIO_OUTPUT_MODE);//reserve
	MSS_GPIO_config(MSS_GPIO_5, MSS_GPIO_INPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_6, MSS_GPIO_OUTPUT_MODE);//reserve
	MSS_GPIO_config(MSS_GPIO_7, MSS_GPIO_INPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_8, MSS_GPIO_INPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_9, MSS_GPIO_INPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_10, MSS_GPIO_INPUT_MODE);//version1
	MSS_GPIO_config(MSS_GPIO_11, MSS_GPIO_INPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_12, MSS_GPIO_INPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_13, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_14, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_15, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_16, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_17, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_18, MSS_GPIO_INPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_19, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_20, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_21, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_22, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_23, MSS_GPIO_OUTPUT_MODE);//reserve
	MSS_GPIO_config(MSS_GPIO_24, MSS_GPIO_INPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_25, MSS_GPIO_INPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_26, MSS_GPIO_INPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_27, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_28, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_29, MSS_GPIO_INPUT_MODE);//version2
	MSS_GPIO_config(MSS_GPIO_30, MSS_GPIO_INPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_31, MSS_GPIO_OUTPUT_MODE);
	
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
