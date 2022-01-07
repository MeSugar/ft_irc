#pragma once

#include "Message.hpp"
#include "Server.hpp"
#include "Channel.hpp"

class Message;
class Server;
class Channel;

class Client
{
	private:
		int							_clientSock;
		std::string					_nickname;
		std::string					_hostname;
		std::string					_username;
		std::string const			_servername;
		std::string					_realname;
		std::string					_password;

		// Server const&				_server;
		std::vector<Channel *>		_channels;
		unsigned const				_channelsLimit;
		
		bool						_isRegistered;
		bool						_isOperator;

		Client(Client const &other);
		Client &operator=(Client const &other);

	public:
		Client();
		~Client();
};
