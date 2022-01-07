#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include "Client.hpp"
#include "Channel.hpp"
#include "Message.hpp"
#include "Reply.hpp"


class Client;
class Channel;
class Message;

class Server
{
	private:
		int									_port;
		std::string							_password;
		int									_serverSock;
		std::string							_servername;
		std::vector<std::string>			_MOTD; //rfc 8.5


		std::vector<Client *>				_clients;
		std::vector<Channel *>				_channels;
		std::map<std::string, std::string>	_operators;


		Server(Server const &other);
		Server &operator=(Server const &other);

		void	parseMOTD();
	public:
		Server(int port, std::string const &password);
		~Server();
};
