#include <platform.h>

#if CFG_DEBUG_EN > 0

#define DBG_BUF_SIZE		128

uint8_t global_debug_level = (uint8_t)DBG_LEVEL_ERROR;

static char_t dbg_buf[DBG_BUF_SIZE];

void plat_dbg_print(uint8_t level, const char_t *fn, uint16_t line, ...)
{
	const char_t *fmt;
	char_t *pbuf = dbg_buf;
	
	va_list args;

	(void)memset(dbg_buf, 0, (size_t)DBG_BUF_SIZE);
		
	switch (level)
	{
	case DBG_LEVEL_TRACE:
		(void)sprintf(pbuf, "[%s, %u][TRACE] ", fn, line);
		break;							   	
	case DBG_LEVEL_INFO:
		(void)sprintf(pbuf, "[%s, %u][INFO ] ", fn, line);
		break;							   	
	case DBG_LEVEL_WARNING:
		(void)sprintf(pbuf, "[%s, %u][WARN ] ", fn, line);
		break;							   	
	case DBG_LEVEL_ERROR:
		(void)sprintf(pbuf, "[%s, %u][ERROR] ", fn, line);
		break;
	default:
		break;
	}
	
	pbuf += strlen((char_t *)dbg_buf);
	
	va_start(args, line);
	fmt = va_arg(args, const char_t*);
	(void)vsprintf(pbuf, fmt, args);
	va_end(args);

	hal_uart_printf(UART_DEBUG, (uint8_t *)dbg_buf);	
}

void plat_dbg_assert(const char_t *cond, const char_t *fn, uint16_t line)
{
	char_t *pbuf = dbg_buf;
	
	(void)memset(dbg_buf, 0, (size_t)DBG_BUF_SIZE);

	(void)sprintf(pbuf, "[%s, %d] ASSERTION FAILED !!! ", fn, line);

	pbuf += strlen((char_t *)dbg_buf);

	(void)sprintf(pbuf, "<%s>\r\n", cond);
	
	hal_uart_printf(UART_DEBUG, (uint8_t *)dbg_buf);

	/*lint -e{716}*/
	while (1) ;
}
#endif
