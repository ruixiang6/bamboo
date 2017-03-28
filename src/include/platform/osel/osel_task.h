/**
 * OSEL Task Management Routings
 *
 * @file osel_task.h
 * @author qhw (2013-3-11)
 *
 * @addtogroup OSEL_TASK OSEL Task Management
 * @ingroup OSEL
 * @{
 */
#ifndef __OSEL_TASK_H
#define __OSEL_TASK_H

#include <os_cfg.h>

//最大任务数
#define OSEL_TASK_MAX				OS_MAX_TASKS
//任务最大堆栈容量
#define OSEL_TASK_STACK_SIZE_MAX	1024
//最低优先级
#define OSEL_TASK_PRIO_LOWEST		OS_LOWEST_PRIO
//最高优先级
#define OSEL_TASK_PRIO_HIGHEST		0
#define OSEL_TASK_PRIO_ORDER		1
#define OSEL_TASK_PRIO(x) \
	(OSEL_TASK_PRIO_HIGHEST + x * OSEL_TASK_PRIO_ORDER)

#define OSEL_TASK_RETURN_TYPE	void
#define OSEL_TASK_PARAM_TYPE	void *

/**
 * OSEL task data type
 */
#pragma pack(4)
typedef struct
{
	uintptr_t handle;		/* task handle */
    uint16_t prio;           /* task prio */
	void *stack;			/* task stack */
    uint32_t stack_size;    /* task stack size */
} osel_task_t;
#pragma pack()

/**
 * Init a task
 */
bool_t osel_task_init(void);

/**
 * Macro for task declaration
 * @note This macro should always been used for task
 *  	 declaration.
 *
 * @b Example:
 * @code
OSEL_DECLARE_TASK(task1, param)
{
  	while (1)
 	{
 		do_something();
	}
}
 * @endcode
 */
#define OSEL_DECLARE_TASK(taskcode, param) \
   static OSEL_TASK_RETURN_TYPE taskcode(OSEL_TASK_PARAM_TYPE param)

/**
 * Create a task
 *
 * @param taskcode task routing
 * @param param task routing parameter
 * @param stacksize task stack depth, the actual stack size
 *  				equals (stack_depth * stack_element_width).
 *  				For example, on 32bit cpu, stack size is
 *  				stack_depth * 4
 * @param prio task priority
 * @param task pointer of task data structure
 *
 * @return osel_task_t when success, NULL when fail
 */
osel_task_t *osel_task_create(OSEL_TASK_RETURN_TYPE (*taskcode)(OSEL_TASK_PARAM_TYPE),
						OSEL_TASK_PARAM_TYPE param,
						uint32_t stack_depth,
						uint16_t prio
						);

/**
 * Delete a task
 *
 * @param task pointer of task data struct
 */
bool_t osel_task_delete(osel_task_t *task);

/**
 * Suspend a task
 *
 * @param task pointer of task data struct
 */
bool_t osel_task_suspend(osel_task_t *task);

/**
 * Resume a suspended task
 *
 * @param task pointer of task data struct
 */
bool_t osel_task_resume(osel_task_t *task);

/**
 * Return a current task
 *
 * @param task pointer of task data struct
 */
bool_t osel_task_query(osel_task_t *task);

void osel_task_idle_hook_reg(uint8_t index, fpv_t func);

#endif

/**
 * @}
 */
