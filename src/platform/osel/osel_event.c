#include <ucos_ii.h>
#include <platform.h>

static pool_t *event_pool;
static pool_t *event_q_pool;

bool_t osel_event_init(void)
{
	event_pool = pool_create(OSEL_EVENT_MAX,
							sizeof(osel_event_t));
	if (event_pool == PLAT_NULL) return PLAT_FALSE;

	event_q_pool = pool_create(OSEL_EVENT_Q_MAX,
							sizeof(void *)*OSEL_EVENT_Q_NUM_MAX);

	if (event_q_pool == PLAT_NULL) return PLAT_FALSE;

	return PLAT_TRUE;	
}


/*
 * In this implementation, use semaphore or mailbox as sync object of event
 */

osel_event_t *osel_event_create(osel_event_type_t type, uint32_t param)
{
	osel_event_t *event;
	
    event = (osel_event_t *)pool_alloc(event_pool);
   
	if (event == NULL)
	{
        return PLAT_NULL;
	}
	
	if (type == OSEL_EVENT_TYPE_SEM)
    {
        event->handle = (uintptr_t)OSSemCreate((uint16_t)param);
		event->type = OSEL_EVENT_TYPE_SEM; 
    }
    else if (type == OSEL_EVENT_TYPE_MSG)
    {               
		if (param>OSEL_EVENT_Q_NUM_MAX)
        {
            param = OSEL_EVENT_Q_NUM_MAX;
        }
		event->object.msg.base = pool_alloc(event_q_pool);
		if (event->object.msg.base == NULL)
        {            
            return PLAT_NULL;
        }
		event->object.msg.num = param;
        event->handle = (uintptr_t)OSQCreate(event->object.msg.base, param);
		event->type = OSEL_EVENT_TYPE_MSG; 
    }
    else
    {        
        return PLAT_NULL;
    }
	//check create result
    if (event->handle == NULL)
    {
        
        return PLAT_NULL;
    }
    else 
    {
        return event;
    }
}

osel_event_res_t osel_event_wait(osel_event_t *event, uint32_t timeout)
{
    uint8_t err;
	
	if (event == PLAT_NULL)
    {
        return OSEL_EVENT_ERR;
    }
    
    if (event->type == OSEL_EVENT_TYPE_SEM)
	{
        if (event->object.sem.ext_data == OSEL_EVENT_NONE)
        {
            //ms to tick
            if (timeout) timeout = (timeout*OS_TICKS_PER_SEC)/1000;           
            OSSemPend((OS_EVENT *)event->handle, timeout, &err);
            if (err == OS_ERR_NONE) return OSEL_EVENT_NONE;
            if (err == OS_ERR_TIMEOUT) return OSEL_EVENT_TIMEOUT;
            if (err == OS_ERR_PEND_ABORT) return OSEL_EVENT_ABORT;            
        }
        else return OSEL_EVENT_NONE;
	}
    else if (event->type == OSEL_EVENT_TYPE_MSG)
    {
        if (event->object.msg.fetch == NULL)
        {
            //ms to tick
            if (timeout) timeout = (timeout*OS_TICKS_PER_SEC)/1000+1;
            event->object.msg.fetch = 
                OSQPend ((OS_EVENT *)event->handle, timeout, &err);
            if (err == OS_ERR_NONE) return OSEL_EVENT_NONE;
            if (err == OS_ERR_TIMEOUT) return OSEL_EVENT_TIMEOUT;
            if (err == OS_ERR_PEND_ABORT) return OSEL_EVENT_ABORT;            
        }
        else return OSEL_EVENT_NONE;
    }
    
    return OSEL_EVENT_ERR;    
}

osel_event_res_t osel_event_set(osel_event_t *event, void *object)
{
	uint8_t err, offset; 
    uint16_t bitset;

	if (event == PLAT_NULL) 
	{
		return OSEL_EVENT_ERR;
	}

	if (event->type == OSEL_EVENT_TYPE_SEM)
	{
		if (object != PLAT_NULL)
        {
            bitset = *(uint16_t *)object;
		    offset = 0;
        }
        else
        {
            bitset = 0;
            offset = 0;
        }
        
		if (bitset != 0)
		{
			while(!(bitset>>(offset++)&1u));
			event->object.sem.ext_cnt[offset-1]++;
			event->object.sem.ext_data |= bitset;
		}
		err = OSSemPost((OS_EVENT *)event->handle);
		if (err == OS_ERR_NONE) return OSEL_EVENT_NONE;
	}
	else if (event->type == OSEL_EVENT_TYPE_MSG)
	{
		if (object != PLAT_NULL)
		{
			err = OSQPostOpt((OS_EVENT *)event->handle, 
                            object,
                            OS_POST_OPT_BROADCAST);
			if (err == OS_ERR_NONE) return OSEL_EVENT_NONE;
			if (err == OS_ERR_Q_FULL) return OSEL_EVENT_FULL;
		}
		
	}
    
	return OSEL_EVENT_ERR;	
}

osel_event_res_t osel_event_clear(osel_event_t *event, void *object)
{
	uint8_t offset; 
    uint16_t bitclr;

	if (event == PLAT_NULL)
	{
		return OSEL_EVENT_ERR;
	}

	if (event->type == OSEL_EVENT_TYPE_SEM)
	{
        if (object == PLAT_NULL)
        {
            return OSEL_EVENT_NONE;
        }
        
        bitclr = *(uint16_t *)object;
		offset = 0;
		
        if (bitclr != 0)
		{
			while(!(bitclr>>(offset++)&1u));
			if (event->object.sem.ext_cnt[offset-1])
			{
				event->object.sem.ext_cnt[offset-1]--;
			}
			if (event->object.sem.ext_cnt[offset-1] == 0)
			{
				event->object.sem.ext_data &= ~bitclr;
			}
		}	
		return OSEL_EVENT_NONE;
	}
	else if (event->type == OSEL_EVENT_TYPE_MSG)
	{
		event->object.msg.fetch = PLAT_NULL;
		return OSEL_EVENT_NONE;
	}

	return OSEL_EVENT_ERR;
}

osel_event_res_t osel_event_delete(osel_event_t *event)
{
	uint8_t err;
	
	if (event == PLAT_NULL)
	{
		return OSEL_EVENT_ERR;
	}

	if (event->type == OSEL_EVENT_TYPE_SEM)
	{
		OSSemDel ((OS_EVENT *)event->handle, OS_DEL_ALWAYS, &err);
		pool_free(event_pool, event);		
		return OSEL_EVENT_NONE;
	}
	else if (event->type == OSEL_EVENT_TYPE_MSG)
	{
		OSQDel((OS_EVENT *)event->handle, OS_DEL_ALWAYS, &err);
		pool_free(event_q_pool, event->object.msg.base);
		pool_free(event_pool, event);
		return OSEL_EVENT_NONE;
	}

	return OSEL_EVENT_ERR;
}
