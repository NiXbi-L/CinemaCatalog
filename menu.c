#include "menu.h"
#include "list.h"
#include "render.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

bool luhn_check(const char* card_number) {
    int sum = 0;
    int len = strlen(card_number);
    int is_second = 0;

    for (int i = len - 1; i >= 0; i--) {
        int digit = card_number[i] - '0';

        if (is_second) {
            digit *= 2;
            if (digit > 9) {
                digit -= 9;
            }
        }

        sum += digit;
        is_second = !is_second;
    }

    return (sum % 10) == 0;
}

int count_spaces(char* line) {
    int spaces = 0;
    for (size_t i = 0; i < strlen(line); i++)
        if (line[i] == ' ')
            spaces++;
    return spaces;
}

int* get_space_indexes(char* line) {
    int spaces = count_spaces(line);
    int* space_indexes;
    size_t j = 0;

    if (spaces == 0)
        return NULL;

    space_indexes = (int*)malloc(sizeof(int) * (size_t)spaces);
    if (space_indexes == NULL)
        return NULL;

    for (size_t i = 0; i < strlen(line); i++)
        if (line[i] == ' ')
            space_indexes[j++] = (int)i;

    return space_indexes;
}

char** parse_db_line(char* line) {
    int spaces = count_spaces(line);
    int fields = spaces + 1;
    char** parse_res = (char**)malloc(sizeof(char*) * (size_t)fields);
    int* space_indexes = get_space_indexes(line);
    int start = 0;

    if (parse_res == NULL) {
        free(space_indexes);
        return NULL;
    }
    if (spaces > 0 && space_indexes == NULL) {
        free(parse_res);
        return NULL;
    }

    for (int i = 0; i < fields; i++) {
        int end = (i < spaces) ? space_indexes[i] : (int)strlen(line);
        int len;

        while (end > start && (line[end - 1] == '\n' || line[end - 1] == '\r'))
            end--;

        len = end - start;
        if (len < 0)
            len = 0;

        parse_res[i] = (char*)malloc((size_t)len + 1);
        if (parse_res[i] == NULL) {
            for (int k = 0; k < i; k++)
                free(parse_res[k]);
            free(parse_res);
            free(space_indexes);
            return NULL;
        }

        if (len > 0)
            strncpy(parse_res[i], line + start, (size_t)len);
        parse_res[i][len] = '\0';
        start = end + 1;
    }

    free(space_indexes);
    return parse_res;
}

static FILE* open_data(const char* name);

static void free_fields(char** fields, int count) {
    if (fields == NULL)
        return;
    for (int i = 0; i < count; i++)
        free(fields[i]);
    free(fields);
}

user* load_users() {
    const int line_size = 350;
    FILE* file = open_data("Users.txt");
    char* line = (char*)malloc((size_t)line_size);
    user* first_user = NULL;
    user* last_user = NULL;

    if (file == NULL || line == NULL) {
        free(line);
        if (file != NULL)
            fclose(file);
        return NULL;
    }

    if (fgets(line, line_size, file) == NULL) {
        free(line);
        fclose(file);
        return NULL;
    }

    while (fgets(line, line_size, file) != NULL) {
        int spaces = count_spaces(line);
        int fields = spaces + 1;
        char** db_arr = parse_db_line(line);
        user* node;

        if (db_arr == NULL || fields < 5) {
            free_fields(db_arr, db_arr == NULL ? 0 : fields);
            continue;
        }

        node = (user*)malloc(sizeof(user));
        if (node == NULL) {
            free_fields(db_arr, fields);
            break;
        }

        node->id = (unsigned int)strtoul(db_arr[0], NULL, 10);
        node->name = (char*)malloc(strlen(db_arr[1]) + 1);
        node->password = (char*)malloc(strlen(db_arr[2]) + 1);
        if (node->name != NULL)
            strcpy(node->name, db_arr[1]);
        if (node->password != NULL)
            strcpy(node->password, db_arr[2]);
        node->is_admin = db_arr[3][0] == '1';
        strncpy(node->card_num, db_arr[4], sizeof(node->card_num) - 1);
        node->card_num[sizeof(node->card_num) - 1] = '\0';
        node->next = NULL;
        node->previous = last_user;

        if (last_user == NULL)
            first_user = node;
        else
            last_user->next = node;
        last_user = node;

        free_fields(db_arr, fields);
    }

    free(line);
    fclose(file);
    return first_user;
}


static FILE* open_data(const char* name) {
    FILE* file = fopen(name, "r");
    char alt[512];

    if (file != NULL)
        return file;

    snprintf(alt, sizeof(alt), "../%s", name);
    return fopen(alt, "r");
}

static char* dup_str(const char* src) {
    size_t len = strlen(src);
    char* copy = (char*)malloc(len + 1);
    if (copy != NULL)
        memcpy(copy, src, len + 1);
    return copy;
}

static char* join_fields(char** fields, int from, int to) {
    size_t len = 0;
    char* out;
    int i;

    for (i = from; i < to; i++)
        len += strlen(fields[i]) + (i > from ? 1 : 0);

    out = (char*)malloc(len + 1);
    if (out == NULL)
        return NULL;

    out[0] = '\0';
    for (i = from; i < to; i++) {
        if (i > from)
            strcat(out, " ");
        strcat(out, fields[i]);
    }
    return out;
}

static int starts_with_digit(const char* text) {
    return text != NULL && text[0] >= '0' && text[0] <= '9';
}

static float parse_float(const char* text) {
    float value = 0.0f;
    float scale = 0.1f;
    int i = 0;

    if (text == NULL)
        return 0.0f;
    while (text[i] == ' ')
        i++;
    for (; text[i] >= '0' && text[i] <= '9'; i++)
        value = value * 10.0f + (float)(text[i] - '0');
    if (text[i] == '.' || text[i] == ',') {
        for (i++; text[i] >= '0' && text[i] <= '9'; i++, scale *= 0.1f)
            value += (float)(text[i] - '0') * scale;
    }
    return value;
}

static film* load_films(void) {
    const int line_size = 350;
    FILE* file = open_data("Films.txt");
    char* line = (char*)malloc((size_t)line_size);
    film* head = NULL;

    if (file == NULL || line == NULL) {
        free(line);
        if (file != NULL)
            fclose(file);
        return NULL;
    }

    while (fgets(line, line_size, file) != NULL) {
        int spaces = count_spaces(line);
        int fields = spaces + 1;
        int name_count;
        char** db_arr = parse_db_line(line);
        film* node;

        if (db_arr == NULL || fields < 5 || !starts_with_digit(db_arr[fields - 4])) {
            free_fields(db_arr, db_arr == NULL ? 0 : fields);
            continue;
        }

        name_count = fields - 4;
        node = (film*)malloc(sizeof(film));
        if (node == NULL) {
            free_fields(db_arr, fields);
            break;
        }

        node->name = join_fields(db_arr, 0, name_count);
        node->year_create = atoi(db_arr[fields - 4]);
        node->country = dup_str(db_arr[fields - 3]);
        node->genre = dup_str(db_arr[fields - 2]);
        node->rating = parse_float(db_arr[fields - 1]);
        node->next = NULL;
        node->previous = NULL;
        head = film_add_end(head, node);
        free_fields(db_arr, fields);
    }

    free(line);
    fclose(file);

    if (head != NULL) {
        film* tail = head;
        while (tail->next != NULL)
            tail = tail->next;
        tail->next = head;
        head->previous = tail;
    }
    return head;
}

static int read_line(char* buf, int size) {
    size_t len;

    if (fgets(buf, size, stdin) == NULL)
        return 0;
    len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
        buf[--len] = '\0';
    return 1;
}

static user* find_user(user* users, const char* name, const char* password) {
    for (; users != NULL; users = users->next) {
        if (users->name != NULL && users->password != NULL &&
            strcmp(users->name, name) == 0 &&
            strcmp(users->password, password) == 0)
            return users;
    }
    return NULL;
}

typedef struct favorite {
    film* item;
    struct favorite* next;
} favorite;

static int favorite_count(favorite* head) {
    int count = 0;
    for (; head != NULL; head = head->next)
        count++;
    return count;
}

static bool is_favorite(favorite* head, film* item) {
    for (; head != NULL; head = head->next)
        if (head->item == item)
            return true;
    return false;
}

static favorite* add_favorite(favorite* head, film* item) {
    favorite* node = (favorite*)malloc(sizeof(favorite));
    if (node == NULL)
        return head;
    node->item = item;
    node->next = head;
    return node;
}

static void catalog(user* current, film* films) {
    film* active = films;
    favorite* favorites = NULL;
    char message[256];

    message[0] = '\0';
    while (active != NULL) {
        char line[64];
        int command;

        render_film(active);
        printf("Вход: %s%s | В избранном: %d\n",
            current->name,
            current->is_admin ? " (админ)" : "",
            favorite_count(favorites));
        if (message[0] != '\0')
            printf("%s\n", message);
        printf("Команда: ");
        fflush(stdout);

        if (!read_line(line, (int)sizeof(line)))
            break;
        if (line[0] == '\0')
            continue;

        command = atoi(line);
        if (command == 1) {
            active = active->next;
            message[0] = '\0';
        }
        else if (command == 2) {
            active = active->previous;
            message[0] = '\0';
        }
        else if (command == 3) {
            snprintf(message, sizeof(message),
                "Выбран фильм «%s» (%d, %s, %s), рейтинг %.1f. Билет оплачен картой.",
                active->name, active->year_create, active->country, active->genre, active->rating);
        }
        else if (command == 4) {
            if (is_favorite(favorites, active)) {
                snprintf(message, sizeof(message), "«%s» уже в избранном.", active->name);
            }
            else {
                favorites = add_favorite(favorites, active);
                snprintf(message, sizeof(message), "«%s» добавлен в избранное.", active->name);
            }
        }
        else if (command == 5) {
            break;
        }
        else {
            snprintf(message, sizeof(message), "Введите число от 1 до 5.");
        }
    }

    while (favorites != NULL) {
        favorite* next = favorites->next;
        free(favorites);
        favorites = next;
    }
}

void start_menu() {
    user* users = load_users();
    film* films = load_films();
    char name[128];
    char password[128];

    printf("Кинокаталог\n");
    if (users == NULL) {
        printf("Не удалось открыть Users.txt. Запустите программу из папки проекта.\n");
        return;
    }
    if (films == NULL) {
        printf("Не удалось открыть Films.txt. Запустите программу из папки проекта.\n");
        return;
    }

    for (;;) {
        user* current;

        printf("\nИмя (пустая строка — выход): ");
        fflush(stdout);
        if (!read_line(name, (int)sizeof(name)) || name[0] == '\0')
            break;

        printf("Пароль: ");
        fflush(stdout);
        if (!read_line(password, (int)sizeof(password)))
            break;

        current = find_user(users, name, password);
        if (current == NULL) {
            printf("Неверное имя или пароль.\n");
            continue;
        }
        if (!luhn_check(current->card_num)) {
            printf("Номер карты не прошёл проверку.\n");
            continue;
        }

        catalog(current, films);
        break;
    }
}