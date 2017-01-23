#include <platform.h>

bool_t queue_init(queue_t *queue_ptr, uint32_t queue_size)
{
    queue_ptr->base = (queue_data_t *)heap_alloc(queue_size * sizeof(queue_data_t), PLAT_TRUE);

	if (queue_ptr->base == NULL)
    {
        return PLAT_FALSE;
    }
    queue_ptr->length = queue_size;
    queue_ptr->front = queue_ptr->rear = 0;

	return PLAT_TRUE;
}

uint32_t queue_length(queue_t *queue_ptr)
{
    uint32_t length;
	OSEL_DECL_CRITICAL();

	OSEL_ENTER_CRITICAL();
    length = (queue_ptr->rear - queue_ptr->front + queue_ptr->length) % queue_ptr->length;
    OSEL_EXIT_CRITICAL();

	return length;    
}

bool_t enter_queue(queue_t *queue_ptr, queue_data_t e)
{
	OSEL_DECL_CRITICAL();
	
    OSEL_ENTER_CRITICAL();
    if ((queue_ptr->rear + 1) % queue_ptr->length == queue_ptr->front)
    {
		OSEL_EXIT_CRITICAL();
        return PLAT_FALSE;
    }
    queue_ptr->base[queue_ptr->rear] = e;
    queue_ptr->rear = ( queue_ptr->rear + 1) % queue_ptr->length;
    OSEL_EXIT_CRITICAL();
    return PLAT_TRUE;
}

bool_t delete_queue(queue_t *queue_ptr, queue_data_t *e)
{
    OSEL_DECL_CRITICAL();
	
    OSEL_ENTER_CRITICAL();
    if (queue_ptr->rear == queue_ptr->front)
    {
		OSEL_EXIT_CRITICAL();
        return PLAT_FALSE;
    }
    OSEL_EXIT_CRITICAL();
    *e = queue_ptr->base[queue_ptr->front];
    queue_ptr->front = (queue_ptr->front + 1) % queue_ptr->length;
    return PLAT_TRUE;
}