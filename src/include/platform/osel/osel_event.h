/**
 * OSEL Event Management Routines
 *
 * @file osel_event.h
 * @author qhw (2013-3-11)
 *
 * @addtogroup OSEL_EVENT OSEL Event Management
 * @ingroup OSEL
 * @{
 */
#ifndef __OSEL_EVENT_H
#define __OSEL_EVENT_H

#include <platform.h>
#include <os_cfg.h>

//最大事件数量
#define OSEL_EVENT_MAX          OS_MAX_EVENTS
//消息队列总长
#define OSEL_EVENT_Q_MAX        OS_MAX_QS
//每一个队列包含的消息数量
#define OSEL_EVENT_Q_NUM_MAX	16
//信号量外部数据数量
#define OSEL_EVENT_DATA_NUM		16
//事件永久等待
#define OSEL_WAIT_FOREVER       0
//事件消息
#define OSEL_EVENT_NONE         0
#define OSEL_EVENT_TIMEOUT      1
#define OSEL_EVENT_ABORT        2
#define OSEL_EVENT_ERR          3
#define OSEL_EVENT_FULL			4

/**
 * OSEL event data type
 */
typedef uint16_t osel_event_res_t;

typedef enum
{
    OSEL_EVENT_TYPE_MSG = 1,
    OSEL_EVENT_TYPE_SEM    
}osel_event_type_t;

/**
 * OSEL event control block
 */
#pragma pack(4)
typedef struct
{
	uintptr_t handle;    
    uint16_t type;
    union
    {
        struct
		{
			void* fetch;
			void** base;			
			uint32_t num;
		}msg;
        struct  
        {
            uint16_t ext_data;
            uint16_t ext_cnt[OSEL_EVENT_DATA_NUM];    
        }sem;
    }object;    
} osel_event_t;
#pragma pack()

#define OSEL_EVENT_GET(event, param_type) \
	((event->type == OSEL_EVENT_TYPE_MSG)?(param_type)(event->object.msg.fetch):(param_type)(event->object.sem.ext_data))

/**
 * Init event control block
 *  
 * @return on success, on failure
 */
bool_t osel_event_init(void);

/**
 * Create event control block
 *  
 * @param event pointer to event to be initialized
 * @param event type type  
 *
 * @return osel_event_t on success, NULL on failure
 */
osel_event_t *osel_event_create(osel_event_type_t type, uint32_t param);

/**
 * Wait on event with timeout
 *  
 * @param event pointer to event to wait on
 * @param timeout is ms unit
 * 
 * @return OSEL_EVENT_NONE
 *  	   OSEL_EVENT_TIMEOUT 
 *         OSEL_EVENT_ERR 
 */
osel_event_res_t osel_event_wait(osel_event_t *event, uint32_t timeout);

/**
 * Set event bits
 *  
 * @param event pointer to event to be set
 * @param object is set with different event type define
 */
osel_event_res_t osel_event_set(osel_event_t *event, void *object);

/**
 * Clear event bits
 *  
 * @param event pointer to event to be clear
 * @param object is clear with different event type define
 */
osel_event_res_t osel_event_clear(osel_event_t *event, void *object);

/**
 * Delete event
 *  
 * @param event pointer to event to be delete 
 */
osel_event_res_t osel_event_delete(osel_event_t *event);

#endif
/**
 * @}
 */
