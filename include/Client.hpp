#pragma once

#include <cctype>
#include <cstdlib>
#include "TemplateRun.hpp"
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

class Client : public TemplateRun
{
	private:
		bool						_isRegistered;
		bool						_isOperator;
		std::string					_password;
		std::string					_nickname;
		std::string					_hostname;
		std::string					_username;
		std::string const			_servername;
		std::string					_realname;

		// Server const&				_server;
		std::vector<Channel *>		_channels;
		unsigned const				_channelsLimit;
		
		std::vector<std::string>	_nicknameHistory;

		Client(Client const &other);
		Client &operator=(Client const &other);

		// parser utils
		int		check_length(char* buf);
		int		get_prefix(char* buf, Message& res);
		void    get_command(char *buf, Message& res, int& i);
		void	get_params(char *buf, Message& res, int& i);
		int		check_command(Message& mes);

	public:
		Client();
		virtual ~Client();

		// getters
		bool				getRegistrationStatus() const;
		std::string	const	&getPassword() const;
		std::string const	&getNickname() const;
		std::string const	&getUsername() const;
		std::string const	&getHostname() const;

		// setters
		void	setPassword(std::string const &pass);
		void	setNickname(std::string const &nick);
		void	setUser(std::vector<std::string> &params);

		void	setRegistrationStatus();
		void	setOperatorStatus();

		// parser
		Message	parse(char* buf);
};
