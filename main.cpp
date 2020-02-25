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

typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

typedef enum {
    INSERT,
    SELECT
} StatementType;

typedef struct {
    StatementType type;
} Statement;

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

MetaCommandResult exec_meta_command(InputBuffer *ibuffer){
    if(strcmp(ibuffer->buffer, ".exit") == 0) {
        exit(EXIT_SUCCESS);
    } else  {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

PrepareResult preprare_statement(InputBuffer *ibuffer, Statement* statement) {
    if (strncmp(ibuffer->buffer, "insert", 6) == 0){
        statement->type = INSERT;
        return PREPARE_SUCCESS;
    }
    if (strncmp(ibuffer->buffer, "select", 6) == 0) {
        statement->type = SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void exec_statement(Statement* statement){
    switch (statement->type)
    {
    case INSERT:
        printf("This is where we would do an insert. \n");
        break;
    case SELECT:
        printf("This is where we would do an select. \n");
        break;
    default:
        break;
    }
}

int main(){
    InputBuffer* ibuffer = new_input_buffer();
    while(true){
        print_promt();
        read_input(ibuffer);

        if(ibuffer->buffer[0] == '.'){
            switch (exec_meta_command(ibuffer))
            {
            case META_COMMAND_SUCCESS:
                break;
            default:
                printf("Unrecognized command '%s'\n", ibuffer->buffer);
                break;
            }
            continue;
        }

        Statement statement;
        switch(preprare_statement(ibuffer, &statement)) {
            case PREPARE_SUCCESS:
                break;
            default:
                printf("Unrecognized keyword at start of '%s'.\n",ibuffer->buffer);
                continue;
        }

        exec_statement(&statement);
        printf("Executed. \n");
    }
}