/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/* lwIP includes. */
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/stats.h"

osel_task_t *tcpip_task_h;


//当消息指针为空时,指向一个常量NullPtr所指向的值.
//在OS中如果msg==NULL会返回一条OS_ERR_POST_NULL错误
//在lwip中会调用sys_mbox_post(mbox,NULL)发送一条空消息,我们
//在本函数中把NULL变成一个常量指针0Xffffffff
const void * const NullPtr = (uint32_t *)0xffffffff;

#if !LWIP_MEM_USE_SRAM
extern uint8_t *memp_memory;				//在memp.c里面定义.
extern uint8_t *ram_heap;					//在mem.c里面定义.
#endif


//创建一个消息邮箱
//*mbox:消息邮箱
//size:邮箱大小
//返回值:ERR_OK,创建成功
//         其他,创建失败
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
	if (size > MAX_QUEUE_ENTRIES) size = MAX_QUEUE_ENTRIES;

	*mbox = osel_event_create(OSEL_EVENT_TYPE_MSG, size);

	if (mbox)
	{
		return ERR_OK;
	}
	else
	{
		return ERR_MEM;	
	}
}


//释放并删除一个消息邮箱
//*mbox:要删除的消息邮箱
void sys_mbox_free(sys_mbox_t *mbox)
{
	osel_event_delete(*mbox);
	mbox = PLAT_NULL;
}


//检查一个消息邮箱是否有效
//*mbox:消息邮箱
//返回值:1,有效.
//      0,无效
int sys_mbox_valid(sys_mbox_t *mbox)
{  
	return (*mbox != PLAT_NULL)? 1:0;
} 


/*------------------------------------------------------------------------------
*/
void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
	*mbox = PLAT_NULL;
}



//向消息邮箱中发送一条消息(必须发送成功)
//*mbox:消息邮箱
//*msg:要发送的消息
void sys_mbox_post(sys_mbox_t *mbox,void *msg)
{
	osel_event_res_t res;
	
	if (msg == PLAT_NULL)
	{
		msg = (void *)&NullPtr;//当msg为空时 msg等于NullPtr指向的值
	}
		
	do
	{
		res = osel_event_set(*mbox, msg);
		if (res == OSEL_EVENT_FULL) 
		{
			osel_systick_delay(1);
		}
	}
	while (res != OSEL_EVENT_NONE);//死循环等待消息发送成功 
}


//尝试向一个消息邮箱发送消息
//此函数相对于sys_mbox_post函数只发送一次消息，
//发送失败后不会尝试第二次发送
//*mbox:消息邮箱
//*msg:要发送的消息
//返回值:ERR_OK,发送OK
// 	     ERR_MEM,发送失败
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{ 
	if (msg == PLAT_NULL)
	{
		msg = (void*)&NullPtr;//当msg为空时 msg等于NullPtr指向的值 
	}	
	
	if (osel_event_set(*mbox, msg) != OSEL_EVENT_NONE)
	{
		return ERR_MEM;
	}
	else
	{
		return ERR_OK;
	}
}


//等待邮箱中的消息
//*mbox:消息邮箱
//*msg:消息
//timeout:超时时间，如果timeout为0的话,就一直等待
//返回值:当timeout不为0时如果成功的话就返回等待的时间，
//		失败的话就返回超时SYS_ARCH_TIMEOUT
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
	osel_event_res_t res;
	u32_t os_tick, os_tick_prev_wait, os_tick_post_wait;

	//ms转换为操作系统节拍数
	if (timeout != 0) 
	{
		os_tick = (timeout * OS_TICKS_PER_SEC) / 1000;
		if (os_tick < 1) os_tick = 1;
	}
	else
	{
		os_tick = 0;
	}
	//清除队列消息
	osel_event_clear(*mbox, PLAT_NULL);
	
	os_tick_prev_wait = osel_systick_get();
	
	res = osel_event_wait(*mbox, os_tick);

	*msg = (*mbox)->object.msg.fetch;

	if (*msg == (void*)&NullPtr)
	{
		*msg = PLAT_NULL;
	}

	if (res != OSEL_EVENT_TIMEOUT)
	{
		os_tick_post_wait = osel_systick_get();

		if (os_tick_post_wait >= os_tick_prev_wait) 
		{
			os_tick = os_tick_post_wait - os_tick_prev_wait;
		}
		else 
		{
			os_tick = 0xffffffff - os_tick_prev_wait + os_tick_post_wait;
		}
		//算出请求消息或使用的时间(ms)
		timeout = os_tick*1000/OS_TICKS_PER_SEC + 1;
	}
	else
	{
		timeout = SYS_ARCH_TIMEOUT;//请求超时
	}

	return timeout;
}



//尝试获取消息
//*mbox:消息邮箱
//*msg:消息
//返回值:等待消息所用的时间/SYS_ARCH_TIMEOUT
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
	return sys_arch_mbox_fetch(mbox, msg, 1);//尝试获取一个消息
}


//创建一个信号量
//*sem:创建的信号量
//count:信号量值
//返回值:ERR_OK,创建OK
// 	     ERR_MEM,创建失败
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{ 
	*sem = osel_event_create(OSEL_EVENT_TYPE_SEM, count);

	if (sem == PLAT_NULL)
	{
		return ERR_MEM; 
	}
	else
	{
		return ERR_OK;
	}
} 


//等待一个信号量
//*sem:要等待的信号量
//timeout:超时时间
//返回值:当timeout不为0时如果成功的话就返回等待的时间，
//		失败的话就返回超时SYS_ARCH_TIMEOUT
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
	osel_event_res_t res;
	u32_t os_tick, os_tick_prev_wait, os_tick_post_wait;

	//ms转换为操作系统节拍数
	if (timeout != 0) 
	{
		os_tick = (timeout * OS_TICKS_PER_SEC) / 1000;
		if (os_tick < 1) os_tick = 1;
	}
	else
	{
		os_tick = 0;
	}

	os_tick_prev_wait = osel_systick_get();
	
	res = osel_event_wait(*sem, os_tick);
	if (res != OSEL_EVENT_TIMEOUT)
	{
		os_tick_post_wait = osel_systick_get();

		if (os_tick_post_wait >= os_tick_prev_wait) 
		{
			os_tick = os_tick_post_wait - os_tick_prev_wait;
		}
		else 
		{
			os_tick = 0xffffffff - os_tick_prev_wait + os_tick_post_wait;
		}
		//算出请求消息或使用的时间(ms)
		timeout = os_tick*1000/OS_TICKS_PER_SEC + 1;
	}
	else
	{
		timeout = SYS_ARCH_TIMEOUT;//请求超时
	}

	return timeout;
}


//发送一个信号量
//sem:信号量指针
void sys_sem_signal(sys_sem_t *sem)
{
	osel_event_set(*sem, PLAT_NULL);
}


//释放并删除一个信号量
//sem:信号量指针
void sys_sem_free(sys_sem_t *sem)
{
	osel_event_delete(*sem);
	sem = PLAT_NULL;
}


//查询一个信号量的状态,无效或有效
//sem:信号量指针
//返回值:1,有效.
//      0,无效
int sys_sem_valid(sys_sem_t *sem)
{
	return (sem != PLAT_NULL)? 1:0;
}


//设置一个信号量无效
//sem:信号量指针
void sys_sem_set_invalid(sys_sem_t *sem)
{
	*sem = PLAT_NULL;
}


//arch初始化
void sys_init(void)
{
#if !LWIP_MEM_USE_SRAM	
	uint32_t mempsize;
	uint32_t ramheapsize; 

	mempsize = memp_get_memorysize();			//得到memp_memory数组大小
	ramheapsize = LWIP_MEM_ALIGN_SIZE(MEM_SIZE) + 2*LWIP_MEM_ALIGN_SIZE(4*3) + MEM_ALIGNMENT;//得到ram heap大小
	
	memp_memory = heap_alloc(mempsize, 1);	//为memp_memory申请内存
	ram_heap = heap_alloc(ramheapsize, 1);	//为ram_heap申请内存	
	if(!memp_memory || !ram_heap)
	{
		DBG_PRINTF("sys_init error\r\n");
		return;
	}
#endif
} 

//创建一个新进程
//*name:进程名称
//thred:进程任务函数
//*arg:进程任务函数的参数
//stacksize:进程任务的堆栈大小
//prio:进程任务的优先级
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
	/* 创建 INIT 任务 */
	tcpip_task_h = osel_task_create(thread, 
    								arg, 
    								stacksize, 
    								prio);
    LWIP_ASSERT("task create",tcpip_task_h != PLAT_NULL);
	
	return 0;
}


//lwip延时函数
//ms:要延时的ms数
void sys_sleep(u32_t ms)
{
	uint32_t os_tick;
	
	os_tick = (ms * OS_TICKS_PER_SEC) / 1000;

	osel_systick_delay(os_tick);
}


//获取系统时间,LWIP1.4.1增加的函数
//返回值:当前系统时间(单位:毫秒)
u32_t sys_now(void)
{
	u32_t os_tick, os_time;
	
	os_tick = osel_systick_get();	
	//将节拍数转换为时间MS
	os_time = (os_tick * 1000 / OS_TICKS_PER_SEC + 1);

	return os_time; 
}

/*------------------------------------------------------------------------------
  This optional function does a "fast" critical region protection and returns
  the previous protection level. This function is only called during very short
  critical regions. An embedded system which supports ISR-based drivers might
  want to implement this function by disabling interrupts. Task-based systems
  might want to implement this by using a mutex or disabling tasking. This
  function should support recursive calls from the same task or interrupt. In
  other words, sys_arch_protect() could be called while already protected. In
  that case the return value indicates that it is already protected.

  sys_arch_protect() is only required if your port is supporting an operating
  system.
*/
sys_prot_t sys_arch_protect(void)
{
	return OS_CPU_SR_Save();
}

/*------------------------------------------------------------------------------
  This optional function does a "fast" set of critical region protection to the
  value specified by pval. See the documentation for sys_arch_protect() for
  more information. This function is only required if your port is supporting
  an operating system.
*/
void sys_arch_unprotect(sys_prot_t pval)
{
    (void) pval;
    OS_CPU_SR_Restore(pval);
}

u32_t sys_arch_random(void)
{
    return 0x42414523u;
}

