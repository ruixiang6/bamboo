/**
 * SenStack Debug Routines
 *
 * @file
 * @author herry
 *
 * @addtogroup LIB_DEBUG Debug Routines
 * @ingroup LIB
 * @{
 */
#ifndef __DEBUG_H
#define __DEBUG_H


/**
 * Debug level difinitions 
 */
#define DBG_LEVEL_PRINTF	0x00
#define DBG_LEVEL_TRACE		0x01
#define DBG_LEVEL_INFO		0x02
#define DBG_LEVEL_WARNING	0x03
#define DBG_LEVEL_ERROR		0x04
#define DBG_LEVEL_CRITICAL	0x05

#if CFG_DEBUG_EN > 0

/**
 * Global debug level setting, only messages with higher levels
 * than global_debug_level will be output
 */
extern uint8_t global_debug_level;

void plat_dbg_print(uint8_t level, const char_t *fn, uint16_t line, ...);
void plat_dbg_assert(const char_t *cond, const char_t *fn, uint16_t line);

#define DBG_SET_LEVEL(level) 					\
	do											\
	{											\
		global_debug_level = (uint8_t)level;	\
	} while (__LINE__ == -1)

/*lint -e717*/
#define DBG_PRINT(level, ...)													\
	do																			\
	{																			\
		if ((uint8_t)level >= global_debug_level)								\
		{																		\
			plat_dbg_print((uint8_t)level, __FILE__, (uint16_t)__LINE__, __VA_ARGS__);	\
		}																		\
	} while (__LINE__ == -1)


#define DBG_PRINTF(...) 	DBG_PRINT(DBG_LEVEL_PRINTF, __VA_ARGS__)
#define DBG_TRACE(...) 		DBG_PRINT(DBG_LEVEL_TRACE, __VA_ARGS__)
#define DBG_INFO(...)		DBG_PRINT(DBG_LEVEL_INFO, __VA_ARGS__)
#define DBG_WARNING(...)	DBG_PRINT(DBG_LEVEL_WARNING, __VA_ARGS__)
#define DBG_ERROR(...)		DBG_PRINT(DBG_LEVEL_ERROR, __VA_ARGS__)
#define DBG_CRITICAL(...)	DBG_PRINT(DBG_LEVEL_CRITICAL, __VA_ARGS__)



/*lint -e717*/
#define DBG_ASSERT(cond)								\
	do													\
	{													\
		if (!(cond))									\
		{												\
			plat_dbg_assert(#cond, __FILE__, (uint16_t)__LINE__);	\
		}												\
	} while (__LINE__ == -1)

#else

#define DBG_SET_LEVEL(level)
#define DBG_PRINT(level,  ...)  
#define DBG_TRACE(...)
#define DBG_INFO(...)
#define DBG_WARNING(...)
#define DBG_ERROR(...)
#define DBG_CRITICAL(...)
#define DBG_ASSERT(cond)

#endif /* #if CFG_DEBUG_EN > 0 */

#endif
