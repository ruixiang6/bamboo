/*******************************************************************************
 * (c) Copyright 2010-2013 Microsemi SoC Products Group.  All rights reserved.
 * 
 * SmartFusion2 vector table and startup code for IAR EWARM.
 *
 * SVN $Revision: 5268 $
 * SVN $Date: 2013-03-21 20:51:18 +0000 (Thu, 21 Mar 2013) $
 */

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)
	
        EXTERN  __iar_program_start
        EXTERN  SystemInit
		EXTERN  OS_CPU_PendSVHandler
        EXTERN  OS_CPU_SysTickHandler
        PUBLIC  __vector_table

        DATA
__vector_table
        DCD     sfe(CSTACK)
        DCD     Reset_Handler

        DCD     NMI_Handler
        DCD     HardFault_Handler
        DCD     MemManage_Handler
        DCD     BusFault_Handler
        DCD     UsageFault_Handler
        DCD     0
        DCD     0
        DCD     0
        DCD     0
        DCD     SVC_Handler
        DCD     DebugMon_Handler
        DCD     0
        DCD     PendSV_Handler
        DCD     SysTick_Handler

        ; External Interrupts
        DCD     WdogWakeup_IRQHandler
        DCD     RTC_Wakeup_IRQHandler
        DCD     SPI0_IRQHandler
        DCD     SPI1_IRQHandler
        DCD     I2C0_IRQHandler
        DCD     I2C0_SMBAlert_IRQHandler
        DCD     I2C0_SMBus_IRQHandler
        DCD     I2C1_IRQHandler
        DCD     I2C1_SMBAlert_IRQHandler
        DCD     I2C1_SMBus_IRQHandler
        DCD     UART0_IRQHandler
        DCD     UART1_IRQHandler
        DCD     EthernetMAC_IRQHandler
        DCD     DMA_IRQHandler
        DCD     Timer1_IRQHandler
        DCD     Timer2_IRQHandler
        DCD     CAN_IRQHandler
        DCD     ENVM0_IRQHandler
        DCD     ENVM1_IRQHandler
        DCD     ComBlk_IRQHandler
        DCD     USB_IRQHandler
        DCD     USB_DMA_IRQHandler
        DCD     PLL_Lock_IRQHandler
        DCD     PLL_LockLost_IRQHandler
        DCD     CommSwitchError_IRQHandler
        DCD     CacheError_IRQHandler
        DCD     DDR_IRQHandler
        DCD     HPDMA_Complete_IRQHandler
        DCD     HPDMA_Error_IRQHandler
        DCD     ECC_Error_IRQHandler
        DCD     MDDR_IOCalib_IRQHandler
        DCD     FAB_PLL_Lock_IRQHandler
        DCD     FAB_PLL_LockLost_IRQHandler
        DCD     FIC64_IRQHandler
        DCD     FabricIrq0_IRQHandler
        DCD     FabricIrq1_IRQHandler
        DCD     FabricIrq2_IRQHandler
        DCD     FabricIrq3_IRQHandler
        DCD     FabricIrq4_IRQHandler
        DCD     FabricIrq5_IRQHandler
        DCD     FabricIrq6_IRQHandler
        DCD     FabricIrq7_IRQHandler
        DCD     FabricIrq8_IRQHandler
        DCD     FabricIrq9_IRQHandler
        DCD     FabricIrq10_IRQHandler
        DCD     FabricIrq11_IRQHandler
        DCD     FabricIrq12_IRQHandler
        DCD     FabricIrq13_IRQHandler
        DCD     FabricIrq14_IRQHandler
        DCD     FabricIrq15_IRQHandler                        
        DCD     GPIO0_IRQHandler
        DCD     GPIO1_IRQHandler
        DCD     GPIO2_IRQHandler
        DCD     GPIO3_IRQHandler
        DCD     GPIO4_IRQHandler
        DCD     GPIO5_IRQHandler
        DCD     GPIO6_IRQHandler
        DCD     GPIO7_IRQHandler
        DCD     GPIO8_IRQHandler
        DCD     GPIO9_IRQHandler
        DCD     GPIO10_IRQHandler
        DCD     GPIO11_IRQHandler
        DCD     GPIO12_IRQHandler
        DCD     GPIO13_IRQHandler
        DCD     GPIO14_IRQHandler
        DCD     GPIO15_IRQHandler
        DCD     GPIO16_IRQHandler
        DCD     GPIO17_IRQHandler
        DCD     GPIO18_IRQHandler
        DCD     GPIO19_IRQHandler
        DCD     GPIO20_IRQHandler
        DCD     GPIO21_IRQHandler
        DCD     GPIO22_IRQHandler
        DCD     GPIO23_IRQHandler
        DCD     GPIO24_IRQHandler
        DCD     GPIO25_IRQHandler
        DCD     GPIO26_IRQHandler
        DCD     GPIO27_IRQHandler
        DCD     GPIO28_IRQHandler
        DCD     GPIO29_IRQHandler
        DCD     GPIO30_IRQHandler
        DCD     GPIO31_IRQHandler


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
        THUMB

        PUBWEAK Reset_Handler
        SECTION .text:CODE:REORDER(2)
Reset_Handler
        LDR     R0, =SystemInit
        BLX     R0
        LDR     R0, =__iar_program_start
        BX      R0
        
        PUBWEAK NMI_Handler
        SECTION .text:CODE:REORDER(1)
NMI_Handler
        B NMI_Handler
		
        PUBWEAK HardFault_Handler
        SECTION .text:CODE:REORDER(1)
HardFault_Handler
        B HardFault_Handler
		
        PUBWEAK MemManage_Handler
        SECTION .text:CODE:REORDER(1)
MemManage_Handler
        B MemManage_Handler
		
        PUBWEAK BusFault_Handler
        SECTION .text:CODE:REORDER(1)
BusFault_Handler
        B BusFault_Handler
		
        PUBWEAK UsageFault_Handler
        SECTION .text:CODE:REORDER(1)
UsageFault_Handler
        B UsageFault_Handler
		
        PUBWEAK SVC_Handler
        SECTION .text:CODE:REORDER(1)
SVC_Handler
        B SVC_Handler
		
        PUBWEAK DebugMon_Handler
        SECTION .text:CODE:REORDER(1)
DebugMon_Handler
        B DebugMon_Handler
		
        PUBWEAK PendSV_Handler
        SECTION .text:CODE:REORDER(1)
PendSV_Handler
        B OS_CPU_PendSVHandler
		
        PUBWEAK SysTick_Handler
        SECTION .text:CODE:REORDER(1)
SysTick_Handler
        B OS_CPU_SysTickHandler

        PUBWEAK WdogWakeup_IRQHandler
        SECTION .text:CODE:REORDER(1)
WdogWakeup_IRQHandler
        B WdogWakeup_IRQHandler
		
        PUBWEAK RTC_Wakeup_IRQHandler
        SECTION .text:CODE:REORDER(1)
RTC_Wakeup_IRQHandler
        B RTC_Wakeup_IRQHandler

        PUBWEAK SPI0_IRQHandler
        SECTION .text:CODE:REORDER(1)
SPI0_IRQHandler
        B SPI0_IRQHandler
		
        PUBWEAK SPI1_IRQHandler
        SECTION .text:CODE:REORDER(1)
SPI1_IRQHandler
        B SPI1_IRQHandler

        PUBWEAK I2C0_IRQHandler
        SECTION .text:CODE:REORDER(1)
I2C0_IRQHandler
        B I2C0_IRQHandler
		
        PUBWEAK I2C0_SMBAlert_IRQHandler
        SECTION .text:CODE:REORDER(1)
I2C0_SMBAlert_IRQHandler
        B I2C0_SMBAlert_IRQHandler

        PUBWEAK I2C0_SMBus_IRQHandler
        SECTION .text:CODE:REORDER(1)
I2C0_SMBus_IRQHandler
        B I2C0_SMBus_IRQHandler
		
        PUBWEAK I2C1_IRQHandler
        SECTION .text:CODE:REORDER(1)
I2C1_IRQHandler
        B I2C1_IRQHandler
		
        PUBWEAK I2C1_SMBAlert_IRQHandler
        SECTION .text:CODE:REORDER(1)
I2C1_SMBAlert_IRQHandler
        B I2C1_SMBAlert_IRQHandler
		
        PUBWEAK I2C1_SMBus_IRQHandler
        SECTION .text:CODE:REORDER(1)
I2C1_SMBus_IRQHandler
        B I2C1_SMBus_IRQHandler

        PUBWEAK UART0_IRQHandler
        SECTION .text:CODE:REORDER(1)
UART0_IRQHandler
        B UART0_IRQHandler
		
        PUBWEAK UART1_IRQHandler
        SECTION .text:CODE:REORDER(1)
UART1_IRQHandler
        B UART1_IRQHandler

        PUBWEAK EthernetMAC_IRQHandler
        SECTION .text:CODE:REORDER(1)
EthernetMAC_IRQHandler
        B EthernetMAC_IRQHandler
       
        PUBWEAK DMA_IRQHandler
        SECTION .text:CODE:REORDER(1)
DMA_IRQHandler
        B DMA_IRQHandler
       
        PUBWEAK Timer1_IRQHandler
        SECTION .text:CODE:REORDER(1)
Timer1_IRQHandler
        B Timer1_IRQHandler
		
        PUBWEAK Timer2_IRQHandler
        SECTION .text:CODE:REORDER(1)
Timer2_IRQHandler
        B Timer2_IRQHandler
        
        PUBWEAK CAN_IRQHandler
        SECTION .text:CODE:REORDER(1)
CAN_IRQHandler
        B CAN_IRQHandler

        PUBWEAK ENVM0_IRQHandler
        SECTION .text:CODE:REORDER(1)
ENVM0_IRQHandler
        B ENVM0_IRQHandler
		
        PUBWEAK ENVM1_IRQHandler
        SECTION .text:CODE:REORDER(1)
ENVM1_IRQHandler
        B ENVM1_IRQHandler

        PUBWEAK ComBlk_IRQHandler
        SECTION .text:CODE:REORDER(1)
ComBlk_IRQHandler
        B ComBlk_IRQHandler
		
        PUBWEAK USB_IRQHandler
        SECTION .text:CODE:REORDER(1)
USB_IRQHandler
        B USB_IRQHandler

        PUBWEAK USB_DMA_IRQHandler
        SECTION .text:CODE:REORDER(1)
USB_DMA_IRQHandler
        B USB_DMA_IRQHandler

        PUBWEAK PLL_Lock_IRQHandler
        SECTION .text:CODE:REORDER(1)
PLL_Lock_IRQHandler
        B PLL_Lock_IRQHandler
		
        PUBWEAK PLL_LockLost_IRQHandler
        SECTION .text:CODE:REORDER(1)
PLL_LockLost_IRQHandler
        B PLL_LockLost_IRQHandler

        PUBWEAK CommSwitchError_IRQHandler
        SECTION .text:CODE:REORDER(1)
CommSwitchError_IRQHandler
        B CommSwitchError_IRQHandler

        PUBWEAK CacheError_IRQHandler
        SECTION .text:CODE:REORDER(1)
CacheError_IRQHandler
        B CacheError_IRQHandler

        PUBWEAK DDR_IRQHandler
        SECTION .text:CODE:REORDER(1)
DDR_IRQHandler
        B DDR_IRQHandler
        
        PUBWEAK HPDMA_Complete_IRQHandler
        SECTION .text:CODE:REORDER(1)
HPDMA_Complete_IRQHandler
        B HPDMA_Complete_IRQHandler
        
        PUBWEAK HPDMA_Error_IRQHandler
        SECTION .text:CODE:REORDER(1)
HPDMA_Error_IRQHandler
        B HPDMA_Error_IRQHandler
        
        PUBWEAK ECC_Error_IRQHandler
        SECTION .text:CODE:REORDER(1)
ECC_Error_IRQHandler
        B ECC_Error_IRQHandler
        
        PUBWEAK MDDR_IOCalib_IRQHandler
        SECTION .text:CODE:REORDER(1)
MDDR_IOCalib_IRQHandler
        B MDDR_IOCalib_IRQHandler
        
        PUBWEAK FAB_PLL_Lock_IRQHandler
        SECTION .text:CODE:REORDER(1)        
FAB_PLL_Lock_IRQHandler
        B FAB_PLL_Lock_IRQHandler

        PUBWEAK FAB_PLL_LockLost_IRQHandler
        SECTION .text:CODE:REORDER(1)          
FAB_PLL_LockLost_IRQHandler        
        B FAB_PLL_LockLost_IRQHandler

        PUBWEAK FIC64_IRQHandler
        SECTION .text:CODE:REORDER(1)          
FIC64_IRQHandler
        B FIC64_IRQHandler
        
        PUBWEAK FabricIrq0_IRQHandler
        SECTION .text:CODE:REORDER(1)
FabricIrq0_IRQHandler
        B FabricIrq0_IRQHandler

        PUBWEAK FabricIrq1_IRQHandler
        SECTION .text:CODE:REORDER(1)
FabricIrq1_IRQHandler
        B FabricIrq1_IRQHandler

        PUBWEAK FabricIrq2_IRQHandler
        SECTION .text:CODE:REORDER(1)
FabricIrq2_IRQHandler
        B FabricIrq2_IRQHandler

        PUBWEAK FabricIrq3_IRQHandler
        SECTION .text:CODE:REORDER(1)
FabricIrq3_IRQHandler
        B FabricIrq3_IRQHandler

        PUBWEAK FabricIrq4_IRQHandler
        SECTION .text:CODE:REORDER(1)
FabricIrq4_IRQHandler
        B FabricIrq4_IRQHandler

        PUBWEAK FabricIrq5_IRQHandler
        SECTION .text:CODE:REORDER(1)
FabricIrq5_IRQHandler
        B FabricIrq5_IRQHandler

        PUBWEAK FabricIrq6_IRQHandler
        SECTION .text:CODE:REORDER(1)
FabricIrq6_IRQHandler
        B FabricIrq6_IRQHandler

        PUBWEAK FabricIrq7_IRQHandler
        SECTION .text:CODE:REORDER(1)
FabricIrq7_IRQHandler
        B FabricIrq7_IRQHandler
        
        PUBWEAK FabricIrq8_IRQHandler
        SECTION .text:CODE:REORDER(1)
FabricIrq8_IRQHandler
        B FabricIrq8_IRQHandler

        PUBWEAK FabricIrq9_IRQHandler
        SECTION .text:CODE:REORDER(1)
FabricIrq9_IRQHandler
        B FabricIrq9_IRQHandler

        PUBWEAK FabricIrq10_IRQHandler
        SECTION .text:CODE:REORDER(1)
FabricIrq10_IRQHandler
        B FabricIrq10_IRQHandler

        PUBWEAK FabricIrq11_IRQHandler
        SECTION .text:CODE:REORDER(1)
FabricIrq11_IRQHandler
        B FabricIrq11_IRQHandler

        PUBWEAK FabricIrq12_IRQHandler
        SECTION .text:CODE:REORDER(1)
FabricIrq12_IRQHandler
        B FabricIrq12_IRQHandler

        PUBWEAK FabricIrq13_IRQHandler
        SECTION .text:CODE:REORDER(1)
FabricIrq13_IRQHandler
        B FabricIrq13_IRQHandler

        PUBWEAK FabricIrq14_IRQHandler
        SECTION .text:CODE:REORDER(1)
FabricIrq14_IRQHandler
        B FabricIrq14_IRQHandler

        PUBWEAK FabricIrq15_IRQHandler
        SECTION .text:CODE:REORDER(1)
FabricIrq15_IRQHandler
        B FabricIrq15_IRQHandler        
		
        PUBWEAK GPIO0_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO0_IRQHandler
        B GPIO0_IRQHandler
		
        PUBWEAK GPIO1_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO1_IRQHandler
        B GPIO1_IRQHandler
		
        PUBWEAK GPIO2_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO2_IRQHandler
        B GPIO2_IRQHandler
		
        PUBWEAK GPIO3_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO3_IRQHandler
        B GPIO3_IRQHandler
		
        PUBWEAK GPIO4_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO4_IRQHandler
        B GPIO4_IRQHandler
		
        PUBWEAK GPIO5_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO5_IRQHandler
        B GPIO5_IRQHandler
		
        PUBWEAK GPIO6_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO6_IRQHandler
        B GPIO6_IRQHandler
		
        PUBWEAK GPIO7_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO7_IRQHandler
        B GPIO7_IRQHandler
		
        PUBWEAK GPIO8_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO8_IRQHandler
        B GPIO8_IRQHandler
		
        PUBWEAK GPIO9_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO9_IRQHandler
        B GPIO9_IRQHandler
		
        PUBWEAK GPIO10_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO10_IRQHandler
        B GPIO10_IRQHandler
		
        PUBWEAK GPIO11_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO11_IRQHandler
        B GPIO11_IRQHandler
		
        PUBWEAK GPIO12_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO12_IRQHandler
        B GPIO12_IRQHandler
		
        PUBWEAK GPIO13_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO13_IRQHandler
        B GPIO13_IRQHandler
		
        PUBWEAK GPIO14_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO14_IRQHandler
        B GPIO14_IRQHandler
		
        PUBWEAK GPIO15_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO15_IRQHandler
        B GPIO15_IRQHandler
		
        PUBWEAK GPIO16_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO16_IRQHandler
        B GPIO16_IRQHandler
		
        PUBWEAK GPIO17_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO17_IRQHandler
        B GPIO17_IRQHandler
		
        PUBWEAK GPIO18_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO18_IRQHandler
        B GPIO18_IRQHandler
		
        PUBWEAK GPIO19_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO19_IRQHandler
        B GPIO19_IRQHandler
		
        PUBWEAK GPIO20_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO20_IRQHandler
        B GPIO20_IRQHandler
		
        PUBWEAK GPIO21_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO21_IRQHandler
        B GPIO21_IRQHandler
		
        PUBWEAK GPIO22_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO22_IRQHandler
        B GPIO22_IRQHandler
		
        PUBWEAK GPIO23_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO23_IRQHandler
        B GPIO23_IRQHandler
		
        PUBWEAK GPIO24_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO24_IRQHandler
        B GPIO24_IRQHandler
		
        PUBWEAK GPIO25_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO25_IRQHandler
        B GPIO25_IRQHandler
		
        PUBWEAK GPIO26_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO26_IRQHandler
        B GPIO26_IRQHandler
		
        PUBWEAK GPIO27_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO27_IRQHandler
        B GPIO27_IRQHandler
		
        PUBWEAK GPIO28_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO28_IRQHandler
        B GPIO28_IRQHandler
		
        PUBWEAK GPIO29_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO29_IRQHandler
        B GPIO29_IRQHandler
		
        PUBWEAK GPIO30_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO30_IRQHandler
        B GPIO30_IRQHandler
		
        PUBWEAK GPIO31_IRQHandler
        SECTION .text:CODE:REORDER(1)
GPIO31_IRQHandler
        B GPIO31_IRQHandler

        PUBWEAK mscc_post_hw_cfg_init
        SECTION .text:CODE:REORDER(1)
mscc_post_hw_cfg_init
        BX LR
        
        END
