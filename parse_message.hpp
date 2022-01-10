#ifndef PARSE_MESSAGE_HPP
# define PARSE_MESSAGE_HPP

#include "Message.hpp"

Message	parse(char* buf);
int	check_command(Message& mes);

#endif
