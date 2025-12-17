// ELEC2645 Unit 2 Project - main.c
// Menu-driven CLI calculator.
// Main menu selection uses fgets + strtol to reject invalid input (e.g. "2abc").

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "funcs.h"

static void print_menu(void);
static int  get_choice(void);
static void wait_back(void);

int main(void)
{
    for (;;) {
        print_menu();

        switch (get_choice()) {
            case 1: menu_item_1(); break; // Voltage Divider
            case 2: menu_item_2(); break; // Resistor Tools
            case 3: menu_item_3(); break; // AC Reactance & Resonance
            case 4: menu_item_4(); break; // RC Transient
            case 5: menu_item_5(); break; // Power (P = V * I)
            case 6: view_log(); break;    // View saved log
            case 7:
                printf("Bye!\n");
                return 0;
            default:
                printf("Invalid choice.\n");
                continue;
        }

        wait_back();
    }
}

static void print_menu(void)
{
    printf("\n====== EEE Helper CLI ======\n");
    printf("1) Voltage divider (Vout)\n");
    printf("2) Resistor tools (series / parallel-2)\n");
    printf("3) AC reactance & resonance\n");
    printf("4) RC transient (tau / %%charge / %%discharge)\n");
    printf("5) Power (P = V * I)\n");
    printf("6) View saved log\n");
    printf("7) Quit\n");
    printf("Select: ");
}

static int get_choice(void)
{
    char buf[64];
    long v;
    char *end = NULL;

    if (!fgets(buf, sizeof buf, stdin)) return -1;

    errno = 0;
    v = strtol(buf, &end, 10);
    if (errno != 0) return -1;
    if (end == buf) return -1;

    while (*end == ' ' || *end == '\t') end++;
    if (*end != '\n' && *end != '\0') return -1;

    return (int)v;
}

static void wait_back(void)
{
    char buf[64];

    for (;;) {
        printf("\nEnter 'b' to go back to the main menu: ");

        if (!fgets(buf, sizeof buf, stdin)) {
            printf("Input error. Exiting.\n");
            exit(1);
        }

        buf[strcspn(buf, "\r\n")] = '\0';

        if ((buf[0] == 'b' || buf[0] == 'B') && buf[1] == '\0')
            return;
    }
}
