#include "menu.h"
#include "render.h"

#include <stdio.h>
#include <string.h>

#define MAX_USERS 128

static void eat_line(void) {
    int ch;

    while ((ch = getchar()) != '\n' && ch != EOF)
        ;
}

static int read_line(char *buf) {
    int ok = scanf("%127[^\n]", buf);

    eat_line();
    if (ok != 1)
        buf[0] = '\0';
    return ok == 1;
}

static void register_user(User *users, int *nusers) {
    char login[128];
    char password[128];
    char card[128];
    int code;

    printf("Логин (3–20 латинских букв и цифр): ");
    read_line(login);
    printf("Пароль (6–20, заглавная, строчная и цифра): ");
    read_line(password);
    printf("Номер карты (16 цифр): ");
    read_line(card);

    code = user_register(users, nusers, MAX_USERS, login, password, card);
    if (code == 0) {
        users_save("Users.txt", users, *nusers);
        printf("Пользователь %s зарегистрирован.\n", login);
    } else if (code == 1)
        printf("Неверный логин.\n");
    else if (code == 2)
        printf("Неверный пароль.\n");
    else if (code == 3)
        printf("Неверный номер карты.\n");
    else if (code == 4)
        printf("Логин занят.\n");
    else
        printf("Достигнут предел числа пользователей.\n");
}

static int prompt_film(Film *film) {
    memset(film, 0, sizeof *film);

    printf("Название: ");
    if (!read_line(film->title))
        return 0;
    printf("Год: ");
    if (scanf("%d", &film->year) != 1)
        film->year = 0;
    eat_line();
    printf("Страны: ");
    if (!read_line(film->country))
        return 0;
    printf("Жанры: ");
    if (!read_line(film->genre))
        return 0;
    printf("Рейтинг: ");
    if (scanf("%lf", &film->rating) != 1)
        film->rating = -1;
    eat_line();
    return 1;
}

static void show_catalog(User *users, int nusers, int index, FilmList *catalog) {
    User *current = &users[index];
    FilmList favorites;
    FilmNode *active = catalog->head;
    char fav_path[35];
    char message[256];

    list_init(&favorites);
    favorites_filename(current->login, fav_path);
    if (films_load(&favorites, fav_path) >= 0)
        current->favorites_count = favorites.count;

    message[0] = '\0';
    while (active != NULL) {
        int command;

        render_film(active);
        printf("Вход: %s%s | В избранном: %d\n",
            current->login,
            current->is_admin ? " (админ)" : "",
            favorites.count);
        if (current->is_admin)
            printf("6 - удалить фильм из каталога    7 - добавить фильм\n");
        if (message[0] != '\0')
            printf("%s\n", message);
        printf("Команда: ");

        if (scanf("%d", &command) != 1) {
            if (feof(stdin))
                break;
            eat_line();
            sprintf(message, "Введите число от 1 до %s.", current->is_admin ? "7" : "5");
            continue;
        }
        eat_line();

        if (command == 1) {
            active = active->next;
            message[0] = '\0';
        } else if (command == 2) {
            active = active->prev;
            message[0] = '\0';
        } else if (command == 3) {
            sprintf(message, "Выбран фильм «%s» (%d, %s, %s), рейтинг %.1f. Билет оплачен картой.",
                active->film.title, active->film.year, active->film.country,
                active->film.genre, active->film.rating);
        } else if (command == 4) {
            if (favorites_add(&favorites, &active->film)) {
                current->favorites_count = favorites.count;
                films_save(&favorites, fav_path);
                users_save("Users.txt", users, nusers);
                sprintf(message, "«%s» добавлен в избранное.", active->film.title);
            } else {
                sprintf(message, "«%s» уже в избранном.", active->film.title);
            }
        } else if (command == 5) {
            break;
        } else if (command == 6 && current->is_admin) {
            int alone = catalog->count == 1;
            FilmNode *next = alone ? NULL : active->next;
            char title[FILM_TEXT];

            strcpy(title, active->film.title);
            if (catalog_delete(catalog, title, users, nusers)) {
                films_save(catalog, "Films.txt");
                list_remove(&favorites, title);
                current->favorites_count = favorites.count;
                users_save("Users.txt", users, nusers);
                sprintf(message, "«%s» удалён из каталога.", title);
                if (alone) {
                    active = NULL;
                    printf("%s\n", message);
                    break;
                }
                active = next;
            } else {
                sprintf(message, "Фильм не найден.");
            }
        } else if (command == 7 && current->is_admin) {
            Film film;

            if (!prompt_film(&film))
                break;
            if (!valid_film(&film)) {
                sprintf(message, "Фильм не принят: название, страны и жанры не пустые, год 1895–2100, рейтинг 0–10.");
            } else {
                list_append(catalog, &film);
                films_save(catalog, "Films.txt");
                sprintf(message, "«%s» добавлен в каталог.", film.title);
            }
        } else {
            sprintf(message, "Введите число от 1 до %s.", current->is_admin ? "7" : "5");
        }
    }

    if (active == NULL && catalog->count == 0)
        printf("Каталог пуст.\n");

    list_free(&favorites);
}

void start_menu(void) {
    User users[MAX_USERS];
    FilmList catalog;
    char name[128];
    char password[128];
    int nusers;

    list_init(&catalog);
    printf("Кинокаталог\n");

    nusers = users_load("Users.txt", users, MAX_USERS);
    if (nusers <= 0) {
        printf("Не удалось открыть Users.txt. Запустите программу из папки проекта.\n");
        return;
    }
    if (films_load(&catalog, "Films.txt") <= 0) {
        printf("Не удалось открыть Films.txt. Запустите программу из папки проекта.\n");
        list_free(&catalog);
        return;
    }

    for (;;) {
        int index;

        printf("\nИмя (пустая строка — выход, new — регистрация): ");
        if (!read_line(name))
            break;
        if (strcmp(name, "new") == 0) {
            register_user(users, &nusers);
            continue;
        }

        printf("Пароль: ");
        if (!read_line(password))
            break;

        index = user_login(users, nusers, name, password);
        if (index < 0) {
            printf("Неверное имя или пароль.\n");
            continue;
        }

        show_catalog(users, nusers, index, &catalog);
        break;
    }

    list_free(&catalog);
}
