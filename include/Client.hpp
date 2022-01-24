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
		int							_clientFd;
		bool						_isRegistered;
		bool						_isOperator;
		bool						_isAway;
		std::string					_awayMessage;
		std::string					_password;
		std::string					_nickname;
		std::string					_hostname;
		std::string					_username;
		std::string const			_servername;
		std::string					_realname;

		// Server const&				_server;
		std::vector<Channel *>		_channels;
		unsigned const				_channelsLimit;
		std::vector<std::string>	_channels_invited;
		time_t						_lastMessageTime;
		time_t						_messageTimeout;
		
		std::vector<std::string>	_nicknameHistory;

		Client(Client const &other);
		Client &operator=(Client const &other);

		// parser utils
		int		check_length(const char* buf);
		int		get_prefix(const char* buf, Message& res);
		void    get_command(const char *buf, Message& res, int& i);
		void	get_params(const char *buf, Message& res, int& i);
		int		check_command(Message& mes);

	public:
		Client(int sockfd);
		virtual ~Client();

		// getters
		int					getClientFd() const;
		bool				getRegistrationStatus() const;
		bool				getAwayStatus() const;
		std::string	const	&getAwayMessage() const;
		std::string	const	&getPassword() const;
		std::string const	&getNickname() const;
		std::string const	&getUsername() const;
		std::string const	&getHostname() const;
		time_t				getLastMessageTime() const;
		time_t				getMessageTimeout() const;

		// setters
		void	setClientFd(int fd);
		void	setPassword(std::string const &pass);
		void	setNickname(std::string const &nick);
		void	setHostname(std::string const &hostname);
		void	setUser(std::vector<std::string> &params);

		void	setRegistrationStatus();
		void	setOperatorStatus();
		void	setAwayStatus(const std::string &msg = std::string());
		void	setLastMessageTime(time_t time);
		void	setMessageTimeout(time_t time);

		void	add_channel(Channel* channel);
		void	remove_channel(Channel *channel);

		bool	check_invitation(const std::string&	ch_name);
		bool	under_channels_limit() const;

		// parser
		Message	parse(const char* buf);
};
