#include <ucos_ii.h>
#include <platform.h>

#define STK_MEM_SIZE	(16*1024)
#define STK_MEM_WORD_SIZE	(STK_MEM_SIZE/sizeof(OS_STK))

/*任务管理池*/
static pool_t *task_pool;
/*任务堆栈管理*/
static OS_STK* task_heap_free_ptr;
static uint32_t task_heap_free_size;
/*任务堆栈*/
#pragma location = ".task_stk"
__no_init static OS_STK task_stk[STK_MEM_WORD_SIZE];

bool_t osel_task_init(void)
{
	task_pool = pool_create(OSEL_TASK_MAX,	sizeof(osel_task_t));
	if (task_pool == PLAT_NULL) 
	{
		return PLAT_FALSE;
	}
	
	task_heap_free_ptr = task_stk;
	task_heap_free_size = STK_MEM_WORD_SIZE;

	return PLAT_TRUE;
}

osel_task_t *osel_task_create(OSEL_TASK_RETURN_TYPE (*taskcode)(OSEL_TASK_PARAM_TYPE),
						OSEL_TASK_PARAM_TYPE param,
						uint32_t stack_depth,
						uint16_t prio
						)
{
    OSEL_DECL_CRITICAL();
    
    OSEL_ENTER_CRITICAL();
	osel_task_t *task;	
	
	if (taskcode == PLAT_NULL)
	{
		return PLAT_NULL;
	}
	// alloc task pool
	task = (osel_task_t *)pool_alloc(task_pool);

	if (task == PLAT_NULL)
	{
        OSEL_EXIT_CRITICAL();
		return PLAT_NULL;
	}
	
	/* Allocate stack space */
	if (stack_depth%sizeof(OS_STK) == 0 && task_heap_free_size >= stack_depth)
	{
		task->stack = task_heap_free_ptr;

		task_heap_free_ptr = task_heap_free_ptr+stack_depth;
		task_heap_free_size -= stack_depth;
	}
	else
	{
		pool_free(task_pool, (void *)task);
        OSEL_EXIT_CRITICAL();
		return PLAT_NULL;	
	}
	
	task->prio = prio;	
    task->stack_size = stack_depth;
	if (task->stack_size>OSEL_TASK_STACK_SIZE_MAX)
	{
		task->stack_size = OSEL_TASK_STACK_SIZE_MAX;
	}

	/* Create task */
	if (OSTaskCreateExt(taskcode, 
					   param, 
					   (OS_STK *)task->stack + stack_depth - 1, 
					   (uint8_t)prio,
					   (uint16_t)prio,
					   (OS_STK *)task->stack,
					   stack_depth,
					   NULL,
					   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR) == OS_ERR_NONE)
    {
    	task->handle = (uintptr_t)prio;
        OSEL_EXIT_CRITICAL();
        return task;
    }
	else
	{
		pool_free(task_pool, (void *)task);
        OSEL_EXIT_CRITICAL();
		return PLAT_NULL;
	}
}

bool_t osel_task_delete(osel_task_t *task)
{
    OSEL_DECL_CRITICAL();
    
    OSEL_ENTER_CRITICAL();
	if (task != PLAT_NULL)
	{
        if (pool_free(task_pool, (void *)task) == PLAT_FALSE) 
        {
            OSEL_EXIT_CRITICAL();
            return PLAT_FALSE;
        }			
		
        if (OSTaskDel((uint8_t)(task->handle)) == OS_ERR_NONE)
		{
            OSEL_EXIT_CRITICAL();
			return PLAT_TRUE;
		}
		else
		{
            OSEL_EXIT_CRITICAL();
			return PLAT_FALSE;
		}
	}
	else
	{
        OSEL_EXIT_CRITICAL();
		return PLAT_FALSE;
	}	
}

bool_t osel_task_suspend(osel_task_t *task)
{
	return (OSTaskSuspend((uint8_t)(task->handle)) == OS_ERR_NONE);
}

bool_t osel_task_resume(osel_task_t *task)
{
	return (OSTaskResume((uint8_t)(task->handle)) == OS_ERR_NONE);
}

bool_t osel_task_query(osel_task_t *task)
{
	OS_TCB  tcb;
		
	if (task != PLAT_NULL)
	{
		OSTaskQuery(OS_PRIO_SELF, &tcb);

		task->handle = tcb.OSTCBPrio;
		task->prio = tcb.OSTCBPrio;
		task->stack = tcb.OSTCBStkBottom + tcb.OSTCBStkSize;
		task->stack_size = tcb.OSTCBStkSize*sizeof(OS_STK);

		return PLAT_TRUE;
	}
	else
	{
		return PLAT_FALSE;
	}
}


