#include <platform.h>
#include <lib_mem.h>

void *memory_init(uint32_t *mem_total_size)
{
    Mem_Init();
    
    if (mem_total_size != NULL) *mem_total_size = LIB_MEM_CFG_HEAP_SIZE;
    return PLAT_NULL;
}

void *heap_alloc(uint32_t size, bool_t init)
{
    CPU_SIZE_T octets_reqd;
    LIB_ERR err;
    void *ptr;
    
    ptr = Mem_HeapAlloc (size, sizeof(CPU_ALIGN), &octets_reqd, &err);
    
    if (err != LIB_MEM_ERR_NONE) return PLAT_NULL;
	else
	{
		if (init)
		{
			mem_clr(ptr, size);
		}		
		return ptr;
	}		
}

pool_t *pool_create(size_t blk_num, uint32_t blk_size)
{
	CPU_SIZE_T octets_reqd;
    LIB_ERR err;
	pool_t *pool;

	pool = Mem_HeapAlloc (sizeof(pool_t)+sizeof(MEM_POOL), sizeof(CPU_ALIGN), &octets_reqd, &err);
	if (err != LIB_MEM_ERR_NONE) return PLAT_NULL;
	
	mem_set(pool, 0, sizeof(pool_t));
	
	pool->ptr = (uint8_t *)pool + sizeof(pool_t);
	Mem_PoolCreate ((MEM_POOL *)pool->ptr,
						NULL,
                        blk_num*blk_size,
                        blk_num,
                        blk_size,
                        sizeof(CPU_ALIGN),
                        &octets_reqd,
                        &err);
	if (err != LIB_MEM_ERR_NONE) return PLAT_NULL;
	pool->blk_size = blk_size;
	pool->blk_total_num = blk_num;
	pool->blk_rem_num = blk_num;

	return pool;
}

void *pool_alloc(pool_t *pool)
{
	LIB_ERR err;
	void *ptr;

	ptr = Mem_PoolBlkGet ((MEM_POOL*)pool->ptr,
                           pool->blk_size,
                           &err);
	if (err != LIB_MEM_ERR_NONE) return PLAT_NULL;
    mem_clr(ptr, pool->blk_size);
    pool->blk_rem_num--;

	return ptr;
}

bool_t pool_free(pool_t *pool, void *ptr)
{
	LIB_ERR err;
	
	Mem_PoolBlkFree((MEM_POOL *)pool->ptr,
                      ptr,
                      &err);
	if (err != LIB_MEM_ERR_NONE) return PLAT_FALSE;
    
	pool->blk_rem_num++;

	return PLAT_TRUE;
}

void mem_set(void *buffer, uint8_t c, uint32_t count)
{
	Mem_Set (buffer, c, count);	
}

void mem_clr(void *buffer, uint32_t count)
{
	Mem_Clr(buffer, count);	
}

void mem_cpy(void *dest, void *src, uint32_t count)
{
	Mem_Copy(dest, src, count);	
}