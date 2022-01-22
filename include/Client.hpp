#pragma once

#include <cctype>
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

class Client
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
		std::vector<std::string>	_channels_invited;
		bool						_invisible;
		bool						_receive_server_notices;
		bool						_receive_wallops;
		unsigned const				_channelsLimit;
		
		std::vector<std::string>	_nicknameHistory;

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
		virtual ~Client();

		// getters
		bool				getRegistrationStatus() const;
		std::string	const	&getPassword() const;
		std::string const	&getNickname() const;
		std::string const	&getUsername() const;
		std::string const	&getHostname() const;
		const bool&			get_invisible() const;
		const bool&			get_receive_notices() const;
		const bool&			get_receive_wallops() const;
		const bool&			get_operator_status() const;

		// setters
		void	setPassword(std::string const &pass);
		void	setNickname(std::string const &nick);
		void	setUser(std::vector<std::string> &params);

		void	setRegistrationStatus();
		void	setOperatorStatus();

		void	set_invisible(bool status);
		void	set_receive_notices(bool status);
		void	set_receive_wallops(bool status);
		void	set_operator_status(bool status);

		//TEST
		void	client_test_loop(Server& serv);

		void	add_channel(Channel* channel);
		void	remove_channel(Channel *channel);

		bool	check_invitation(const std::string&	ch_name);
		bool	under_channels_limit() const;
		
		// parser
		Message	parse(const char* buf);
		void	command_handle(Message& mes, Server& serv);
};
