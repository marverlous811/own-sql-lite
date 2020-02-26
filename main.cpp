#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "string.h"
#include "errno.h"
#include "fcntl.h"
#include "unistd.h"

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

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)
#define TABLE_MAX_PAGES 100

const uint32_t PAGE_SIZE = 4096;

typedef struct {
    int fd;
    uint32_t file_len;
    void* pages[TABLE_MAX_PAGES];
} Pager;

Pager* pager_open(const char* filename){
    int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR );
    if(fd == -1) {
        panic("Unable to open file\n");
    }

    off_t file_len = lseek(fd, 0, SEEK_END);

    Pager* pager = (Pager*)malloc(sizeof(Pager));
    pager->fd = fd;
    pager->file_len = file_len;

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++){
        pager->pages[i] = NULL;
    }

    return pager;
}

void pager_flush(Pager* pager, uint32_t page_num, uint32_t size) {
    if(pager->pages[page_num] == NULL) {
        panic("Tried to flush null page\n");
    }

    off_t offset = lseek(pager->fd, page_num * PAGE_SIZE, SEEK_SET);
    if(offset == -1){
        panic("Error seeking: %d\n", errno);
    }

    ssize_t bytes_written = write(pager->fd, pager->pages[page_num], size);
    if(bytes_written == -1) panic("Error writing: %d\n", errno);
}

void* get_page(Pager* pager, uint32_t page_num){
    if(page_num > TABLE_MAX_PAGES) {
        panic("Tried to fetch page number out of bounds %d > %d\n", page_num, TABLE_MAX_PAGES);
    }

    if(pager->pages[page_num] == NULL){
        //Cache miss. Allocate memory and load from file
        void* page = malloc(PAGE_SIZE);
        uint32_t num_pages = pager->file_len / PAGE_SIZE;

        if(pager->file_len % PAGE_SIZE){
            num_pages += 1;
        }

        if(page_num <= num_pages){
            lseek(pager->fd, page_num * PAGE_SIZE, SEEK_SET);
            ssize_t bytes_read = read(pager->fd, page, PAGE_SIZE);
            if (bytes_read == -1) {
                panic("Error reading file: %d\n", errno);
            }
        }

        pager->pages[page_num] = page;
    }

    return pager->pages[page_num];
}

//**************Fixed table*************************//

#define COLUMN_USER_SIZE 32
#define COLUMN_EMAIL_SIZE 255

typedef struct {
    uint32_t id;
    char username[COLUMN_USER_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
} UserRow;

const uint32_t ID_SIZE = size_of_attribute(UserRow, id);
const uint32_t USERNAME_SIZE = size_of_attribute(UserRow, username);
const uint32_t EMAIL_SIZE = size_of_attribute(UserRow, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t USER_ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

void serialize_row(UserRow* source, void* destination){
    memcpy((char*)destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy((char*)destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    memcpy((char*)destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void* source, UserRow* destination){
    memcpy(&(destination->id), (char*)source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), (char*)source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), (char*)source + EMAIL_OFFSET, EMAIL_SIZE);
}

const uint32_t ROWS_PER_PAGE = PAGE_SIZE / USER_ROW_SIZE;
const uint32_t TABLE_MAX_ROW = ROWS_PER_PAGE * TABLE_MAX_PAGES;

typedef struct {
    uint32_t num_rows;
    Pager* pager;
} Table;

void* row_slot(Table* table, uint32_t row_num){
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void* page = get_page(table->pager, page_num);
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * USER_ROW_SIZE;
    return (char*)page + byte_offset;
}

void print_row(UserRow* row){
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

Table* db_open(const char* filename){
    Pager* pager = pager_open(filename);
    uint32_t num_rows = pager->file_len / USER_ROW_SIZE;

    Table* table = (Table*)malloc(sizeof(Table));
    table->pager = pager;
    table->num_rows = num_rows;

    return table;
}

void db_close(Table* table) {
    Pager* pager = table->pager;
    uint32_t num_full_pagers = table->num_rows / ROWS_PER_PAGE;

    for(uint32_t i = 0; i < num_full_pagers; i++){
        if(pager->pages[i] == NULL) {
            continue;
        }

        pager_flush(pager, i, PAGE_SIZE);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
    }

    uint32_t num_additional_rows = table->num_rows % ROWS_PER_PAGE;
    if(num_additional_rows > 0 ) {
        uint32_t page_num = num_full_pagers;
        if(pager->pages[page_num] != NULL){
            pager_flush(pager, page_num, num_additional_rows * USER_ROW_SIZE);
            free(pager->pages[page_num]);
            pager->pages[page_num] = NULL;
        }
    }

    int result = close(pager->fd);
    if(result == -1) panic("Error closing db file.\n");

    for(uint32_t i = 0; i < TABLE_MAX_PAGES; i++){
        void* page = pager->pages[i];
        if(page){
            free(page);
            pager->pages[i] = NULL;
        }
    }

    free(pager);
    free(table);
}


//==================================================//

typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_SYNTAX_ERROR,
    PREPARE_NEGATIVE_ID,
    PREPARE_STRING_TOO_LONG,
    PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

typedef enum {
    INSERT,
    SELECT
} StatementType;

typedef enum {
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL,
} ExecResult;

typedef struct {
    StatementType type;
    UserRow row_to_insert;
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
    printf("db > ");
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

MetaCommandResult exec_meta_command(InputBuffer *ibuffer, Table* table){
    if(strcmp(ibuffer->buffer, ".exit") == 0) {
        db_close(table);
        exit(EXIT_SUCCESS);
    } else  {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

PrepareResult prepare_insert(InputBuffer *ibuffer, Statement* statement){
    statement->type = INSERT;

    char* keyword = strtok(ibuffer->buffer, " ");
    char* id_string = strtok(NULL, " ");
    char* username = strtok(NULL, " ");
    char* email = strtok(NULL, " ");

    if(id_string == NULL || username == NULL || email == NULL) {
        return PREPARE_SYNTAX_ERROR;
    }

    int id = atoi(id_string);
    if(id < 0) {
        return PREPARE_NEGATIVE_ID;
    }
    if(strlen(username) > COLUMN_USER_SIZE) {
        return PREPARE_STRING_TOO_LONG;
    }
    if(strlen(email) > COLUMN_EMAIL_SIZE) {
        return PREPARE_STRING_TOO_LONG;
    }

    statement->row_to_insert.id = id;
    strcpy(statement->row_to_insert.username, username);
    strcpy(statement->row_to_insert.email, email);

    return PREPARE_SUCCESS;
}

PrepareResult preprare_statement(InputBuffer *ibuffer, Statement* statement) {
    if (strncmp(ibuffer->buffer, "insert", 6) == 0){
        return prepare_insert(ibuffer, statement);
    }
    if (strncmp(ibuffer->buffer, "select", 6) == 0) {
        statement->type = SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

ExecResult exec_insert(Statement* statement, Table* table) {
    if(table ->num_rows >= TABLE_MAX_ROW) {
        return EXECUTE_TABLE_FULL;
    }

    UserRow* row_to_insert = &(statement->row_to_insert);

    serialize_row(row_to_insert, row_slot(table, table->num_rows));
    table->num_rows += 1;

    return EXECUTE_SUCCESS;
}

ExecResult exec_select(Statement* statement, Table* table){
    UserRow row;
    for(uint32_t i = 0; i < table->num_rows; i++){
        deserialize_row(row_slot(table, i), &row);
        print_row(&row);
    }

    return EXECUTE_SUCCESS;
}

ExecResult exec_statement(Statement* statement,  Table* table){
    switch (statement->type)
    {
    case INSERT:
        return exec_insert(statement, table);
    case SELECT:
        return exec_select(statement, table);
    }
}

int main(int argc, char* argv[]){
    if(argc < 2) panic("Must supply a db filename.\n");

    char* filename = argv[1];

    InputBuffer* ibuffer = new_input_buffer();
    Table* table = db_open(filename);
    while(true){
        print_promt();
        read_input(ibuffer);

        if(ibuffer->buffer[0] == '.'){
            switch (exec_meta_command(ibuffer, table))
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
            case PREPARE_SYNTAX_ERROR:
                printf("Syntax error. could not parse statement.\n");
                continue;
            case PREPARE_STRING_TOO_LONG:
                printf("String is to long.\n");
                continue;
            case PREPARE_NEGATIVE_ID:
                printf("ID must be positive.\n");
                continue;
            default:
                printf("Unrecognized keyword at start of '%s'.\n",ibuffer->buffer);
                continue;
        }

        switch(exec_statement(&statement, table)){
            case EXECUTE_SUCCESS:
                printf("Executed.\n");
                break;
            case EXECUTE_TABLE_FULL:
                printf("Error: table full.\n");
                break;
        }
    }
}