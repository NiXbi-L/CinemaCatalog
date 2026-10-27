#include "cinema.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int is_latin_alnum(unsigned char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}

static void copy_text(char *dst, size_t dst_size, const char *src) {
    size_t i = 0;

    if (dst_size == 0)
        return;
    if (src != NULL) {
        while (i + 1 < dst_size && src[i] != '\0') {
            dst[i] = src[i];
            i++;
        }
    }
    dst[i] = '\0';
}

static int read_line(FILE *fp, char *buf, size_t cap) {
    char raw[512];
    size_t len;
    int newline = 0;

    if (fgets(raw, (int)sizeof raw, fp) == NULL)
        return 0;

    len = strlen(raw);
    while (len > 0 && (raw[len - 1] == '\n' || raw[len - 1] == '\r')) {
        newline = 1;
        raw[--len] = '\0';
    }
    if (!newline && len + 1 == sizeof raw) {
        int ch;
        while ((ch = fgetc(fp)) != EOF && ch != '\n')
            continue;
    }

    copy_text(buf, cap, raw);
    return 1;
}

static int parse_int(const char *text) {
    int sign = 1;
    int value = 0;

    if (text == NULL)
        return 0;
    if (*text == '-') {
        sign = -1;
        text++;
    }
    while (*text >= '0' && *text <= '9') {
        value = value * 10 + (*text - '0');
        text++;
    }
    return sign * value;
}

static double parse_rating(const char *text) {
    double value = 0.0;
    double scale = 0.1;
    int sign = 1;

    if (text == NULL)
        return 0.0;
    if (*text == '-') {
        sign = -1;
        text++;
    } else if (*text == '+') {
        text++;
    }
    while (*text >= '0' && *text <= '9') {
        value = value * 10.0 + (double)(*text - '0');
        text++;
    }
    if (*text == '.') {
        text++;
        while (*text >= '0' && *text <= '9') {
            value += (double)(*text - '0') * scale;
            scale *= 0.1;
            text++;
        }
    }
    return sign * value;
}

static void write_rating(FILE *fp, double rating) {
    char buf[64];
    size_t i;

    sprintf(buf, "%.1f", rating);
    for (i = 0; buf[i] != '\0'; i++) {
        if (buf[i] == ',')
            buf[i] = '.';
    }
    fprintf(fp, "%s\n", buf);
}

void list_init(FilmList *list) {
    if (list == NULL)
        return;
    list->head = NULL;
    list->count = 0;
}

void list_append(FilmList *list, const Film *film) {
    FilmNode *node;

    if (list == NULL || film == NULL)
        return;

    node = malloc(sizeof(FilmNode));
    if (node == NULL)
        return;

    memset(node, 0, sizeof *node);
    node->film = *film;
    if (list->head == NULL) {
        node->prev = node;
        node->next = node;
        list->head = node;
        list->count = 1;
        return;
    }

    node->next = list->head;
    node->prev = list->head->prev;
    list->head->prev->next = node;
    list->head->prev = node;
    list->count++;
}

FilmNode *list_find(const FilmList *list, const char *title) {
    FilmNode *node;
    int i;

    if (list == NULL || list->head == NULL || title == NULL)
        return NULL;

    node = list->head;
    for (i = 0; i < list->count; i++) {
        if (strcmp(node->film.title, title) == 0)
            return node;
        node = node->next;
    }
    return NULL;
}

int list_remove(FilmList *list, const char *title) {
    FilmNode *node;
    int i;

    if (list == NULL || list->head == NULL || title == NULL)
        return 0;

    node = list->head;
    for (i = 0; i < list->count; i++) {
        FilmNode *next = node->next;

        if (strcmp(node->film.title, title) == 0) {
            if (list->count == 1) {
                list->head = NULL;
                list->count = 0;
            } else {
                node->prev->next = node->next;
                node->next->prev = node->prev;
                if (list->head == node)
                    list->head = node->next;
                list->count--;
            }
            free(node);
            return 1;
        }
        node = next;
    }
    return 0;
}

void list_free(FilmList *list) {
    FilmNode *node;
    int i;
    int n;

    if (list == NULL)
        return;

    node = list->head;
    n = list->count;
    for (i = 0; i < n && node != NULL; i++) {
        FilmNode *next = node->next;
        free(node);
        node = next;
    }
    list->head = NULL;
    list->count = 0;
}

int films_load(FilmList *list, const char *filename) {
    FILE *fp;
    int added = 0;
    char line[512];

    if (list == NULL || filename == NULL)
        return -1;

    fp = fopen(filename, "r");
    if (fp == NULL)
        return -1;

    while (read_line(fp, line, sizeof line)) {
        Film film;

        if (line[0] == '\0')
            continue;

        memset(&film, 0, sizeof film);
        copy_text(film.title, sizeof film.title, line);
        if (!read_line(fp, line, sizeof line))
            break;
        film.year = parse_int(line);
        if (!read_line(fp, film.country, sizeof film.country))
            break;
        if (!read_line(fp, film.genre, sizeof film.genre))
            break;
        if (!read_line(fp, line, sizeof line))
            break;
        film.rating = parse_rating(line);
        list_append(list, &film);
        added++;
    }

    fclose(fp);
    return added;
}

int films_save(const FilmList *list, const char *filename) {
    FILE *fp;
    const FilmNode *node;
    int i;

    if (list == NULL || filename == NULL)
        return 0;

    fp = fopen(filename, "w");
    if (fp == NULL)
        return 0;

    node = list->head;
    for (i = 0; i < list->count && node != NULL; i++) {
        fprintf(fp, "%s\n%d\n%s\n%s\n",
            node->film.title,
            node->film.year,
            node->film.country,
            node->film.genre);
        write_rating(fp, node->film.rating);
        node = node->next;
    }

    fclose(fp);
    return 1;
}

int valid_film(const Film *film) {
    if (film == NULL)
        return 0;
    if (film->title[0] == '\0' || film->country[0] == '\0' || film->genre[0] == '\0')
        return 0;
    if (film->year < 1895 || film->year > 2100)
        return 0;
    if (!(film->rating >= 0.0 && film->rating <= 10.0))
        return 0;
    return 1;
}

int valid_login(const char *login) {
    int n = 0;

    if (login == NULL)
        return 0;
    while (login[n] != '\0') {
        if (!is_latin_alnum((unsigned char)login[n]) || n >= 20)
            return 0;
        n++;
    }
    return n >= 3 && n <= 20;
}

int valid_password(const char *password) {
    int n = 0;
    int upper = 0;
    int lower = 0;
    int digit = 0;

    if (password == NULL)
        return 0;
    while (password[n] != '\0') {
        unsigned char c = (unsigned char)password[n];

        if (n >= 20)
            return 0;
        if (c >= 'A' && c <= 'Z')
            upper = 1;
        else if (c >= 'a' && c <= 'z')
            lower = 1;
        else if (c >= '0' && c <= '9')
            digit = 1;
        else
            return 0;
        n++;
    }
    return n >= 6 && n <= 20 && upper && lower && digit;
}

int valid_card(const char *card) {
    int n = 0;

    if (card == NULL)
        return 0;
    while (card[n] != '\0') {
        if (card[n] < '0' || card[n] > '9' || n >= 16)
            return 0;
        n++;
    }
    return n == 16;
}

int users_load(const char *filename, User *users, int max) {
    FILE *fp;
    int count = 0;
    char line[512];

    if (filename == NULL || users == NULL || max <= 0)
        return 0;

    fp = fopen(filename, "r");
    if (fp == NULL)
        return 0;

    while (count < max && read_line(fp, line, sizeof line)) {
        char login[128];
        char password[128];
        char card[128];
        int favorites;
        int admin;

        if (line[0] == '\0')
            continue;
        if (sscanf(line, "%127s %127s %127s %d %d",
                login, password, card, &favorites, &admin) != 5)
            continue;

        memset(&users[count], 0, sizeof users[count]);
        copy_text(users[count].login, sizeof users[count].login, login);
        copy_text(users[count].password, sizeof users[count].password, password);
        copy_text(users[count].card, sizeof users[count].card, card);
        users[count].favorites_count = favorites;
        users[count].is_admin = admin;
        count++;
    }

    fclose(fp);
    return count;
}

int users_save(const char *filename, const User *users, int n) {
    FILE *fp;
    int i;

    if (filename == NULL || (users == NULL && n > 0) || n < 0)
        return 0;

    fp = fopen(filename, "w");
    if (fp == NULL)
        return 0;

    for (i = 0; i < n; i++) {
        fprintf(fp, "%s %s %s %d %d\n",
            users[i].login,
            users[i].password,
            users[i].card,
            users[i].favorites_count,
            users[i].is_admin);
    }

    fclose(fp);
    return 1;
}

int user_find(const User *users, int n, const char *login) {
    int i;

    if (users == NULL || login == NULL)
        return -1;
    for (i = 0; i < n; i++) {
        if (strcmp(users[i].login, login) == 0)
            return i;
    }
    return -1;
}

int user_login(const User *users, int n, const char *login, const char *password) {
    int index;

    if (password == NULL)
        return -1;
    index = user_find(users, n, login);
    if (index < 0)
        return -1;
    if (strcmp(users[index].password, password) != 0)
        return -1;
    return index;
}

int user_register(User *users, int *n, int max, const char *login, const char *password, const char *card) {
    User *user;

    if (!valid_login(login))
        return 1;
    if (!valid_password(password))
        return 2;
    if (!valid_card(card))
        return 3;
    if (user_find(users, *n, login) >= 0)
        return 4;
    if (*n >= max)
        return 5;

    user = &users[*n];
    memset(user, 0, sizeof *user);
    copy_text(user->login, sizeof user->login, login);
    copy_text(user->password, sizeof user->password, password);
    copy_text(user->card, sizeof user->card, card);
    user->favorites_count = 0;
    user->is_admin = 0;
    (*n)++;
    return 0;
}

void favorites_filename(const char *login, char *out) {
    const char *prefix = "favorites_";
    const char *suffix = ".txt";
    int i = 0;
    int k;

    if (out == NULL)
        return;
    if (login == NULL)
        login = "";

    for (k = 0; prefix[k] != '\0'; k++)
        out[i++] = prefix[k];
    for (k = 0; login[k] != '\0'; k++)
        out[i++] = login[k];
    for (k = 0; suffix[k] != '\0'; k++)
        out[i++] = suffix[k];
    out[i] = '\0';
}

int favorites_add(FilmList *favorites, const Film *film) {
    int before;

    if (favorites == NULL || film == NULL)
        return 0;
    if (list_find(favorites, film->title) != NULL)
        return 0;
    before = favorites->count;
    list_append(favorites, film);
    return favorites->count > before;
}

int catalog_delete(FilmList *catalog, const char *title, User *users, int n) {
    int i;

    if (catalog == NULL || title == NULL)
        return 0;
    if (!list_remove(catalog, title))
        return 0;

    for (i = 0; i < n; i++) {
        FilmList favorites;
        char path[35];
        int loaded;
        int removed;

        if (users == NULL)
            break;

        favorites_filename(users[i].login, path);
        list_init(&favorites);
        loaded = films_load(&favorites, path);
        if (loaded < 0)
            continue;

        removed = list_remove(&favorites, title);
        if (removed)
            films_save(&favorites, path);
        users[i].favorites_count = favorites.count;
        list_free(&favorites);
    }
    return 1;
}
