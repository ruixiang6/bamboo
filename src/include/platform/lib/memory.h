/** 
 * Memory Management Routines 
 *  
 * @file memory.h  
 * @author herry
 *  
 * @addtogroup MEM	Memory Management 
 * @ingroup  
 * @{ 
 */
#ifndef __MEMORY_H
#define __MEMORY_H

typedef struct
{
	void *ptr;	
	uint32_t blk_size;
	uint32_t blk_total_num;
	uint32_t blk_rem_num;
}pool_t;


void *memory_init(uint32_t *mem_total_size);

/**
 * Allocate memory block from heap.
 *  
 * @param size size of memory to allocate
 * 
 * @return pointer of memory allocated on success, NULL on 
 *  	   failure
 */
void *heap_alloc(uint32_t size, bool_t init_flag);

/**
 * Create pool from heap and make pool element.
 * 
 * @param blk_num block number
 * @param blk_size block size
 * 
 * @return pointer of memory allocated on success, NULL on fail
 */
pool_t *pool_create(size_t blk_num, uint32_t blk_size);

/**
 * Allocate pool block.
 *  
 * @param pool pointer
 * 
 * @return pointer of memory allocated on success, NULL on fail
 */
void *pool_alloc(pool_t *pool);

/**
 * Free pool block.
 *  
 * @param pool pointer
 * 
 * @return success on TRUE, and error on FAIL
 */
bool_t pool_free(pool_t *pool, void *ptr);

/**
 * set a memory block with int8 value c
 *  
 * @param mem pointer to memory block
 * 
 * @return pointer of the buffer 
 */
void mem_set(void *buffer, uint8_t c, uint32_t count);

/**
 * clear a memory block 
 *  
 * @param mem pointer to memory block
 * 
 * @return pointer of the buffer 
 */
void mem_clr(void *buffer, uint32_t count);


/**
 * copy src memory block to dest memory block
 *  
 * @param mem pointer to memory block
 * 
 * @return pointer of the dest
 */
void mem_cpy(void *dest, void *src, uint32_t count);

#endif
/**
 * @}
 */
