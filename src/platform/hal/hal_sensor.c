#include <platform.h>
#include <mss_i2c.h>
#include <math.h>

#define HMC5883_ADDRESS 0x3C>>1
#define ADXL345_ADDRESS 0xA7>>1
#define BQ27510_ADDRRESS 0xAA>>1
#define LTC2943_ADDRESS 0XC8>>1

#define BQ27510_REG_AR			0X02		//AtRate
#define BQ27510_REG_ARTTE		0X04		//AtRateTimeToEmpty
#define BQ27510_REG_TEMP		0X06		//Temperature
#define BQ27510_REG_VOLT		0X08		//Voltage
#define BQ27510_REG_FLAGS		0X0A		//Flags
#define BQ27510_REG_NAC			0X0C		//NominalAvailableCapacity
#define BQ27510_REG_FAC			0X0E		//FullAvailableCapacity
#define BQ27510_REG_RM			0X10		//RemainingCapacity
#define BQ27510_REG_FCC			0X12		//FullChargeCapacity
#define BQ27510_REG_AI			0X14		//AverageCurrent
#define BQ27510_REG_TTE			0X16		//TimeToEmpty
#define BQ27510_REG_TTF			0X18		//TimeToFull
#define BQ27510_REG_SI			0X1A		//StandbyCurrent
#define BQ27510_REG_STTF		0X1C		//StandbyTimeToEmpty
#define BQ27510_REG_MLI			0X1E		//MaxLoadCurrent
#define BQ27510_REG_MLTTE		0X20		//MaxLoadTimeToEmpty
#define BQ27510_REG_AE			0X22		//AvailableEnergy
#define BQ27510_REG_AP			0X24		//AveragePower
#define BQ27510_REG_TTECP		0X26		//TTEatConstantPower
#define BQ27510_REG_CYCT		0X2A		//CycleCount
#define BQ27510_REG_SOC			0X2C		//StateOfCharge
#define BQ27510_REG_DCAP		0X3C

#define LTC2943_REG_STAT		0X00		//STATUS
#define LTC2943_REG_CTRL		0X01		//Control
#define LTC2943_REG_ACHG          0X02		       //Accumulated charge :2
#define LTC2943_REG_CHGTH		0X04		//Charge threshold high :2
#define LTC2943_REG_CHGTL		0X06		//Charge threshold low :2
#define LTC2943_REG_VOLT		0X08		//Voltage :2
#define LTC2943_REG_VOLTH		0X0a		//Voltage threshold high :2
#define LTC2943_REG_VOLTL		0X0c		//Voltage threshold low :2
#define LTC2943_REG_CURT		0X0e		//Current :2
#define LTC2943_REG_CURTH		0X10		//Current threshold high :2
#define LTC2943_REG_CURTL		0X12		//Current threshold low :2
#define LTC2943_REG_TEMP	       0X14		//Temperature :2
#define LTC2943_REG_TEMPT		0X16            //Temperature threshold :2

#define PI 3.1415926535897932f

static struct 
{
	fp32_t Xcal;//-2;
	fp32_t Ycal;//-34;
	fp32_t Zcal;
	fp32_t xGain;
	fp32_t yGain;
	fp32_t zGain;
} CompassCalParam;

typedef struct
{
	int16_t x; /**< holds x-axis acceleration data sign extended. Range -512 to 511. */
	int16_t y; /**< holds y-axis acceleration data sign extended. Range -512 to 511. */
	int16_t z; /**< holds z-axis acceleration data sign extended. Range -512 to 511. */
} HMC5883mag_t;


static uint8_t i2c_address = 0;
static void I2C_Config(uint8_t I2C_ADDRESS);
static bool_t I2C_WriteByte(uint8_t addr, uint8_t value);
static bool_t I2C_WriteMultiByte(uint8_t addr, uint8_t *buffer, uint8_t len);
static bool_t I2C_ReadByte(uint8_t addr, uint8_t *value);
static bool_t I2C_ReadMultiByte(uint8_t addr, uint8_t *read_buffer, uint8_t len);

static uint8_t GerHeading(uint16_t *pHeading);
static uint8_t fchol(fp64_t R[] );
static void Utsolve(fp64_t U[], fp64_t b[6]);
static void Usolve(fp64_t U[], fp64_t b[6]);
static void OutputResult(fp64_t X[6], fp32_t result[6]);
static void InitHMC5883ForCalib(void);
static uint8_t MgnCalibration(void);
static void write2Flash(fp32_t result[6]);
static void loadCalibPara(bool_t res);

void hal_sensor_init(uint8_t sensor_type)
{
	if ((sensor_type & SENSOR_COMPASS) || (sensor_type & SENSOR_BATTERY))
	{
		MSS_I2C_init(&g_mss_i2c0, i2c_address, MSS_I2C_PCLK_DIV_256);
		if (sensor_type & SENSOR_COMPASS)  loadCalibPara(1);
	}
}

bool_t hal_sensor_get_data(uint8_t sensor_type, void *p_data)
{
	battery_info_t *p_battery;
	uint8_t value8;
	uint16_t value16;
	bool_t result = PLAT_FALSE;
	
	if (sensor_type & SENSOR_COMPASS)
	{
		if (GerHeading((uint16_t *)p_data) == 0)
		{
			*(uint16_t *)p_data = 0;
		}
	}

	if (sensor_type & SENSOR_BATTERY)
	{
	    p_battery = (battery_info_t *)p_data;
#if 0   //V1.2
		I2C_Config(BQ27510_ADDRRESS);

		I2C_ReadMultiByte(BQ27510_REG_VOLT, (uint8_t *)&p_battery->voltage, 2);
		I2C_ReadMultiByte(BQ27510_REG_AI, (uint8_t *)&p_battery->current, 2);
		I2C_ReadMultiByte(BQ27510_REG_TEMP, (uint8_t *)&p_battery->temper, 2);
#else //v2.1       
        I2C_Config(LTC2943_ADDRESS);
        //delay_ms(1);
        if (!I2C_ReadByte(LTC2943_REG_CTRL, &value8))
        {        	
        	return PLAT_FALSE;
        }
        //test
        /*
        if((p_battery->ctrl & 0x01) == 0x0)    //shutdown
        {
            p_battery->ctrl |= 0x1;
            DBG_PRINTF("shutdown \r\n");
            I2C_WriteByte(LTC2943_REG_CTRL, p_battery->ctrl);
            delay_ms(70);
        }
        */
        if((value8 & 0xc0) == 0x0)  //sleep
        {
			value8 |= 0x40;
			//DBG_PRINTF("write convert single\r\n");
			if (!I2C_WriteByte(LTC2943_REG_CTRL, value8))
			{
				return PLAT_FALSE;
			}
			//delay_ms(70);
        }
        if (!I2C_ReadMultiByte(LTC2943_REG_ACHG, (uint8_t *)&value16, 2))
		{
			return PLAT_FALSE;
        }
		p_battery->quantity = (value16 >> 8) | ((value16 & 0xff) << 8);
        if (!I2C_ReadMultiByte(LTC2943_REG_VOLT, (uint8_t *)&value16, 2))
        {
        	return PLAT_FALSE;
        }
		p_battery->voltage = (value16 >> 8) | ((value16 & 0xff) << 8);      

		p_battery->voltage = (p_battery->voltage * 23) >> 6;
#endif
	}
	
	if (sensor_type & SENSOR_BATTERY_ALL)
	{
		p_battery = (battery_info_t *)p_data;
#if 0   //V1.2
		I2C_Config(BQ27510_ADDRRESS);

		I2C_ReadMultiByte(BQ27510_REG_VOLT, (uint8_t *)&p_battery->voltage, 2);
		I2C_ReadMultiByte(BQ27510_REG_AI, (uint8_t *)&p_battery->current, 2);
		I2C_ReadMultiByte(BQ27510_REG_TEMP, (uint8_t *)&p_battery->temper, 2);
#else //v2.1       
		I2C_Config(LTC2943_ADDRESS);
		if (!I2C_ReadByte(LTC2943_REG_CTRL, &value8))
        {        	
        	return PLAT_FALSE;
        }
		//test
		/*
		if((p_battery->ctrl & 0x01) == 0x0)    //shutdown
		{
			p_battery->ctrl |= 0x1;
			DBG_PRINTF("shutdown \r\n");
			I2C_WriteByte(LTC2943_REG_CTRL, p_battery->ctrl);
			delay_ms(70);
		}
		*/
		if((value8 & 0xc0) == 0x0)	//sleep
		{
			value8 |= 0x40;
			//DBG_PRINTF("write convert single\r\n");
			if (!I2C_WriteByte(LTC2943_REG_CTRL, value8))
			{
				return PLAT_FALSE;
			}
			//delay_ms(70);
		}
		if (!I2C_ReadMultiByte(LTC2943_REG_ACHG, (uint8_t *)&value16, 2))
		{
			return PLAT_FALSE;
		}
		p_battery->quantity = (value16 >> 8) | ((value16 & 0xff) << 8);
		if (!I2C_ReadMultiByte(LTC2943_REG_VOLT, (uint8_t *)&value16, 2))
		{
			return PLAT_FALSE;
		}
		p_battery->voltage = (value16 >> 8) | ((value16 & 0xff) << 8);		
		if (!I2C_ReadMultiByte(LTC2943_REG_CURT, (uint8_t *)&value16, 2))
		{
			return PLAT_FALSE;
		}
		p_battery->current = (value16 >> 8) | ((value16 & 0xff) << 8);
		if (!I2C_ReadMultiByte(LTC2943_REG_TEMP, (uint8_t *)&value16, 2))
		{
			return PLAT_FALSE;
		}
		p_battery->temper = (value16 >> 8) | ((value16 & 0xff) << 8);

		p_battery->voltage = (p_battery->voltage * 23) >> 6;

		if(p_battery->current < 0x7FFF) //discharge
		{
			 //DBG_PRINTF("dischg Current=%d mA\r\n", ((32767-cur_val) * 600) >> 15);
			 p_battery->current = ((0x7FFF - p_battery->current) * 600) >> 15;
		}
		else //charging
		{
			//DBG_PRINTF("chging Current=%d mA\r\n", ((cur_val-32767) * 600) >> 15);
			p_battery->current = ((p_battery->current-0x7FFF) * 600) >> 15;

		}
			 
		//DBG_PRINTF("Temper=%d 'C\r\n", (swbyte_uint16(bat.temper) >> 6) * 510 -273000-7400);
		p_battery->temper = (p_battery->temper >> 6)*510 - 273000 - 7400;
#endif
	}
    
	if (sensor_type & SENSOR_BATTERY_INIT)
	{
		p_battery = (battery_info_t *)p_data;
  
		I2C_Config(LTC2943_ADDRESS);
        //delay_ms(1);
		if (!I2C_ReadByte(LTC2943_REG_CTRL, &value8))
		{
			return PLAT_FALSE;
		}
		
		if((value8 & 0xc0) == 0x0)	//sleep
		{
			   value8 |= 0x40;			   
			   I2C_WriteByte(LTC2943_REG_CTRL, value8);
			   delay_ms(100);
		}
		if (!I2C_ReadMultiByte(LTC2943_REG_ACHG, (uint8_t *)&value16, 2))
		{
			return PLAT_FALSE;
		}
		p_battery->quantity = (value16 >> 8) | ((value16 & 0xff) << 8);
		if (!I2C_ReadMultiByte(LTC2943_REG_VOLT, (uint8_t *)&value16, 2))
		{
			return PLAT_FALSE;
		}
		p_battery->voltage = (value16 >> 8) | ((value16 & 0xff) << 8);		
		p_battery->voltage = (p_battery->voltage * 23) >> 6;
	}

	return PLAT_TRUE;
}

uint8_t hal_sensor_calib(uint8_t sensor_type)
{
	static uint16_t init_quantity = 0;
	static uint16_t cur_quantity = 0;
	battery_info_t bat;
	uint8_t value;
	fp32_t tmp;
	
	if (sensor_type & SENSOR_COMPASS)
	{
		return MgnCalibration();
	}
	else if (sensor_type & SENSOR_BATTERY_INIT)
	{
		//��ȡһ�ε�ص����ĵ�ѹֵ	
		if (!hal_sensor_get_data(SENSOR_BATTERY_INIT, &bat))
		{
			return 0;
		}
		DBG_TRACE("Bat Voltage=%dmV\r\n", bat.voltage);
		if (bat.voltage>8166) 
		{
			bat.voltage = 8166;
		}
		else if (bat.voltage<7056)
		{
			bat.voltage = 7056;
		}
		//��ʼֵ
		init_quantity = 1.8*bat.voltage - 12700;
		//��ǰ�궨ֵ
		cur_quantity = bat.quantity;
		DBG_TRACE("Bat Quantity=%dmAh\r\n", init_quantity);
	}
	else if (sensor_type & SENSOR_BATTERY)
	{
        //�����磬��������̬
        if (!hal_gpio_input(GPIO_CHARGE_DONE))
        {
            tmp = 10;
            return tmp;
        }
		//��ȡһ�ε�ص����ĵ�ѹֵ	
		if (!hal_sensor_get_data(SENSOR_BATTERY, &bat))
		{
			return 0;
		}
        //��������
		if (!hal_gpio_input(GPIO_CHARGE))
		{
			tmp = (bat.quantity - cur_quantity)*0.34;

			tmp = (init_quantity + tmp)/BATTERY_QUANTITY;			
		}
		else
		{
			tmp = (cur_quantity - bat.quantity)*0.34;

			tmp = (init_quantity - tmp)/BATTERY_QUANTITY;			
		}
		
		//DBG_TRACE("Qua=%d,Cur=%d,BatRate=%f\r\n", bat.quantity, cur_quantity, tmp);

		return tmp*10;
	}
    
    return 0;
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


/*******************************************************************************
* ��  ��  ��      : Init_HMC5883
* ��      ��      : ��ʼ��HMC5883L�Ŵ�����.
* ��      ��      : ��:
* ��      ��      : �����Ƿ�ɹ�
*******************************************************************************/
uint8_t Init_HMC5883(void)
{
	bool_t result = 0;

	I2C_Config(HMC5883_ADDRESS);
	result = I2C_WriteByte(0x00, 0x70);//ƽ����Ϊ8�Σ���������Ϊ15HZ
	if(PLAT_TRUE != result) return PLAT_FALSE;
	delay_ms(5);//��ʱ�ǳ��ؼ���ȥ����������ݲ���
	result = I2C_WriteByte(0x01, 0x80);//����4.7Ga
	delay_ms(5);//��ʱ�ǳ��ؼ���ȥ����������ݲ���
	result = I2C_WriteByte(0x02, 0x00); //��������ģʽ
	delay_ms(5);//��ʱ�ǳ��ؼ���ȥ����������ݲ���

	return PLAT_TRUE;
}

/*******************************************************************************
* ��  ��  ��      : HMC5883_Standby
* ��      ��      : HMC5883L������ʡ��.
* ��      ��      : ��:
* ��      ��      : �����Ƿ�ɹ�
*******************************************************************************/
uint8_t HMC5883_Standby(void)
{
	bool_t result = 0;
	I2C_Config(HMC5883_ADDRESS);
	result = I2C_WriteByte(0x02, 0x03);
	if(PLAT_TRUE != result) return PLAT_FALSE;
	return PLAT_TRUE;
}
/*******************************************************************************
* ��  ��  ��      : Init_ADXL
* ��      ��      : ��ʼ��ADXL345��
* ��      ��      : ��:
* ��      ��      : �����Ƿ�ɹ�
*******************************************************************************/
uint8_t Init_ADXL(void)
{
	bool_t result = 0;
	I2C_Config(ADXL345_ADDRESS);
	result = I2C_WriteByte(0x31, 0x09);//������Χ,����4g��13λģʽ��3.9mg/LSB
	if(PLAT_TRUE != result) return PLAT_FALSE;
	result = I2C_WriteByte(0x2C, 0x0A);//100hz, normal operation
	result = I2C_WriteByte(0x2E, 0x80); //ʹ�� DATA_READY �ж�
	result = I2C_WriteByte(0x2D, 0x08);//ѡ���Դģʽ

	return PLAT_TRUE;
}

/*******************************************************************************
* ��  ��  ��      : ADXL_Standby
* ��      ��      : ADXL345����ģʽ��
* ��      ��      : ��:
* ��      ��      : ��
*******************************************************************************/
void ADXL_Standby(void)
{
	I2C_WriteByte(0x2D, 0x00);
}
/*******************************************************************************
* ��  ��  ��      : GetHMCxyz
* ��      ��      : ���HMC5883L���������������
* ��      ��      : pMag:��������ṹ��
* ��      ��      : ��
*******************************************************************************/
void GetHMCxyz(HMC5883mag_t *pMag )
{
  uint8_t buffer[6];
  I2C_Config(HMC5883_ADDRESS);
  I2C_ReadMultiByte(0x03, buffer, 6); //�����������ݣ���λ��ǰ����λ�ں󣬴洢��BUF��
  pMag->x=(buffer[0] << 8 | buffer[1]); //Combine MSB and LSB of X Data output register
  pMag->z=(buffer[2] << 8 | buffer[3]); //Combine MSB and LSB of Z Data output register
  pMag->y=(buffer[4] << 8 | buffer[5]); //Combine MSB and LSB of Y Data output register

}
/*******************************************************************************
* ��  ��  ��      : GetADXLxyz
* ��      ��      : ���ADXL345���������������X��������ȵͺ�ߣ���λ��0x32,��λ��0x33��  ����HMC5883��ͬ
* ��      ��      : X, Y, Z:�������
* ��      ��      : �����Ƿ�ɹ�
*******************************************************************************/
uint8_t GetADXLxyz(int *x, int *y, int *z )
{
  bool_t result = 0;
  uint8_t buffer[6];
  I2C_Config(ADXL345_ADDRESS);
  result = I2C_ReadMultiByte(0x32, buffer, 6);
  if(PLAT_TRUE != result) return PLAT_FALSE;
  *x=(buffer[1] << 8 | buffer[0]); //Combine MSB and LSB of X Data output register
  *y=(buffer[3] << 8 | buffer[2]); //Combine MSB and LSB of Z Data output register
  *z=(buffer[5] << 8 | buffer[4]); //Combine MSB and LSB of Y Data output register
  return PLAT_TRUE;
}

/*******************************************************************************
* ��  ��  ��      : CalcCompassAngle
* ��      ��      : ���㷽��Ƕ�(��������X����ű��н�)
* ��      ��      : x_raw,y_raw,z_raw:ADXL345�������������
                    pMag��HMC8883L��������������ṹ��
* ��      ��      : ����������ű��н�
*******************************************************************************/
uint16_t CalcCompassAngle(int x_raw, int y_raw, int z_raw,HMC5883mag_t *pMag)
{
   uint16_t head=0;
   fp32_t pitch=0;//alpha
   fp32_t roll=0;//beta
   fp32_t tmp = 0.0;
//   tmp=(-1)*(float)(x_raw)*0.0039;//�ֱ���Ϊ0.0039mg/LSB
//   pitch=(float)(x_raw);

   tmp=(-1.0)*(fp64_t)(x_raw)*0.0039;
   if (tmp>1.0)
   {
      tmp=1.0;
   }
    if (tmp<-1.0)
   {
      tmp=-1.0;
   }
   pitch=asin(tmp);//ע����������
   tmp = 0;
//   tmp=(float)(y_raw)/((float)(z_raw)*9.8);

   //================
//   roll=atan((float)(y_raw)/((float)(z_raw)*9.8));
   tmp = (fp32_t)(y_raw)*0.0039/(cos(pitch));
   if (tmp>1.0)
   {
      tmp=1.0;
   }
    if (tmp<-1.0)
   {
      tmp=-1.0;
   }
   roll=asin(tmp);
   tmp = 0;
   //===============

   fp32_t Hx,Hy;
//   Hx=(pMag->x)*cos(pitch)+(pMag->y)*sin(pitch)*sin(roll)+(pMag->z)*sin(pitch)*cos(roll);
//   Hy=(pMag->y)*cos(roll)-(pMag->z)*sin(roll);

   Hx = CompassCalParam.xGain*(pMag->x - CompassCalParam.Xcal)*cos(pitch)
   			+ CompassCalParam.yGain*(pMag->y - CompassCalParam.Ycal)*sin(pitch)*sin(roll)
   			+ CompassCalParam.zGain*(pMag->z - CompassCalParam.Zcal)*sin(pitch)*cos(roll);
   Hy = CompassCalParam.yGain * (pMag->y - CompassCalParam.Ycal)*cos(roll)
   			- CompassCalParam.zGain * (pMag->z - CompassCalParam.Zcal)*sin(roll);

   // Hx = xGain*(pMag->x - Xcal);
    //Hy= yGain * (pMag->y - Ycal);

   tmp=180/PI*atan(Hy/Hx);
   if(Hx>0.0 && Hy>0.0)
   {
      head=tmp;
   }
   else if(Hx<0.0)
   {
      head=180.0+tmp;
   }
   else if(Hx>0.0 && Hy<0.0)
   {
      head=360.0+tmp;
   }
   else if(Hx==0.0 && Hy>0.0)
   {
      head=90;
   }
   else if(Hx==0.0 && Hy<0.0)
   {
      head=270;
   }
   else if(Hy==0.0 && Hx<0.0)
   {
      head=180;
   }
   else if(Hy==0.0 && Hx>0.0)
   {
      head=0;
   }
    return head;
}
/*******************************************************************************
* ��  ��  ��      : GerHeading
* ��      ��      : ��õ�������X����ű��н�
* ��      ��      : ����������ű��нǴ�Ž��
* ��      ��      : �����Ƿ�ɹ�
*******************************************************************************/
static uint8_t GerHeading(uint16_t *pHeading)
{
   bool_t result = 0;
   int32_t Lx = 0, Ly = 0, Lz = 0;
   HMC5883mag_t hmc_mag;

   hmc_mag.x = 0.0;
   hmc_mag.y = 0.0;
   hmc_mag.z = 0.0;

   result = Init_ADXL();
    if(PLAT_TRUE != result) return PLAT_FALSE;

   //��ȡ���ٶȴ�������ֵ
   GetADXLxyz(&Lx, &Ly, &Lz );//x�������¸�����y�������ҷ���
   //�������ģʽ
   ADXL_Standby();

   result = Init_HMC5883();
   if(PLAT_TRUE != result) return PLAT_FALSE;
   //��ȡ�ų�����
   GetHMCxyz(&hmc_mag);
   //�������ģʽ
   HMC5883_Standby();

   //�ܼ���
   *pHeading = CalcCompassAngle(-Ly, Lx, Lz,&hmc_mag);

   return PLAT_TRUE;

}


/*******************************************************************************
* ��  ��  ��      : MgnCalibration
* ��      ��      : �شŴ�����У׼
* ��      ��      : ��
* ��      ��      : 
  0 ok
  1 HMC5883 data overflow
  2 not good enough for Calibration
*******************************************************************************/
static uint8_t MgnCalibration(void)
{
	int16_t i,j;      
	int32_t count = 500;
	fp64_t alpha[6] = {0};
	fp64_t beta = 0;
	uint8_t offset[6] = {0,6,11,15,18,20};
	int32_t minX = 4096, maxX = -4096, minY = 4096, maxY = -4096,minZ = 4096, maxZ = -4096;
	HMC5883mag_t hmc_mag;
	fp64_t HtH[21] = {0};
	fp64_t Htw[6] = {0};
	fp32_t CaliResult[6];
	uint8_t CalibThresh = 125;

	InitHMC5883ForCalib();
	GetHMCxyz(&hmc_mag);//drop the first data;
	delay_ms(15);
	for (i = 0; i < 6; i++)
	{
		Htw[i] = 0;
		for(j = i; j < 6; j++ )
		HtH[offset[i] + j - i] = 0;
	}	
	//calculate HtH and Htw	
	for (; count > 0; count--)
	{       
		//delay_ms(5);
		//P1OUT ^= BIT2;//LED
		GetHMCxyz(&hmc_mag);
		if(hmc_mag.x == -4096 ||hmc_mag.x == -4096||hmc_mag.x == -4096)
		{
			HMC5883_Standby();
			return 1; //data overflow
		}
		minX = minX < hmc_mag.x? minX : hmc_mag.x;
		maxX = maxX > hmc_mag.x? maxX : hmc_mag.x;
		minY = minY < hmc_mag.y? minY : hmc_mag.y;
		maxY = maxY > hmc_mag.y? maxY : hmc_mag.y;
		minZ = minZ < hmc_mag.z? minZ : hmc_mag.z;
		maxZ = maxZ > hmc_mag.z? maxZ : hmc_mag.z; //update max and min


		alpha[0] = hmc_mag.x;
		alpha[1] = hmc_mag.y;
		alpha[2] = hmc_mag.z;
		alpha[3] = -((double)hmc_mag.y) * (hmc_mag.y);
		alpha[4] = -((double)hmc_mag.z) * (hmc_mag.z);
		alpha[5] = 1;
		beta = (fp64_t)hmc_mag.x * hmc_mag.x;
		for (i = 0; i < 6; i++)
		{
			Htw[i] = Htw[i] + beta*alpha[i];
			for(j = i; j < 6; j++ )
			HtH[offset[i] + j - i] = HtH[offset[i] + j - i] + ((double)alpha[i]*alpha[j]);
		}	
	}

	HMC5883_Standby();
	if((maxX - minX)< 2*CalibThresh || (maxY - minY)< 2*CalibThresh || (maxZ - minZ)< 2*CalibThresh)
	//if((maxX - minX)<255 || (maxY - minY)<255 || (maxZ - minZ)<255)
	return 2; //not good enough for Calibration

	if(1 == fchol(HtH))
	{
		return 2;
	}
	Utsolve(HtH, Htw);
	Usolve(HtH, Htw);
	OutputResult(Htw, CaliResult);
	write2Flash(CaliResult);
	loadCalibPara(0);
	return 0;
}

/*

******
 *****
  ****
   ***
    **
     *
*/
//performs Cholesky factoristation

static bool_t fchol(fp64_t R[21] )
{
	uint8_t k, i, j;
	uint8_t offset[6] = {0,6,11,15,18,20};
	fp64_t scale;
	
	for (k = 0; k < 6; k++)
	{
		for (i = 1; i < 6 - k; i++)
		{
			//scale = R[k][i]/R[k][k];
			scale = R[offset[k] + i]/R[offset[k]];
			for (j = 0; j < 6 - (k + i); j++)
			{
				R[offset[i + k] + j] -= scale*R[offset[k] + j + i];
			}
		}
		if(R[offset[k]] <= 0)
		{
			return PLAT_FALSE;
		}
		scale = sqrt(R[offset[k]]);
		for (j = 0; j < 6 - k; j++)
		{
			R[offset[k] + j] = R[offset[k] + j]/scale;
		}
	}
	return PLAT_TRUE;
}

// solves U'*x=b
static void Utsolve(fp64_t U[21], fp64_t b[6])
{
	uint8_t i,j;
	uint8_t offset[6] = {0,6,11,15,18,20};

	b[0] = b[0]/U[0];
	
	for (i = 1; i < 6; i++)
	{
		for (j = 0; j < i; j++)
		{
			b[i] -= b[j]*U[offset[j] + i - j];
		}
		b[i] = b[i]/U[offset[i]];
	}
}

//
/*******************************************************************************
* ��  ��  ��      : Usolve
* ��      ��      : solves U*x=b
* ��      ��      : U��b
* ��      ��      : ���������b
*******************************************************************************/
static void Usolve(fp64_t U[21], fp64_t b[6])
{
	int32_t i,j;
	uint8_t offset[6] = {0,6,11,15,18,20};

	b[5] = b[5]/U[20];
	for (i = 4; i >= 0; i--)
	{
		for (j = 1; j < 6 - i; j++)
		{
			b[i] -= b[i + j]*U[offset[i] + j];
		}
		b[i] = b[i]/U[offset[i]];
	}
}


/*******************************************************************************
* ��  ��  ��      : OutputResult
* ��      ��      : �شŴ�����У׼�������
* ��      ��      : У׼�м�ֵ
* ��      ��      : ��
*******************************************************************************/
static void OutputResult(fp64_t X[6], fp32_t result[6])
{
	result[0] = X[0]/2;
	result[1] = X[1]/(2*X[3]);
	result[2] = X[2]/(2*X[4]);	
	result[3] = X[5] + result[0]*result[0] + X[3]*result[1]*result[1] 
					+ X[4]*result[2]*result[2];
	result[4] = result[3]/X[3];
	result[5] = result[3]/X[4];
	result[3] = sqrt(result[3]);	
	result[4] = sqrt(result[4]);
	result[5] = sqrt(result[5]);

}

/*******************************************************************************
* ��  ��  ��      : InitHMC5883ForCalib
* ��      ��      : �شŴ�����У׼��ʼ��
* ��      ��      : ��
* ��      ��      : 
*******************************************************************************/
static void InitHMC5883ForCalib(void)
{
	I2C_Config(HMC5883_ADDRESS);
	I2C_WriteByte(0x00, 0x58);//ƽ����Ϊ8�Σ���������Ϊ15HZ
	delay_ms(5);//��ʱ�ǳ��ؼ���ȥ����������ݲ���  
	I2C_WriteByte(0x01, 0x80);//����4.7Ga
	delay_ms(5);//��ʱ�ǳ��ؼ���ȥ����������ݲ���  
	I2C_WriteByte(0x02, 0x00); //��������ģʽ 
	delay_ms(5);//��ʱ�ǳ��ؼ���ȥ����������ݲ���      
}

/*******************************************************************************
* ��  ��  ��      : write2Flash
* ��      ��      : �شŴ�����У׼�������д��FLASH
* ��      ��      : �شŴ�����У׼���
* ��      ��      : ��
*******************************************************************************/
static void write2Flash(fp32_t result[6])
{
	CompassCalParam.Xcal = result[0];
    CompassCalParam.Ycal = result[1];
    CompassCalParam.Zcal = result[2];
    CompassCalParam.xGain = result[3];
    CompassCalParam.yGain = result[4];
    CompassCalParam.zGain = result[5];
}

/*******************************************************************************
* ��  ��  ��      : loadCalibPara
* ��      ��      : �شŴ�����У׼���������FLASH������ȫ�ֱ���
* ��      ��      : ��
* ��      ��      : ��
*******************************************************************************/
static void loadCalibPara(bool_t res)
{  
	if(res)
	{ 
		CompassCalParam.Xcal = 0;
		CompassCalParam.Ycal = -27;
		CompassCalParam.Zcal = -6;
		CompassCalParam.xGain = 1;
		CompassCalParam.yGain = 1;
		CompassCalParam.zGain = 1;
	}
	else
	{ 
		//CompassCalParam.Xcal = 0;
		//CompassCalParam.Ycal = 0;
		//CompassCalParam.Zcal = 0;
		//CompassCalParam.xGain = 0;
		//CompassCalParam.yGain = 0;
		//CompassCalParam.zGain = 0;
		CompassCalParam.yGain = CompassCalParam.xGain/CompassCalParam.yGain;
		CompassCalParam.zGain = CompassCalParam.xGain/CompassCalParam.zGain;
		CompassCalParam.xGain = 1;
	}
}




