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


//����Ϣָ��Ϊ��ʱ,ָ��һ������NullPtr��ָ���ֵ.
//��OS�����msg==NULL�᷵��һ��OS_ERR_POST_NULL����
//��lwip�л����sys_mbox_post(mbox,NULL)����һ������Ϣ,����
//�ڱ������а�NULL���һ������ָ��0Xffffffff
const void * const NullPtr = (uint32_t *)0xffffffff;

#if !LWIP_MEM_USE_SRAM
extern uint8_t *memp_memory;				//��memp.c���涨��.
extern uint8_t *ram_heap;					//��mem.c���涨��.
#endif


//����һ����Ϣ����
//*mbox:��Ϣ����
//size:�����С
//����ֵ:ERR_OK,�����ɹ�
//         ����,����ʧ��
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


//�ͷŲ�ɾ��һ����Ϣ����
//*mbox:Ҫɾ������Ϣ����
void sys_mbox_free(sys_mbox_t *mbox)
{
	osel_event_delete(*mbox);
	mbox = PLAT_NULL;
}


//���һ����Ϣ�����Ƿ���Ч
//*mbox:��Ϣ����
//����ֵ:1,��Ч.
//      0,��Ч
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



//����Ϣ�����з���һ����Ϣ(���뷢�ͳɹ�)
//*mbox:��Ϣ����
//*msg:Ҫ���͵���Ϣ
void sys_mbox_post(sys_mbox_t *mbox,void *msg)
{
	osel_event_res_t res;
	
	if (msg == PLAT_NULL)
	{
		msg = (void *)&NullPtr;//��msgΪ��ʱ msg����NullPtrָ���ֵ
	}
		
	do
	{
		res = osel_event_set(*mbox, msg);
		if (res == OSEL_EVENT_FULL) 
		{
			osel_systick_delay(1);
		}
	}
	while (res != OSEL_EVENT_NONE);//��ѭ���ȴ���Ϣ���ͳɹ� 
}


//������һ����Ϣ���䷢����Ϣ
//�˺��������sys_mbox_post����ֻ����һ����Ϣ��
//����ʧ�ܺ󲻻᳢�Եڶ��η���
//*mbox:��Ϣ����
//*msg:Ҫ���͵���Ϣ
//����ֵ:ERR_OK,����OK
// 	     ERR_MEM,����ʧ��
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{ 
	if (msg == PLAT_NULL)
	{
		msg = (void*)&NullPtr;//��msgΪ��ʱ msg����NullPtrָ���ֵ 
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


//�ȴ������е���Ϣ
//*mbox:��Ϣ����
//*msg:��Ϣ
//timeout:��ʱʱ�䣬���timeoutΪ0�Ļ�,��һֱ�ȴ�
//����ֵ:��timeout��Ϊ0ʱ����ɹ��Ļ��ͷ��صȴ���ʱ�䣬
//		ʧ�ܵĻ��ͷ��س�ʱSYS_ARCH_TIMEOUT
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
	osel_event_res_t res;
	u32_t os_tick, os_tick_prev_wait, os_tick_post_wait;

	//msת��Ϊ����ϵͳ������
	if (timeout != 0) 
	{
		os_tick = (timeout * OS_TICKS_PER_SEC) / 1000;
		if (os_tick < 1) os_tick = 1;
	}
	else
	{
		os_tick = 0;
	}
	//���������Ϣ
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
		//���������Ϣ��ʹ�õ�ʱ��(ms)
		timeout = os_tick*1000/OS_TICKS_PER_SEC + 1;
	}
	else
	{
		timeout = SYS_ARCH_TIMEOUT;//����ʱ
	}

	return timeout;
}



//���Ի�ȡ��Ϣ
//*mbox:��Ϣ����
//*msg:��Ϣ
//����ֵ:�ȴ���Ϣ���õ�ʱ��/SYS_ARCH_TIMEOUT
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
	return sys_arch_mbox_fetch(mbox, msg, 1);//���Ի�ȡһ����Ϣ
}


//����һ���ź���
//*sem:�������ź���
//count:�ź���ֵ
//����ֵ:ERR_OK,����OK
// 	     ERR_MEM,����ʧ��
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


//�ȴ�һ���ź���
//*sem:Ҫ�ȴ����ź���
//timeout:��ʱʱ��
//����ֵ:��timeout��Ϊ0ʱ����ɹ��Ļ��ͷ��صȴ���ʱ�䣬
//		ʧ�ܵĻ��ͷ��س�ʱSYS_ARCH_TIMEOUT
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
	osel_event_res_t res;
	u32_t os_tick, os_tick_prev_wait, os_tick_post_wait;

	//msת��Ϊ����ϵͳ������
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
		//���������Ϣ��ʹ�õ�ʱ��(ms)
		timeout = os_tick*1000/OS_TICKS_PER_SEC + 1;
	}
	else
	{
		timeout = SYS_ARCH_TIMEOUT;//����ʱ
	}

	return timeout;
}


//����һ���ź���
//sem:�ź���ָ��
void sys_sem_signal(sys_sem_t *sem)
{
	osel_event_set(*sem, PLAT_NULL);
}


//�ͷŲ�ɾ��һ���ź���
//sem:�ź���ָ��
void sys_sem_free(sys_sem_t *sem)
{
	osel_event_delete(*sem);
	sem = PLAT_NULL;
}


//��ѯһ���ź�����״̬,��Ч����Ч
//sem:�ź���ָ��
//����ֵ:1,��Ч.
//      0,��Ч
int sys_sem_valid(sys_sem_t *sem)
{
	return (sem != PLAT_NULL)? 1:0;
}


//����һ���ź�����Ч
//sem:�ź���ָ��
void sys_sem_set_invalid(sys_sem_t *sem)
{
	*sem = PLAT_NULL;
}


//arch��ʼ��
void sys_init(void)
{
#if !LWIP_MEM_USE_SRAM	
	uint32_t mempsize;
	uint32_t ramheapsize; 

	mempsize = memp_get_memorysize();			//�õ�memp_memory�����С
	ramheapsize = LWIP_MEM_ALIGN_SIZE(MEM_SIZE) + 2*LWIP_MEM_ALIGN_SIZE(4*3) + MEM_ALIGNMENT;//�õ�ram heap��С
	
	memp_memory = heap_alloc(mempsize, 1);	//Ϊmemp_memory�����ڴ�
	ram_heap = heap_alloc(ramheapsize, 1);	//Ϊram_heap�����ڴ�	
	if(!memp_memory || !ram_heap)
	{
		DBG_PRINTF("sys_init error\r\n");
		return;
	}
#endif
} 

//����һ���½���
//*name:��������
//thred:����������
//*arg:�����������Ĳ���
//stacksize:��������Ķ�ջ��С
//prio:������������ȼ�
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
	/* ���� INIT ���� */
	tcpip_task_h = osel_task_create(thread, 
    								arg, 
    								stacksize, 
    								prio);
    LWIP_ASSERT("task create",tcpip_task_h != PLAT_NULL);
	
	return 0;
}


//lwip��ʱ����
//ms:Ҫ��ʱ��ms��
void sys_sleep(u32_t ms)
{
	uint32_t os_tick;
	
	os_tick = (ms * OS_TICKS_PER_SEC) / 1000;

	osel_systick_delay(os_tick);
}


//��ȡϵͳʱ��,LWIP1.4.1���ӵĺ���
//����ֵ:��ǰϵͳʱ��(��λ:����)
u32_t sys_now(void)
{
	u32_t os_tick, os_time;
	
	os_tick = osel_systick_get();	
	//��������ת��Ϊʱ��MS
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

