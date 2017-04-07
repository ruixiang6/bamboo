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

#define SEN_RTC			(1u<<0)
#define SEN_BAT			(1u<<1)
#define SEN_TEMPER		(1u<<2)

#pragma pack(1)

typedef struct 
{
	uint16_t voltage;
	uint16_t current;	
	uint16_t temper;	
	uint8_t rsoc;
	uint16_t fcc;
} battery_info_t;

typedef struct
{
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t day;
	uint8_t date;
	uint8_t month;
	uint8_t year;
} rtc_info_t;

typedef struct
{
	fp32_t temper;
} temper_info_t;
#pragma pack()

void hal_sensor_init(uint8_t sensor_type);

bool_t hal_sensor_get_data(uint8_t sensor_type, void *p_data);

bool_t hal_sensor_set_data(uint8_t sensor_type, void *p_data);

#endif

