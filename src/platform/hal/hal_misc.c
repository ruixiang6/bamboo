#include <platform.h>
#include <m2sxxx.h>
#include <mss_pdma.h>
#include <mss_hpdma.h>
#include <cortex_nvic.h>

#define  CM3_NVIC_ST_CTRL    (*((volatile uint32_t *)0xE000E010uL))

void hal_board_init(void)
{
    //close all irq
    __disable_irq();
	//reset system tick
	CM3_NVIC_ST_CTRL = 0;
    //initialize nvic
    NVIC_DeInit();
    //initilize nvic scb
    NVIC_SCBDeInit();    
    NVIC_SetVTOR(0x00010000);
    //set prigroup no pre-emption prio and 4bits sub prio 0~255
    NVIC_SetPriorityGrouping(7);
	//enable all irq
	__enable_irq();
	//´ò¿ª·¢ËÍDMA
	PDMA_init();
    MSS_HPDMA_init();
}

void hal_board_reset(void)
{
  	__disable_irq();
    //reset system tick
	CM3_NVIC_ST_CTRL = 0;
    //
	NVIC_SetVTOR(0);
	uint32_t *address = (uint32_t *)(0+4); 		//pointer to reset handler of application	
	__set_MSP(*(uint32_t *)0);
	__set_CONTROL(0);
	((void (*)())(* address))();
	while(1);	 
}