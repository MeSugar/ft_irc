#pragma once

#include <string>
#include <vector>
#include <map>
#include "Client.hpp"
#include "Channel.hpp"
#include "Message.hpp"

class Client;
class Channel;
class Message;

class Server
{
	private:
		int								_serverSock;
		std::string						_servername;


		std::vector<Client *>			_clients;
		std::vector<Channel *>			_channels;
	public:
		Server();
		~Server();
};
