#include "stdio.h"
#include "../header/input.hpp"
#include "../header/parser.hpp"

void complier(InputBuffer* ibuffer){
    ParserFactory pfactory;
    Parser* parser = nullptr;

    if(ibuffer->buffer[0] == '.'){
        parser = pfactory.create_command_parser(ibuffer);
    }

    if(parser == nullptr) {
        printf("Unrecognized %s\n", ibuffer->buffer);
        return;
    }

    switch(parser->prepare()){
        case PREPARE_RESULT::PREPARE_SUCCESS:{
            Action* action = parser->parse();
            if( action == nullptr) {
                printf("can build actions: %s\n", ibuffer->buffer);
                return;
            }
            action->exec();
            break;
        }
        case PREPARE_RESULT::PREPARE_UNRECONIZED:
            printf("Unrecognized %s\n", ibuffer->buffer);
            break;
    }
}

int main(int argc, char* argv[]){
    InputBuffer* ibuffer = new InputBuffer();
    while (true)
    {
        print_promt();
        ibuffer->read();

        complier(ibuffer);
    }
    
    return 0;
}