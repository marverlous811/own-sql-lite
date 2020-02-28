#ifndef __ACTION__
#define __ACTION__

#include "input.hpp"

enum EXEC_RESULT { 
    EXECUTED_SUCCESS
};

enum CMD { EXIT , INVALID};

class Action{
protected: 
    InputBuffer* ibuffer;
public:
    Action(InputBuffer* ibuffer) 
        : ibuffer{ibuffer} {}
    virtual EXEC_RESULT exec() = 0;
    Action& operator= (const Action& x) {return *this;}
};

class CommandAction : public Action {
    CMD type;
public:
    CommandAction(InputBuffer* ibuffer, CMD type)
        : Action(ibuffer), type{type}{}    
    EXEC_RESULT exec() override;
};

struct ActionFactory{
    static Action* create_command_action(InputBuffer* ibuffer, CMD type){
        CommandAction* action = new CommandAction(ibuffer, type);
        return action;
    }
};

#endif