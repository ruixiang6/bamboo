#include <platform.h>
#include <mss_i2c.h>
#include <math.h>

#define BQ4050_ADDRRESS 		0x16>>1
#define RTC_ADDRESS 			0xde>>1
#define LM75A_ADDRESS 			0x90>>1

#define BQ4050_REG_TEMP			0x08		//Temperature   U2
#define BQ4050_REG_VOLT			0x09		//Voltage   U2
#define BQ4050_REG_CURRENT		0x0A		//Voltage   I2
#define BQ4050_REG_RSOC			0x0D		//RelativeStateOfCharge U1
#define BQ4050_REG_FCC			0x10		//FullChargeCapacity    U2
#define BQ4050_REG_TTE			0x12		//TimeToEmpty   U2
#define BQ4050_REG_TTF			0x13		//TimeToFull    U2

#define RTC_REG_SECOND			0x00		//
#define RTC_REG_MINUTE			0x01		//
#define RTC_REG_HOUR			0x02		//
#define RTC_REG_DAY			    0x03		//
#define RTC_REG_DATE			0x04		//
#define RTC_REG_MONTH			0x05		//
#define RTC_REG_YEAR			0x06		//
#define RTC_REG_CTRL			0x07		//


static uint8_t i2c_address = 0;
static void I2C_Config(uint8_t I2C_ADDRESS);
static bool_t I2C_WriteByte(uint8_t addr, uint8_t value);
static bool_t I2C_WriteMultiByte(uint8_t addr, uint8_t *buffer, uint8_t len);
static bool_t I2C_ReadByte(uint8_t addr, uint8_t *value);
static bool_t I2C_ReadMultiByte(uint8_t addr, uint8_t *read_buffer, uint8_t len);

void hal_sensor_init(uint8_t sensor_type)
{
	MSS_I2C_init(&g_mss_i2c0, i2c_address, MSS_I2C_PCLK_DIV_256);	
}

bool_t hal_sensor_get_data(uint8_t sensor_type, void *p_data)
{
	rtc_info_t *p_rtc_info = PLAT_NULL;
	battery_info_t *p_bat_info = PLAT_NULL;
	temper_info_t *p_temper_info = PLAT_NULL;
	
	if (p_data == PLAT_NULL) return PLAT_FALSE;
	
	switch(sensor_type)
	{
		case SEN_RTC:			
        	p_rtc_info = (rtc_info_t*)p_data;
        	I2C_Config(RTC_ADDRESS);
			if (!I2C_ReadByte(RTC_REG_SECOND, &p_rtc_info->second))
			{
				return PLAT_FALSE;
			}
			p_rtc_info->second = ((p_rtc_info->second>>4)&0x3)*10+(p_rtc_info->second&0xf);
			if (!I2C_ReadByte(RTC_REG_MINUTE, &p_rtc_info->minute))
	        {
	            return PLAT_FALSE;
	        }
	        p_rtc_info->minute = ((p_rtc_info->minute>>4)&0x3)*10+(p_rtc_info->minute&0xf);

			if (!I2C_ReadByte(RTC_REG_HOUR, &p_rtc_info->hour))
	        {
	            return PLAT_FALSE;
	        }
	        p_rtc_info->hour = ((p_rtc_info->hour>>4)&0x3)*10+(p_rtc_info->hour&0xf);

			if (!I2C_ReadByte(RTC_REG_DATE, &p_rtc_info->date))
	        {
	            return PLAT_FALSE;
	        }
	        p_rtc_info->date = ((p_rtc_info->date>>4)&0x3)*10+(p_rtc_info->date&0xf);

			if (!I2C_ReadByte(RTC_REG_MONTH, &p_rtc_info->month))
	        {
	            return PLAT_FALSE;
	        }
	        p_rtc_info->month = ((p_rtc_info->month>>4)&0x3)*10+(p_rtc_info->month&0xf);
			if (!I2C_ReadByte(RTC_REG_YEAR, &p_rtc_info->year))
	        {
	            return PLAT_FALSE;
	        }
	        p_rtc_info->year = ((p_rtc_info->year>>4)&0x3)*10+(p_rtc_info->year&0xf);
			break;
		case SEN_BAT:
			p_bat_info = (battery_info_t*)p_data;			
	        I2C_Config(BQ4050_ADDRRESS);
	        if (!I2C_ReadByte(BQ4050_REG_RSOC, &p_bat_info->rsoc))
	        {
	            return PLAT_FALSE;
	        }
			
	        if (!I2C_ReadMultiByte(BQ4050_REG_TEMP,(uint8_t *)&p_bat_info->temper, 2))
	        {
	            return PLAT_FALSE;
	        }
			
	        if (!I2C_ReadMultiByte(BQ4050_REG_VOLT,(uint8_t *)&p_bat_info->voltage, 2))
	        {
	            return PLAT_FALSE;
	        }
			
	        if (!I2C_ReadMultiByte(BQ4050_REG_CURRENT,(uint8_t *)&p_bat_info->current, 2))
	        {
	            return PLAT_FALSE;
	        }
			
	        if(p_bat_info->current & 0x8000)
	        {
	            p_bat_info->current = (~p_bat_info->current) + 1;
	        }
			
	        if (!I2C_ReadMultiByte(BQ4050_REG_FCC,(uint8_t *)&p_bat_info->fcc, 2))
	        {
	            return PLAT_FALSE;
	        }
			break;
		case SEN_TEMPER:
			p_temper_info = (temper_info_t*)p_data;
			uint16_t value;

	        I2C_Config(LM75A_ADDRESS);

	        if (!I2C_ReadMultiByte(0x0,(uint8_t *)&value, 2))
	        {
	            return PLAT_FALSE;
	        }
	        value = (value >> 8) | ((value & 0xff) << 8);
	        p_temper_info->temper = (value>>5)*0.125;
			break;
		default:return PLAT_FALSE;
	}

	return PLAT_TRUE;
}

bool_t hal_sensor_set_data(uint8_t sensor_type, void *p_data)
{
	rtc_info_t *p_rtc_info = PLAT_NULL;
	uint8_t remain, multi;
	
	if (p_data == PLAT_NULL) return PLAT_FALSE;

	switch(sensor_type)
	{
		case SEN_RTC:
        	p_rtc_info = (rtc_info_t*)p_data;
			if (p_rtc_info->second>60 ||
				p_rtc_info->minute>60 ||
				p_rtc_info->hour>24 ||
				p_rtc_info->date>31 ||
				p_rtc_info->month>12 ||
				p_rtc_info->year> 99 )
			{
				return PLAT_FALSE;
			}
			I2C_Config(RTC_ADDRESS);

	        I2C_WriteByte(RTC_REG_SECOND,0x00); //stop
	        multi = p_rtc_info->minute/10;
			remain = p_rtc_info->minute%10;
	        I2C_WriteByte(RTC_REG_MINUTE, multi<<4|remain);
			multi = p_rtc_info->hour/10;
			remain = p_rtc_info->hour%10;
	        I2C_WriteByte(RTC_REG_HOUR, multi<<4|remain);
			multi = p_rtc_info->date/10;
			remain = p_rtc_info->date%10;
	        I2C_WriteByte(RTC_REG_DATE, multi<<4|remain);
			multi = p_rtc_info->month/10;
			remain = p_rtc_info->month%10;
	        I2C_WriteByte(RTC_REG_MONTH, multi<<4|remain);
			multi = p_rtc_info->year/10;
			remain = p_rtc_info->year%10;
	        I2C_WriteByte(RTC_REG_YEAR, multi<<4|remain);
			multi = p_rtc_info->second/10;
			remain = p_rtc_info->second%10;
	        I2C_WriteByte(RTC_REG_SECOND,0x80|multi<<4|remain); //start
	        return PLAT_TRUE;
		case SEN_BAT:
		case SEN_TEMPER:
		default:
			return PLAT_FALSE;
	}

	
}

static void I2C_Config(uint8_t I2C_ADDRESS)
{
	i2c_address = I2C_ADDRESS;
}

static bool_t I2C_WriteByte(uint8_t addr, uint8_t value)
{
	uint8_t write_buffer[2];
	uint32_t tmp_cnt = 0;

	write_buffer[0] = addr;
	write_buffer[1] = value;
	MSS_I2C_write(&g_mss_i2c0, i2c_address, write_buffer, 2, MSS_I2C_RELEASE_BUS);
	while (g_mss_i2c0.master_status == MSS_I2C_IN_PROGRESS)
	{
		tmp_cnt++;
		if (tmp_cnt>0x1FF) break;
	}
	if (g_mss_i2c0.master_status == MSS_I2C_SUCCESS) return PLAT_TRUE;
	else return PLAT_FALSE;
}

static bool_t I2C_WriteMultiByte(uint8_t addr, uint8_t *buffer, uint8_t len)
{
	uint8_t write_buffer[129];
	uint32_t tmp_cnt = 0;
	
	if (len>128) return PLAT_FALSE;

	write_buffer[0] = addr;

	for (uint8_t i=0; i<len; i++)
	{
		write_buffer[i+1] = buffer[i];
	}

	MSS_I2C_write(&g_mss_i2c0, i2c_address, write_buffer, len+1, MSS_I2C_RELEASE_BUS);

	while (g_mss_i2c0.master_status == MSS_I2C_IN_PROGRESS)
	{
		tmp_cnt++;
		if (tmp_cnt>0x1FF) break;
	}
	if (g_mss_i2c0.master_status == MSS_I2C_SUCCESS) return PLAT_TRUE;
	else return PLAT_FALSE;
}

static bool_t I2C_ReadByte(uint8_t addr, uint8_t *value)
{
	uint32_t tmp_cnt = 0;
	
	MSS_I2C_write_read(&g_mss_i2c0, i2c_address, 
						&addr, 1, 
						value, 1,
						MSS_I2C_RELEASE_BUS);
	
	while (g_mss_i2c0.master_status == MSS_I2C_IN_PROGRESS)
	{
		tmp_cnt++;
		if (tmp_cnt>0x1FF) break;
	}
	if (g_mss_i2c0.master_status == MSS_I2C_SUCCESS) return PLAT_TRUE;
	else return PLAT_FALSE;
}

static bool_t I2C_ReadMultiByte(uint8_t addr, uint8_t *read_buffer, uint8_t len)
{
	uint32_t tmp_cnt = 0;
	
	MSS_I2C_write_read(&g_mss_i2c0, i2c_address, 
						&addr, 1, 
						read_buffer, len,
						MSS_I2C_RELEASE_BUS);
	
	while (g_mss_i2c0.master_status == MSS_I2C_IN_PROGRESS)
	{
		tmp_cnt++;
		if (tmp_cnt>0x1FF) break;
	}
	if (g_mss_i2c0.master_status == MSS_I2C_SUCCESS) return PLAT_TRUE;
	else return PLAT_FALSE;
}

