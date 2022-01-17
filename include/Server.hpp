#pragma once

#include <iostream>
#include <poll.h>
#include <arpa/inet.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include "Client.hpp"
#include "Channel.hpp"
#include "Reply.hpp"

class Client;
class Channel;
class Server;
struct Message;

typedef std::map<std::string, void (Server::*)(Client &, Message &)> CommandsMap;

class Server : public TemplateRun
{
	private:
		int										_port;
		std::string								_password;
		int										_serverSock;
		std::string								_servername;
		std::vector<std::string>				_MOTD; //rfc 8.5

		std::vector<Client *>					_clients;
		std::vector<Client *>					_connectedClients;
		std::vector<Channel *>					_channels;
		std::map<std::string, std::string>		_operators;
		std::vector<std::string>				_operatorHosts; // list of hostnames whose clinets are allowed to become IRC operator
		CommandsMap								_commands; //list of pairs "command_name->pointer_to_command_method"

		std::string								_message;
		std::vector<struct pollfd>				_userfds;

		Server(Server const &other);
		Server &operator=(Server const &other);
		
		// utils
		void	parseMOTD(); // get message of the day
		void	sendMOTD(Client &client);
		void	sendReply(Client &client, std::string const &reply) const; // sends reply to clientfd
		bool	validateNickname(std::string const &nick); // check if nickname contains invalid characters
		bool	comparePrefixAndNick(std::string const &prefix, Client const &client);
		Client	*findClient(std::string const &nick, std::vector<Client *> &clients); // find a client using nickname
		void	removeClient(Client *client, std::vector<Client *> &clients); // removes a client from a given database
		void	addClient(Client *client); // adds a client to clients database
		int		checkOperatorList(std::string const &user, std::string const &pass); // checks whether _operators list contains given user and given pass matches
		bool	checkHostnameList(std::string const &host); // checks whether server allowes to become an IRC operator being connected from the client's host 

	public:
		Server(int port, std::string const &password);
		~Server();

		// connection managment
		virtual int run();
        // virtual int loop();
        virtual int chat(int fdsock);
		int _recv(int sockfd);
		int _creatpoll(int sockfd);
		int _handler(std::string msg, int sockfd);

		// commands
		void	commandHandler(Client &client, Message &msg);
		void	commandProcessor(Client &client, Message &msg);
		void	commandPASS(Client &client, Message &msg);
		void	commandNICK(Client &client, Message &msg);
		void	commandUSER(Client &client, Message &msg);
		void	commandOPER(Client &client, Message &msg);
};
