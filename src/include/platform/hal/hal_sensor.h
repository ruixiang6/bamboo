 /**
 * provides an abstraction for hardware.
 *
 * @file hal_sensor.h
 * @author qhw
 *
 * @addtogroup HAL_SENSOR
 * @ingroup HAL
 * @{
 */

#ifndef __HAL_SENSOR_H
#define __HAL_SENSOR_H

#define SENSOR_COMPASS			(1u<<0)
#define SENSOR_BATTERY			(1u<<1)
#define SENSOR_BATTERY_ALL		(1u<<2)
#define SENSOR_BATTERY_INIT		(1u<<3)

#define BATTERY_QUANTITY		2000		//mAh

typedef struct 
{
	uint16_t voltage;
	uint16_t current;
	uint16_t temper;
	uint16_t quantity;
} battery_info_t;

void hal_sensor_init(uint8_t sensor_type);
bool_t hal_sensor_get_data(uint8_t sensor_type, void *p_data);
uint8_t hal_sensor_calib(uint8_t sensor_type);

#endif

