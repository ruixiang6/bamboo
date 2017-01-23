#ifndef __LIST_H
#define __LIST_H

typedef struct __list
{
	struct __list *next;
	struct __list *prev;
}list_t;


#define list_entry(ptr, type, member) \
        ((type *)((uint8_t *)(ptr)-(uintptr_t)(&((type *)NULL)->member)))

void list_init(list_t *head);
/**
 * dlist_empty - tests whether a dlist is empty
 *
 * @param  head: the dlist to test.
 *
 * @return  TRUE on dlist empty, FALSE on dlist not empty
 */
inline bool_t list_empty(list_t *head);
/**
 * dlist_front_put - add a new entry
 * Insert a new entry after the specified position.
 * This is good for more flexible inplementations which requires add a new entry at an arbitrary place in a list.
 *
 * @param  new_entry: new entry to be added front this pos
 * @param  pos: the position in a dlist
 */
inline void list_front_put(list_t *new_entry, list_t *pos);
/**
 * dlist_behind_put - add a new entry
 * Insert a new entry after the specified position.
 * This is good for more flexible inplementations which requires add a new entry at an arbitrary place in a list.
 *
 * @param  new_entry: new entry to be added behind this pos 
 * @param  pos: the position in a list
 */
void list_behind_put(list_t *new_entry, list_t *pos);
/**
 * dlist_del - deletes entry from list.
 * Note: dlist_empty on entry does not return true after this, the entry is in an undefined state.
 *
 * @param  entry: the element to be deleted from the dlist.
 */
inline void list_del(list_t *entry);
/*
 * Delete and return a element from dlist pos front
 */
inline list_t *list_front_get(list_t *pos);
/*
 * Delete and return a element from dlist pos behind
 */
inline list_t *list_behind_get(list_t *pos);
/**
 * count number of entries in the list
 *
 * @return  count is the inexplicit return value
 */
inline uint16_t list_count(list_t *head);

#define list_entry(ptr, type, member) \
        ((type *)((uint8_t *)(ptr)-(uintptr_t)(&((type *)NULL)->member)))

#define list_entry_front_get(head, type, member) \
	((list_empty(head))?(type *)NULL:(list_entry(list_front_get(head), type, member)))

#define list_entry_behind_get(head, type, member) \
	((list_empty(head))?(type *)NULL:(list_entry(list_behind_get(head), type, member)))

#define list_sorted_add(new_entry,head,type,member,func,pos) \
do {\
	for ((pos) = (head)->next; (pos) != (head); (pos) = (pos)->next){\
		if (func(list_entry((new_entry), type, member),list_entry(pos, type, member))){\
			break;\
		}\
	}\
	list_behind_put(new_entry, pos);\
} while(0)

/**
 * list_for_each_entry	-	iterate over list of given type
 *
 * @param  pos: the type * to use as a loop counter.
 * @param  head:	the head for your list.
 * @param  type:	the type of the struct this is embedded in.
 * @param  member:	the name of the list_struct within the struct.
 */
#define list_for_each_entry(pos, head, type, member)				\
			for ((pos) = list_entry((head)->next, type, member);	\
				 &(pos)->member != (head);					\
				 (pos) = list_entry((pos)->member.next, type, member))


#endif





