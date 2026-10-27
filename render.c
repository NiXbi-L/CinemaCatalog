#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "render.h"

static void clear_screen(void) {
    system("clear");
}

static int utf8_size(const unsigned char* text) {
    if (*text < 0x80)
        return 1;
    if ((*text & 0xE0) == 0xC0)
        return 2;
    if ((*text & 0xF0) == 0xE0)
        return 3;
    if ((*text & 0xF8) == 0xF0)
        return 4;
    return 1;
}

static void pad_card(char* dst, const char* src) {
    const char* text = src == NULL ? "" : src;
    const char* cursor = text;
    size_t used = 0;
    int columns = 0;
    int total = 0;
    int limit = 20;

    while (*cursor != '\0') {
        cursor += utf8_size((const unsigned char*)cursor);
        total++;
    }
    if (total > 20)
        limit = 17;

    cursor = text;
    while (*cursor != '\0' && columns < limit) {
        int size = utf8_size((const unsigned char*)cursor);
        memcpy(dst + used, cursor, (size_t)size);
        used += (size_t)size;
        cursor += size;
        columns++;
    }
    if (total > 20) {
        memcpy(dst + used, "...", 3);
        used += 3;
        columns += 3;
    }
    while (columns < 20) {
        dst[used++] = ' ';
        columns++;
    }
    dst[used] = '\0';
}

static char* alloc_card(void) {
    char* res = malloc(20 * 4 + 1);
    if (res == NULL) {
        printf("Ошибка выделения памяти.\n");
        exit(1);
    }
    return res;
}

char* create_card_str(const char* name) {
    char* res = alloc_card();
    pad_card(res, name);
    return res;
}

char* create_card_int(int num) {
    char* res = alloc_card();
    char nm[21];
    sprintf(nm, "%d", num);
    pad_card(res, nm);
    return res;
}

char* create_card_float(double num) {
    char* res = alloc_card();
    char nm[21];
    sprintf(nm, "%.1f", num);
    pad_card(res, nm);
    return res;
}

void render_film(const FilmNode *active_film) {
    const FilmNode *prev;
    const FilmNode *next;
    char* cells[15];
    int i;

    if (active_film == NULL)
        return;

    prev = active_film->prev != NULL ? active_film->prev : active_film;
    next = active_film->next != NULL ? active_film->next : active_film;
    clear_screen();

    cells[0] = create_card_str(prev->film.title);
    cells[1] = create_card_str(active_film->film.title);
    cells[2] = create_card_str(next->film.title);
    cells[3] = create_card_int(prev->film.year);
    cells[4] = create_card_int(active_film->film.year);
    cells[5] = create_card_int(next->film.year);
    cells[6] = create_card_str(prev->film.country);
    cells[7] = create_card_str(active_film->film.country);
    cells[8] = create_card_str(next->film.country);
    cells[9] = create_card_str(prev->film.genre);
    cells[10] = create_card_str(active_film->film.genre);
    cells[11] = create_card_str(next->film.genre);
    cells[12] = create_card_float(prev->film.rating);
    cells[13] = create_card_float(active_film->film.rating);
    cells[14] = create_card_float(next->film.rating);

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
        cells[0], cells[1], cells[2],
        cells[3], cells[4], cells[5],
        cells[6], cells[7], cells[8],
        cells[9], cells[10], cells[11],
        cells[12], cells[13], cells[14]
    );

    for (i = 0; i < 15; i++)
        free(cells[i]);
}
