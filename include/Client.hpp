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
		bool						_isRegistered;
		std::string					_password;
		std::string					_nickname;
		std::string					_hostname;
		std::string					_username;
		std::string const			_servername;
		std::string					_realname;

		// Server const&				_server;
		std::vector<Channel *>		_channels;
		unsigned const				_channelsLimit;
		
		bool						_isOperator;

		Client(Client const &other);
		Client &operator=(Client const &other);

	public:
		Client();
		~Client();

		// getters
		bool	getRegistrationStatus() const;

		// setters
		void	setPassword(std::string const &pass);
		void	setNickname(std::string const &nick);

};
