#include <platform.h>
#include <mss_uart.h>
#include <mss_uart_regs.h>

#define UART_RX_FIFO_MAX_LEN	16

typedef struct
{	
	bool_t valid_flag;
	fpv_t func;
	queue_t queue;
}uart_entity_t;

static uart_entity_t uart_entity[UART_NUM];
static void hal_uart_rx_handler(mss_uart_instance_t * this_uart);

void hal_uart_init(uint8_t uart_id, uint32_t baud_rate)
{
	bool_t res;
	
	if (uart_id == UART_DEBUG)
	{
		MSS_UART_init(&g_mss_uart0,
                   baud_rate,
                   MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);
		uart_entity[UART_DEBUG].valid_flag = PLAT_TRUE;
		res = queue_init(&uart_entity[UART_DEBUG].queue, UART_RECV_BUF_SIZE);
		DBG_ASSERT(res != PLAT_FALSE);
	}
	else if (uart_id == UART_BD)
	{
		MSS_UART_init(&g_mss_uart1,
                   baud_rate,
                   MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);
		uart_entity[UART_BD].valid_flag = PLAT_TRUE;
		res = queue_init(&uart_entity[UART_BD].queue, UART_RECV_BUF_SIZE);
		DBG_ASSERT(res != PLAT_FALSE);
	}
}

void hal_uart_send_char(uint8_t uart_id, uint8_t data)
{
	if (uart_id == UART_DEBUG)
	{
		MSS_UART_polled_tx(&g_mss_uart0, &data, 1);
	}
	else if (uart_id == UART_BD)
	{
		MSS_UART_polled_tx(&g_mss_uart1, &data, 1);
	}
}

void hal_uart_send_string(uint8_t uart_id, uint8_t *string, uint32_t length)
{
	if (uart_id == UART_DEBUG)
	{
		MSS_UART_polled_tx(&g_mss_uart0, string, length);
	}
	else if (uart_id == UART_BD)
	{
		MSS_UART_polled_tx(&g_mss_uart1, string, length);
	}
}

void hal_uart_printf(uint8_t uart_id, uint8_t *string)
{
	if (uart_id == UART_DEBUG)
	{
		MSS_UART_polled_tx_string(&g_mss_uart0, string);
	}
	else if (uart_id == UART_BD)
	{
		MSS_UART_polled_tx_string(&g_mss_uart1, string);
	}	
}

uint32_t hal_uart_read(uint8_t uart_id, void *buffer, uint32_t len)
{
	uint32_t i = 0;
    queue_data_t e;
    uint8_t *buf = (uint8_t *)buffer;

    if (queue_length(&uart_entity[uart_id].queue) >= len)
    {
        for (i=0; i<len; i++)
        {
            delete_queue(&uart_entity[uart_id].queue, &e);
            buf[i] = e.data;
        }
    }
    else
    {
        while((queue_length(&uart_entity[uart_id].queue) != 0) && (i<len))
        {
            delete_queue(&uart_entity[uart_id].queue, &e);
            buf[i++] = e.data;
        }
    }

    return i;
	
}

void hal_uart_rx_irq_enable(uint8_t uart_id, fpv_t func)
{	
	if (uart_id == UART_DEBUG)
	{
		uart_entity[UART_DEBUG].func = func;
		
		MSS_UART_set_rx_handler(&g_mss_uart0,
                              hal_uart_rx_handler,
                              MSS_UART_FIFO_EIGHT_BYTES);	
	}
	else if (uart_id == UART_BD)
	{
		uart_entity[UART_BD].func = func;
		
		MSS_UART_set_rx_handler(&g_mss_uart1,
                              hal_uart_rx_handler,
                              MSS_UART_FIFO_EIGHT_BYTES);	
	}
}

void hal_uart_rx_irq_disable(uint8_t uart_id)
{
	if (uart_id == UART_DEBUG)
	{
		uart_entity[UART_DEBUG].func = PLAT_NULL;
		
		MSS_UART_disable_irq (&g_mss_uart0, MSS_UART_RBF_IRQ);
	}
	else if (uart_id == UART_BD)
	{
		uart_entity[UART_BD].func = PLAT_NULL;
		
		MSS_UART_disable_irq (&g_mss_uart1, MSS_UART_RBF_IRQ);
	}
}

static void hal_uart_rx_handler(mss_uart_instance_t * this_uart)
{
	uint8_t rxfifo[UART_RX_FIFO_MAX_LEN];
	uint8_t valid_rx_len;
	queue_data_t e;
	bool_t res;
	
	if (this_uart == &g_mss_uart0)
	{
		valid_rx_len = MSS_UART_get_rx(&g_mss_uart0, rxfifo, UART_RX_FIFO_MAX_LEN);

		if (uart_entity[UART_DEBUG].func != PLAT_NULL)
		{
			for (uint8_t loop=0; loop<valid_rx_len; loop++)
			{
	            e.data = rxfifo[loop];
				res = enter_queue(&uart_entity[UART_DEBUG].queue, e);
				if (res == PLAT_FALSE) break;
			}			
			
			if (valid_rx_len)
			{
				(* (uart_entity[UART_DEBUG].func) )();
			}
		}		
	}
	else if (this_uart == &g_mss_uart1)
	{
		valid_rx_len = MSS_UART_get_rx(&g_mss_uart1, rxfifo, UART_RX_FIFO_MAX_LEN);

		if (uart_entity[UART_BD].func != PLAT_NULL)
		{
			for (uint8_t loop=0; loop<valid_rx_len; loop++)
			{
	            e.data = rxfifo[loop];
				res = enter_queue(&uart_entity[UART_BD].queue, e);
				if (res == PLAT_FALSE) break;
			}			
			
			if (valid_rx_len)
			{
				(* (uart_entity[UART_BD].func) )();
			}
		}	
	}
}





