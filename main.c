#include <locale.h>
#include <stdio.h>
#include "menu.h"

int main(void) {
    setlocale(LC_ALL, "");
    start_menu();
    return 0;
}
