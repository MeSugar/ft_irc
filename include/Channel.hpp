#pragma once

#include "Client.hpp"

class Channel
{
	private:
		std::string				_name;
		std::string				_topic;
		std::string				_key;

		std::vector<Client *>	_members;
		std::vector<Client *>	_channelOperators;

		bool					_isInviteOnly;
	public:
		Channel(/* args */);
		~Channel();
};

