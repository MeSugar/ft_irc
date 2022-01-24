#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <cstdlib>
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

		Server(Server const &other);
		Server &operator=(Server const &other);
		
		// utils
		void		parseMOTD(); // get message of the day
		void		sendMOTD();
		bool		validateNickname(std::string const &nick); // check if nickname contains invalid characters
		bool		comparePrefixAndNick(std::string const &prefix, Client const &client);
		Client		*findClient(std::string const &nick, std::vector<Client *> &clients); // find a client using nickname
		void		removeClient(Client *client, std::vector<Client *> &clients); // removes a client from a list
		void		addClient(Client *client); // adds a client to both client lists
		int			checkOperatorList(std::string const &user, std::string const &pass); // checks whether _operators list contains given user and given pass matches
		bool		checkHostnameList(std::string const &host); // checks whether server allowes to become an IRC operator being connected from the client's host 

		Channel*	find_channel(const std::string& name); // find a channel using nickname
		Channel*	add_channel(std::string name, Client& first); //allocates and adds channel, with the first client as its operator
		void		remove_channel(Channel *to_remove); //removes empty channel from the server
		bool		check_channel_name(const std::string& str) const; //checks if the name is valid
		void 		divide_comma(std::vector<std::string> &to, std::string str); //splits a given string with comma as a delimiter
		bool		check_channel_modes(const std::string& str, const Message& msg); //checks if the parameters of MODE command are valid
		bool		check_user_modes(const std::string& str); //checks if the parameters of MODE command are valid


	public:
		Server(int port, std::string const &password);
		~Server();


		const std::string&	get_servername() const;
		void		sendReply(std::string const &reply) const; // reply management (just for testing, need to be rewritten to send reply to user socket)
		//TEST
		void	server_test_client();
		
		// connection managment
		virtual int run();
        virtual int loop();
		std::string	_recv(int sockfd);
        virtual int chat(int fdsock);

		//command utils
		void	channel_mode(Client &client, Message &msg); //checks and iterates on channel mode string
		void	user_mode(Client &client, Message &msg); //checks and iterates on user mode string
		void	handle_channel_mode(char sign, char mode, Channel* channel, std::string param); //processes channel mode
		void	handle_user_mode(char sign, char mode, Client& client); //process user mode
		void	channel_mods_rpl(Channel* channel, Client& client); //sends the list of channel mods to the user
		void	user_mods_rpl(Client& client); //sends the list of user mods to the user
		void	banlist_rpl(Channel* channel, Client& client); //sends the banlist to the user
		void	all_names_rpl(Client& client); //sends all the visible channel and clients to user
		void	all_list_rpl(Client& client); //sends all the visible channels and their topics to user
		
		// commands
		void	commandHandler(Client &client, Message &msg);
		void	commandProcessor(Client &client, Message &msg);
		void	commandPASS(Client &client, Message &msg);
		void	commandNICK(Client &client, Message &msg);
		void	commandUSER(Client &client, Message &msg);
		void	commandOPER(Client &client, Message &msg);
		void	commandJOIN(Client &client, Message &msg);
		void	commandPART(Client &client, Message &msg);
		void	commandMODE(Client &client, Message &msg);
		void	commandTOPIC(Client &client, Message &msg);
		void	commandNAMES(Client &client, Message &msg);
		void	commandLIST(Client &client, Message &msg);
		void	commandINVITE(Client &client, Message &msg);
		void	commandKICK(Client &client, Message &msg);
};
