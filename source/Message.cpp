#include "../include/Message.hpp"

Message::Message(std::string const &prefix, std::string const &command, std::vector<std::string> const &params)
: _prefix(prefix), _command(command), _params(params)
{}

Message::~Message() {}

// getters
std::string					Message::getPrefix() const { return this->_prefix; }
std::string					Message::getCommand() const { return this->_command; }
std::vector<std::string>	Message::getParams() const { return this->_params; }