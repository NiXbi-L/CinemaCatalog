#pragma once
#include <stdbool.h>
typedef struct film
{
	char* name;
	int year_create;
	char* country;
	char* genre;
	float rating;
	struct film* next;
	struct film* previous;
} film;

typedef struct user
{
    unsigned int id;
    char* name;
	bool is_admin;
	char card_num[16];
	struct user* next;
	struct user* previous;
} user;

void start_menu();