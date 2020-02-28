#ifndef __INPUT__
#define __INPUT__

#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "stdarg.h"

#define print_promt() printf("db > ") 

__attribute__ ((cold))
__attribute__ ((noreturn))
__attribute__ ((format (printf, 1, 2)))
static void panic(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    abort();
}

class InputBuffer{
public:
    InputBuffer();
    ~InputBuffer();

    void read();

    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
};

#endif