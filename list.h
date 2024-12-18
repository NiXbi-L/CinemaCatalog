#pragma once
#include "list.h"
#include "menu.h"
#include <stdlib.h>
#include <stdio.h>

#define CREATE_FUNCTION(type, func_name) \
    type func_name(type iterator,type New) {         \
        for (iterator; ;iterator = iterator->next){\
            if (iterator->next != NULL){\
                iterator->next = New;\
                break;\
            }\
        }\
    }


CREATE_FUNCTION(film*, film_add_end);
CREATE_FUNCTION(user*, user_add_end);
