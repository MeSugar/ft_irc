#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include "Client.hpp"
#include "Channel.hpp"
#include "Reply.hpp"

class Client;
class Channel;
struct Message;

class Server : public TemplateRun
{
	private:
		int									_port;
		std::string							_password;
		int									_serverSock;
		std::string							_servername;
		std::vector<std::string>			_MOTD; //rfc 8.5


		std::vector<Client *>				_clients;
		std::vector<Client *>				_connectedClients;
		std::vector<Channel *>				_channels;
		std::map<std::string, std::string>	_operators;


		Server(Server const &other);
		Server &operator=(Server const &other);
		
		// utils
		void	parseMOTD(); // get message of the day
		void	sendMOTD();
		void	sendReply(std::string const &reply) const; // reply management (just for testing, need to be rewritten to send reply to user socket)
		bool	validateNickname(std::string const &nick); // check if nickname contains invalid characters
		bool	comparePrefixAndNick(std::string const &prefix, Client const &client);
		Client	*findClient(std::string const &nick, std::vector<Client *> &clients); // find a client using nickname
		void	removeClient(Client *client, std::vector<Client *> &clients); // removes a client from a list
		void	addClient(Client *client); // adds a client to both client lists 

	public:
		Server(int port, std::string const &password);
		~Server();


		//TEST
		void	server_test_client();
		
		// connection managment
		virtual int run();
        virtual int loop();
        virtual int chat(int fdsock);

		// commands
		void	commandPASS(Client &client, Message &msg);
		void	commandNICK(Client &client, Message &msg);
};
