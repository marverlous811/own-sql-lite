#include "../header/input.hpp"

InputBuffer::InputBuffer(){
    this->buffer = NULL;
    this->buffer_length = 0;
    this->input_length = 0;
}

InputBuffer::~InputBuffer(){
    free(this->buffer);
}

void InputBuffer::read(){
    ssize_t bytes_read = getline(&this->buffer, &this->buffer_length, stdin);
    if(bytes_read <= 0) {
        panic("Error reading input\n");
    }

    this->input_length = bytes_read - 1;
    this->buffer[bytes_read - 1] = 0;
}