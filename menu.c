#include "menu.h"
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
    for (int i = 0; i < strlen(line); i++)
        if (line[i] == ' ')
            spaces++;
}

int* get_space_indexes(char* line) {
    int* space_indexes = (int*)maloc(sizeof(int) * count_spaces(line));

    for (int i = 0, j = 0; i < strlen(line); i++)
        if (line[i] == ' ') {
            space_indexes[j] = i;
            j++;
        }
}

char** parse_db_line(char* line) {
    int spaces = count_spaces(line);
    char** parse_res = (char**)malloc(sizeof(char*) * (spaces + 1));
    int* space_indexes = get_space_indexes(line);

    for (int i = 0, j = 0, x = 0; i < spaces + 1; i++, x++) {
        strncpy(parse_res[i], line[j], space_indexes[x]);
    }
    
}


user* load_users() {
    FILE* file = fopen("Users.txt", "r");
    char* line = (char*)malloc(sizeof(char) * 350);
    user* first_user = (user*)malloc(sizeof(user));

    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);

    

    for (int i = 0; fgets(line, sizeof(line), file); i++) {
        char** db_arr = parse_db_line(line);
    }

}


void start_menu() {

}