#ifndef __DA_H
#define __DA_H

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define da(type)                 \
	struct {                 \
		type *items;     \
		size_t count;    \
		size_t capacity; \
	}

#define DA_INIT_CAP 255

#define da_append(da, item)                                                    \
	do {                                                                   \
		if ((da)->count >= (da)->capacity) {                           \
			(da)->capacity = (da)->capacity == 0 ?                 \
						 DA_INIT_CAP :                 \
						 (da)->capacity * 2;           \
			(da)->items = realloc((da)->items,                     \
					      (da)->capacity *                 \
						      sizeof(*(da)->items));   \
			assert((da)->items != NULL && "Could not reallocate"); \
		}                                                              \
                                                                               \
		(da)->items[(da)->count++] = (item);                           \
	} while (0)

#define da_append_many(da, new_items, new_items_count)                         \
	do {                                                                   \
		if ((da)->count + new_items_count > (da)->capacity) {          \
			if ((da)->capacity == 0) {                             \
				(da)->capacity = DA_INIT_CAP;                  \
			}                                                      \
			while ((da)->count + new_items_count >                 \
			       (da)->capacity) {                               \
				(da)->capacity *= 2;                           \
			}                                                      \
			(da)->items = realloc((da)->items,                     \
					      (da)->capacity *                 \
						      sizeof(*(da)->items));   \
			assert((da)->items != NULL && "Could not reallocate"); \
		}                                                              \
		memcpy((da)->items + (da)->count, new_items,                   \
		       new_items_count * sizeof(*(da)->items));                \
		(da)->count += new_items_count;                                \
	} while (0);

#define da_foreach(type, item, da) \
	for (type *item = (da).items; item < (da).items + (da).count; item++)

#endif // __DA_H
