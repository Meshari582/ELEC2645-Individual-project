// Function prototypes for the EEE Helper CLI calculator.
// Each function implements a menu-driven calculation module.

#ifndef FUNCS_H
#define FUNCS_H

void menu_item_1(void); // Voltage Divider
void menu_item_2(void); // Resistors (Series / Parallel-2)
void menu_item_3(void); // AC Reactance & Resonance
void menu_item_4(void); // RC Transient
void menu_item_5(void); // Power (P = V * I)

// Data logging 
int  log_line(const char *line);
void view_log(void);

#endif
