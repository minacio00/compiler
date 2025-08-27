#ifndef SEMA_REPORT_H
#define SEMA_REPORT_H

void sema_report_error(const char *msg);
void sema_report_alert(const char *msg, int line);

#endif /* SEMA_REPORT_H */
