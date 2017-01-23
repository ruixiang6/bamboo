#include <platform.h>

void list_init(list_t *head)
{
	head->next = head;
	head->prev = head;
}
/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal dlist manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add(list_t *new_entry,
			      				list_t *prev,
			      				list_t *next)
{
	next->prev = new_entry;
	new_entry->next = next;
	new_entry->prev = prev;
	prev->next = new_entry;
}

/*
 * Delete a dlist entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal dlist manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(list_t *prev, list_t *next)
{
	next->prev = prev;
	prev->next = next;
}

void list_front_put(list_t *new_entry, list_t *pos)
{
	__list_add(new_entry, (pos), (pos)->next);
}

void list_behind_put(list_t *new_entry, list_t *pos)
{
	__list_add(new_entry, (pos)->prev, (pos));
}

void list_del(list_t *entry)
{
	if (entry == NULL && list_empty(entry))
	{
		return;
	}
	
	__list_del(entry->prev, entry->next);
	entry->next = (list_t *) NULL;
	entry->prev = (list_t *) NULL;
}

bool_t list_empty(list_t *head)
{
	return (((head)->next == head) || (head->next == NULL));
}

list_t *list_front_get(list_t *pos)
{
	if (pos == NULL || list_empty(pos))
	{
		return NULL;
	}
	
	list_t *temp = (pos)->next;
	
	__list_del(pos, temp->next);
	temp->next = (list_t *) NULL;
	temp->prev = (list_t *) NULL;

	return temp;
}

list_t *list_behind_get(list_t *pos)
{
	if (pos == NULL || list_empty(pos))
	{
		return NULL;
	}
	
	list_t *temp = (pos)->prev;
	
	__list_del(temp->prev, pos);
	temp->next = (list_t *) NULL;
	temp->prev = (list_t *) NULL;

	return temp;
}

uint16_t list_count(list_t *head)
{
    uint16_t count=0;
    
	for (list_t *pos = (head)->next; pos != (head); pos = pos->next)
    {
        count++;
    }
    
    return count;
}
	




