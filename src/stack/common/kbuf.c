#include <platform.h>
#include <kbuf.h>

OSEL_DECL_CRITICAL();

static pool_t *kbuf_big_pool;
static pool_t *kbuf_small_pool;

bool_t kbuf_init(void)
{
	kbuf_big_pool = (pool_t *)pool_create(KBUF_BIG_NUM, sizeof(kbuf_t) + KBUF_BIG_SIZE);
	DBG_ASSERT(kbuf_big_pool != PLAT_NULL);

	kbuf_small_pool = (pool_t *)pool_create(KBUF_SMALL_NUM, sizeof(kbuf_t) + KBUF_SMALL_SIZE);
	DBG_ASSERT(kbuf_small_pool != PLAT_NULL);
	
	return PLAT_TRUE;
}

kbuf_t *kbuf_alloc(uint8_t type)
{
	kbuf_t *kbuf = PLAT_NULL;

	OSEL_ENTER_CRITICAL();
	
	if (type == KBUF_SMALL_TYPE)
	{		
		kbuf = pool_alloc(kbuf_small_pool);
		if (kbuf)
		{
			kbuf->type = KBUF_SMALL_TYPE;
			kbuf->base = (uint8_t *)kbuf + sizeof(kbuf_t);
			kbuf->offset = kbuf->base;
			kbuf->priv = PLAT_NULL;
		}
	}
	else if (type == KBUF_BIG_TYPE)
	{
		kbuf = pool_alloc(kbuf_big_pool);
		if (kbuf)
		{
			kbuf->type = KBUF_BIG_TYPE;
			kbuf->base = (uint8_t *)kbuf + sizeof(kbuf_t);
			kbuf->offset = kbuf->base;
			kbuf->priv = PLAT_NULL;
			//DBG_PRINTF("-%d\r\n", kbuf_big_pool->blk_rem_num);
		}		
	}
#if 0
	if (kbuf == NULL 
		|| kbuf_small_pool->blk_rem_num == 2 
		|| kbuf_big_pool->blk_rem_num == 1)
	{
		if (if_spi_state != IF_SEND_STATE)
		{
			hal_spi_set_state(HAL_SPI_BUSY);
			//hal_uart_send_char(UART_DEBUG, 'E');
		}
	}
#endif
	OSEL_EXIT_CRITICAL();
	return kbuf;
}

bool_t kbuf_free(kbuf_t *kbuf)
{
	
	if (kbuf == PLAT_NULL) return PLAT_FALSE;

	OSEL_ENTER_CRITICAL();
	
	if (kbuf->type & KBUF_SMALL_TYPE)
	{		
		pool_free(kbuf_small_pool, kbuf);
	}
	else
	{
		pool_free(kbuf_big_pool, kbuf);
		//DBG_PRINTF("+%d\r\n", kbuf_big_pool->blk_rem_num);
	}
#if 0
	if ( kbuf_small_pool->blk_rem_num>2 
		&& kbuf_big_pool->blk_rem_num>1)
	{
		if (if_spi_state != IF_SEND_STATE)
		{
			hal_spi_set_state(HAL_SPI_IDLE);
		}
	}
#endif
	OSEL_EXIT_CRITICAL();
	return PLAT_TRUE;
}

bool_t kbuf_is_empty(uint8_t type)
{
	if (type == KBUF_SMALL_TYPE)
	{
		if (kbuf_small_pool->blk_rem_num == 0) return PLAT_TRUE;
		else return PLAT_FALSE;
	}
	else if(type == KBUF_BIG_TYPE)
	{
		if (kbuf_big_pool->blk_rem_num == 0) return PLAT_TRUE;
		else return PLAT_FALSE;
	}
	else
	{
	  	return PLAT_FALSE;
	}
}

int kbuf_get_num(uint8_t type)
{
	if (type == KBUF_SMALL_TYPE)
	{
		return kbuf_small_pool->blk_rem_num;
	}
	else if(type == KBUF_BIG_TYPE)
	{
		return kbuf_big_pool->blk_rem_num;
	}
	else
	{
	  	return -1;
	}
}

