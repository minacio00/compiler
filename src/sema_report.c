#include "sema_report.h"
#include <stdio.h>

void sema_report_error(const char *msg) {
    fprintf(stderr, "\033[31mErro semântico: %s\033[0m\n", msg);
}

