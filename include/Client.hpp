#pragma once

#include <cctype>
#include <cstdlib>
#include "Server.hpp"
#include "Channel.hpp"

class Server;
class Channel;

struct Message
{
	std::string					prefix;
	std::string					command;
	std::vector<std::string>	params;
};

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

		// parser utils
		int		check_length(const char* buf);
		int		get_prefix(const char* buf, Message& res);
		void    get_command(const char *buf, Message& res, int& i);
		void	get_params(const char *buf, Message& res, int& i);
		int		check_command(const Message& mes);

	public:
		Client();
		~Client();

		// getters
		bool	getRegistrationStatus() const;

		// setters
		void	setPassword(std::string const &pass);
		void	setNickname(std::string const &nick);


		//TEST
		void	client_test_loop(Server& serv);

		// parser
		Message	parse(const char* buf);
		void	command_handle(Message& mes, Server& serv);
};
