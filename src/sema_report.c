#include "sema_report.h"
#include <stdio.h>

void sema_report_error(const char *msg) {
    fprintf(stderr, "\033[31mErro semântico: %s\033[0m\n", msg);
}

void sema_report_alert(const char *msg, int line) {
    fprintf(stderr, "\033[33mAlerta semântico (linha %d): %s\033[0m\n", line, msg);
}

