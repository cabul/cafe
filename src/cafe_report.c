#include <stdio.h>

void cafe_report_begin() { printf("\n"); }
void cafe_report_end(int passing, int pending, int failing) {}
void cafe_report_suite(char *suite);
void cafe_report_test(char *test, int status, char *failure);
