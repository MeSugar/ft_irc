#pragma once

#include <string>
#include <vector>

class Message
{
	private:
		std::string					_prefix;
		std::string					_command;
		std::vector<std::string>	_params;
	public:
		Message(/* args */);
		~Message();
};

