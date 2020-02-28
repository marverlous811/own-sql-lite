#include "../header/action.hpp"

EXEC_RESULT CommandAction::exec(){
    switch (this->type)
    {
    case CMD::EXIT:
        exit(EXIT_SUCCESS);
        break;
    default:
        break;
    }
}