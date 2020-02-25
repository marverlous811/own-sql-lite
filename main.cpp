#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "string.h"


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

typedef struct 
{
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
}InputBuffer;

InputBuffer* new_input_buffer(){
    InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;

    return input_buffer;
}

void print_promt(){
    printf("DB > ");
}

void read_input(InputBuffer* ibuffer){
    ssize_t bytes_read = getline(&ibuffer->buffer, &ibuffer->buffer_length, stdin);
    if(bytes_read <= 0) {
        panic("Error reading input\n");
    }

    ibuffer->input_length = bytes_read - 1;
    ibuffer->buffer[bytes_read - 1] =  0;
}

void close_input_buffer(InputBuffer* ibuffer){
    free(ibuffer->buffer);
    free(ibuffer);
}

int main(){
    InputBuffer* ibuffer = new_input_buffer();
    while(true){
        print_promt();
        read_input(ibuffer);

        if(strcmp(ibuffer->buffer, "exit") == 0){
            // printf("get exit cmd");
            close_input_buffer(ibuffer);
            exit(EXIT_SUCCESS);
        } else {
            printf("Unrecognized command '%s'.\n", ibuffer->buffer);
        }
    }
}