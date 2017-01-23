#ifndef  __QUEUE_H
#define  __QUEUE_H

typedef struct
{
    uint8_t data;
} queue_data_t;

typedef struct
{
    queue_data_t *base;
	uint32_t length;
    uint32_t front;
    uint32_t rear;
} queue_t;

bool_t queue_init(queue_t *queue_ptr, uint32_t queue_size);
uint32_t queue_length(queue_t *queue_ptr);
bool_t enter_queue(queue_t *queue_ptr, queue_data_t e);
bool_t delete_queue(queue_t *queue_ptr, queue_data_t *e);

#endif