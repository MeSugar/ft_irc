#pragma once

#include <string>
#include <vector>

class Message
{
	private:
		std::string					_prefix;
		std::string					_command;
		std::vector<std::string>	_params;

		Message(Message const &other);
		Message &operator=(Message const &other);
	public:
		Message(std::string const &, std::string const &, std::vector<std::string> const &);
		~Message();

		// getters
		std::string					getPrefix() const;
		std::string					getCommand() const;
		std::vector<std::string>	getParams() const;
};

