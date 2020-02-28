#include "../header/parser.hpp"

PREPARE_RESULT CommandParser::prepare() {
    const std::string command(this->ibuffer->buffer);
    auto it = this->listCommand.find(command);
    if(it != this->listCommand.end()) {
        this->type = it->second;
        return PREPARE_RESULT::PREPARE_SUCCESS;
    }

    return PREPARE_RESULT::PREPARE_UNRECONIZED;
}

Action* CommandParser::parse(){
    return ActionFactory::create_command_action(this->ibuffer, this->type);
}