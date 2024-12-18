#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "render.h"
#include "menu.h"

char* create_card_str(char* name) {
    char* res = (char*)malloc(21);
    if (res == NULL) {
        printf("Ошибка выделения памяти.\n");
        exit(1);
    }

    if (strlen(name) > 20) {
        strncpy(res, name, 17);
        for (int i = 17; i < 20; i++) res[i] = '.';
        res[20] = '\0'; 
    }
    else if (strlen(name) == 20) {
        return name;
    }
    else {
        strcpy(res, name);
        for (int i = strlen(name); i < 20; i++) res[i] = ' ';
        res[20] = '\0'; 
    }

    return res;
}


char* create_card_int(int num) {
    char* res = (char*)malloc(21);
    if (res == NULL) {
        printf("Ошибка выделения памяти.\n");
        return NULL;
    }

    char nm[21]; 
    sprintf(nm, "%i", num); 

    strcpy(res, nm); 
    for (int i = strlen(nm); i < 20; i++) res[i] = ' '; 
    res[20] = '\0'; 

    return res; 
}


char* create_card_float(float num) {
    char* res = (char*)malloc(21);
    if (res == NULL) {
        printf("Ошибка выделения памяти.\n");
        return NULL;
    }

    char nm[21];
    sprintf(nm, "%f", num);

    strcpy(res, nm);
    for (int i = strlen(nm); i < 20; i++) res[i] = ' ';
    res[20] = '\0';

    return res;
}


void render_film(film* active_film) {
    system("cls");

    printf(
        "+=============================================================================+\n"
        "|1 - вперед 2 - назад 3 - выбрать фильм                                       |\n"
        "|4 - добавить в избранное 5 - выход                                           |\n"
        "+=============================================================================+\n"
        "|                            /``````````````````\\                             |\n"
        "|   +====================+  +====================+  +====================+    |\n"
        "|   |                    |  |                    |  |                    |    |\n"
        "|   |%s|  |%s|  |%s|    |\n"
        "|   |%s|  |%s|  |%s|    |\n"
        "|   |%s|  |%s|  |%s|    |\n"
        "|   |%s|  |%s|  |%s|    |\n"
        "|   |%s|  |%s|  |%s|    |\n"
        "|   |                    |  |                    |  |                    |    |\n"
        "|   +====================+  +====================+  +====================+    |\n"
        "|                           \\___________________/                             |\n"
        "+=============================================================================+\n",
        create_card_str(active_film->previous->name),
        create_card_str(active_film->name),
        create_card_str(active_film->next->name),
        create_card_int(active_film->previous->year_create),
        create_card_int(active_film->year_create),
        create_card_int(active_film->next->year_create),
        create_card_str(active_film->previous->country),
        create_card_str(active_film->country),
        create_card_str(active_film->next->country),
        create_card_str(active_film->previous->genre),
        create_card_str(active_film->genre),
        create_card_str(active_film->next->genre),
        create_card_float(active_film->previous->rating),
        create_card_float(active_film->rating),
        create_card_float(active_film->next->rating)
    );
}
/*
+===========================================================================+
|1 - вперед 2 - назад 3 - выбрать фильм                                     |
|4 - добавить в избранное 5 - выход                                         |
+===========================================================================+
|                            /`````````````````\                            |
|   +====================+  +===================+  +===================+    |
|   |asklfjkasdjfklsdl...|  |                   |  |                   |    |
|   |                    |  |                   |  |                   |    |
|   |                    |  |                   |  |                   |    |
|   |                    |  |                   |  |                   |    |
|   |                    |  |                   |  |                   |    |
|   |                    |  |                   |  |                   |    |
|   |                    |  |                   |  |                   |    |
|   +====================+  +===================+  +===================+    |
|                            \_________________/                            |
+===========================================================================+
*/