#include "list.h"
#include <stdlib.h>
#include <stdio.h>

#define CREATE_FUNCTION(type, func_name) \
    type func_name(type iterator, type New) { \
        type head = iterator; \
        if (New != NULL) { \
            New->next = NULL; \
            New->previous = NULL; \
        } \
        if (iterator == NULL) \
            return New; \
        while (iterator->next != NULL) \
            iterator = iterator->next; \
        iterator->next = New; \
        if (New != NULL) \
            New->previous = iterator; \
        return head; \
    }

CREATE_FUNCTION(film*, film_add_end);
CREATE_FUNCTION(user*, user_add_end);
