/*
************************************************************************************************************************
*1.ƽ̨����ϵͳʹ����UCOSII���ں�,�¼������ź�������Ϣ���з�ʽ.Ŀǰ����������ȼ�Ϊ63,�����������û��Զ���.�����ڲ�������1��ϵͳ������
* IdleTask ���ȼ�Ϊ63, ����һ��stateͳ������Ϊ62(δʹ�ã��ʱ���)
*2.Ӳ��CPU����M2SXXX(Cortex M3�ں�),û�������ж�Ƕ��ģʽ������ռʽ���ȼ�,���������ȼ�0~15
* System Tick 	���ȼ�Ϊ0xff
* SVPend 		���ȼ�Ϊ0xff
************************************************************************************************************************
*/

#ifndef __PLATFORM_H
#define __PLATFORM_H

#define CFG_DEBUG_EN 		1	/* debug enable */
#define RTC_EN				1

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* DATA TYPES (Compiler Specific) Add ,most define at stdint.h*/
#include <stdint.h>
typedef char           		char_t;  	/* char type */
typedef unsigned char         bool_t;  	/* Boolean type */
typedef float                  fp32_t;
typedef double                 fp64_t;
typedef void (*fpv_t)(void);      /* function pointer void */      
typedef void (*fppv_t)(void *);   /* function pointer void and parameter void pointer*/

#ifndef PLAT_NULL
#define PLAT_NULL			        ((void*)0)
#endif

#ifndef PLAT_TRUE
#define PLAT_TRUE			        (uint8_t)1
#endif

#ifndef PLAT_FALSE
#define PLAT_FALSE   		        (uint8_t)0
#endif

#include <ucos_ii.h>
////////////////////////Library////////////////////////
#include <debug.h>
#include <check.h>
#include <queue.h>
#include <list.h>
#include <memory.h>
////////////////////////OSEL////////////////////////
#include <osel_kernel.h>
#include <osel_task.h>
#include <osel_event.h>
////////////////////////HAL////////////////////////
#include <hal_misc.h>
#include <hal_uart.h>
#include <hal_timer.h>
#include <hal_rf.h>
#include <hal_mem.h>
#include <hal_gpio.h>
#include <hal_usb.h>
#include <hal_ethernet.h>
#include <hal_audio.h>
#endif
