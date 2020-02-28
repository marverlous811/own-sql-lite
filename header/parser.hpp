#ifndef __PARSER__
#define __PARSER__

#include "input.hpp"
#include "action.hpp"
#include "map"
#include "string"

enum PREPARE_RESULT {
    PREPARE_SUCCESS,
    PREPARE_SYNTAX_ERROR,
    PREPARE_UNRECONIZED,
};

class Parser{
protected:
    InputBuffer* ibuffer;
public: 
    Parser(InputBuffer* ibuffer) : ibuffer{ibuffer}{}
    virtual PREPARE_RESULT prepare() = 0;
    virtual Action* parse() = 0;
    Parser& operator= (const Parser& x) {return *this;}
};

class CommandParser : public Parser{
    std::map<std::string, CMD> listCommand;
    CMD type;
public:
    CommandParser(InputBuffer* ibuffer) 
        : Parser(ibuffer) {
            this->listCommand.emplace(".exit", CMD::EXIT);
        }
    PREPARE_RESULT prepare() override;
    Action* parse() override;
};

struct ParserFactory{
    static Parser* create_command_parser(InputBuffer* ibuffer){
        CommandParser* p = new CommandParser(ibuffer);
        return p;
    }
};

#endif