#include <stdio.h>
#include <stdarg.h>
#include <time.h>

static void print_timestamp() {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    printf("[%02d:%02d:%02d] ",
           t->tm_hour,
           t->tm_min,
           t->tm_sec);
}

void log_info(const char* fmt, ...) {
    print_timestamp();
    printf("[INFO] ");

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n");
}

void log_error(const char* fmt, ...) {
    print_timestamp();
    printf("[ERROR] ");

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n");
}