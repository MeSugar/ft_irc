#pragma once

#include <iostream>
#include <poll.h>
#include <arpa/inet.h>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <fstream>
#include <cstdlib>
#include <climits>
#include <cctype>
#include <cerrno>
#include "Client.hpp"
#include "Channel.hpp"
#include "Reply.hpp"
#include "Socket.hpp"

class Client;
class Channel;
class Server;
struct Message;

typedef std::map<std::string, void (Server::*)(Client &, Message &)> CommandsMap;

class Server
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
        Socket									*s;

		Server(Server const &other);
		Server &operator=(Server const &other);
		
		// utils
		void	parseMOTD(); // get message of the day
		void	printLog(Message &msg) const;
		void	sendMOTD(Client &client);
		bool	validateNickname(std::string const &nick); // check if nickname contains invalid characters
		bool	comparePrefixAndNick(std::string const &prefix, Client const &client);
		Client	*findClient(std::string const &nick, std::vector<Client *> &clients); // find a client using nickname
		void	removeClient(Client *client, std::vector<Client *> &clients); // removes a client from a given database
		void	addClient(Client *client); // adds a client to clients database
		int		checkOperatorList(std::string const &user, std::string const &pass); // checks whether _operators list contains given user and given pass matches
		bool	checkHostnameList(std::string const &host); // checks whether server allows to become an IRC operator being connected from the client's host
		bool	validateMask(Client &client, const std::string &mask, Message &msg); // checks whether the mask have at least one '.'  in it and no wildcards following the last '.'
		bool	checkUserHostnameByMask(Client const &client, const std::string &mask); // checks whether hostname matches the mask 
		bool	containsText(std::vector<std::string> &params); //checks whether params contain text to be sent
		std::set<std::string>	*checkAndComposeRecipientsList(Client &client, Message &msg,
																std::vector<std::string> &params, 
																std::map<std::string, std::string> *channel_members);
		//checks whether list contains existing channel-, server-, host user names and composes a list of user nicknames to whom a message should be sent

		bool		addRecipientToList(std::set<std::string> &recipients, Client &from, Client *to, Message &msg); //tries to add the client to the list if it's not already present
		Channel*	find_channel(const std::string& name); // find a channel using nickname
		Channel*	add_channel(std::string name, Client& first); //allocates and adds channel, with the first client as its operator
		void		remove_channel(Channel *to_remove); //removes empty channel from the server
		bool		check_channel_name(const std::string& str) const; //checks if the name is valid
		void 		divide_comma(std::vector<std::string> &to, std::string str); //splits a given string with comma as a delimiter
		bool		check_channel_modes(const std::string& str, const Message& msg, Client& client); //checks if the parameters of MODE command are valid
		bool		check_user_modes(const std::string& str, Client& client); //checks if the parameters of MODE command are valid
		bool		floodCheck(Client &client); // true - if(currentTime - lastMessageTime > timeout), else - false 
	public:
		Server(int port, std::string const &password);
		~Server();

		const	std::string&	get_servername() const;
		void	sendReply(Client &client, std::string const &reply) const; // sends reply to clientfd
		
		// connection managment
		virtual int run();
		virtual int loop();
		virtual int chat(Client &client);
		int _recv(int sockfd);
		int _creatpoll(int sockfd);
		void _deletepoll(int sockfd);
		int _handler(std::string msg, int sockfd);
		Client &_findclient(int sockfd);

		//command utils
		void	channel_mode(Client &client, Message &msg); //checks and iterates on channel mode string
		void	user_mode(Client &client, Message &msg); //checks and iterates on user mode string
		void	handle_channel_mode(char sign, char mode, Channel* channel, std::string param, Client& client); //processes channel mode
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
		void	commandAWAY(Client &client, Message &msg);
		void	commandPRIVMSG(Client &client, Message &msg);
		void	commandNOTICE(Client &client, Message &msg);
		void	commandQUIT(Client &client, Message &msg);
		void	commandKILL(Client &client, Message &msg);
};
