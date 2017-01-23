#ifndef __KBUF_H
#define __KBUF_H

#define KBUF_BIG_TYPE		(0u<<7)
#define KBUF_SMALL_TYPE		(1u<<7)

#define KBUF_BIG_SIZE		2700
#define KBUF_BIG_NUM		2048

#define KBUF_SMALL_SIZE		256
#define KBUF_SMALL_NUM		256

#pragma pack(1)

typedef struct
{
	list_t list;	
	uint8_t *base;
	uint8_t *offset;
	uint16_t type;
	uint16_t valid_len;
	void *priv;
}kbuf_t;

#pragma pack()

bool_t kbuf_init(void);
kbuf_t *kbuf_alloc(uint8_t type);
bool_t kbuf_free(kbuf_t *kbuf);
bool_t kbuf_is_empty(uint8_t type);
int kbuf_get_num(uint8_t type);

#endif
