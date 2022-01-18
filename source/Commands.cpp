#include "../include/Server.hpp"

void	Server::commandHandler(Client &client, Message &msg)
{
	if (this->_commands.find(msg.command) != this->_commands.end())
		this->commandProcessor(client, msg);
	else
		this->sendReply(client, generateErrorReply(this->_servername, ERR_UNKNOWNCOMMAND, client.getNickname(), msg.command));
}

void	Server::commandProcessor(Client &client, Message &msg)
{
	if (!client.getRegistrationStatus() && msg.command != "PASS" && msg.command != "NICK" && msg.command != "USER" && msg.command != "QUIT")
		this->sendReply(client, generateErrorReply(this->_servername, ERR_NOTREGISTERED, client.getNickname(), msg.command));
	else
	{
		CommandsMap::iterator it = this->_commands.find(msg.command);
		(this->*it->second)(client, msg);
	}
}

void	Server::commandPASS(Client &client, Message &msg)
{
	if (msg.prefix.empty() && client.getNickname().empty() && client.getUsername().empty())
	{
		if (client.getRegistrationStatus())
			this->sendReply(client, generateErrorReply(this->_servername, ERR_ALREADYREGISTRED));
		else if (msg.params.empty())
			this->sendReply(client, generateErrorReply(this->_servername, ERR_NEEDMOREPARAMS, "", "PASS"));
		else if (msg.params[0] != this->_password || msg.params.size() > 1)
			this->sendReply(client, generateErrorReply(this->_servername, ERR_PASSWDMISMATCH, "", "PASS"));
		else
			client.setPassword(msg.params[0]);
	}
}

void	Server::commandNICK(Client &client, Message &msg)
{
	if ((msg.prefix.empty() || this->comparePrefixAndNick(msg.prefix, client)) && !client.getPassword().empty())
	{
		if (msg.params.empty())
			this->sendReply(client, generateErrorReply(this->_servername, ERR_NONICKNAMEGIVEN, client.getNickname(), "NICK"));
		else if (msg.params.size() != 1 || !this->validateNickname(msg.params[0]))
			this->sendReply(client, generateErrorReply(this->_servername, ERR_ERRONEUSNICKNAME, client.getNickname(), msg.params[0]));
		else if (this->findClient(msg.params[0], this->_connectedClients))
			this->sendReply(client, generateErrorReply(this->_servername, ERR_NICKNAMEINUSE, client.getNickname(), msg.params[0]));
		else
		{
			Client *tmp = this->findClient(msg.params[0], this->_clients);
			if (tmp != NULL)
				removeClient(tmp, this->_clients);
			client.setNickname(msg.params[0]);
			this->addClient(&client);
		}
	}
}

void	Server::commandUSER(Client &client, Message &msg)
{
	if (msg.prefix.empty() && !client.getPassword().empty())
	{
		if (client.getRegistrationStatus())
			this->sendReply(client, generateErrorReply(this->_servername, ERR_ALREADYREGISTRED, client.getNickname()));
		else if (msg.params.size() < 4)
			this->sendReply(client, generateErrorReply(this->_servername, ERR_NEEDMOREPARAMS, client.getNickname(), "USER"));
		else
		{
			client.setUser(msg.params);
			this->addClient(&client);
		}
	}
}

void	Server::commandOPER(Client &client, Message &msg)
{
	if (msg.prefix.empty() || this->comparePrefixAndNick(msg.prefix, client))
	{
		if (msg.params.size() < 4)
			this->sendReply(client, generateErrorReply(this->_servername, ERR_NEEDMOREPARAMS, client.getNickname(), "OPER"));
		else if (msg.params.size() == 2 && this->checkOperatorList(msg.params[0], msg.params[1]) == ERR_WRONGUSERNAME)
			this->sendReply(client, generateErrorReply(this->_servername, ERR_WRONGUSERNAME, client.getNickname(), "OPER"));
		else if (msg.params.size() == 2 && this->checkOperatorList(msg.params[0], msg.params[1]) == ERR_PASSWDMISMATCH)
			this->sendReply(client, generateErrorReply(this->_servername, ERR_PASSWDMISMATCH, client.getNickname(), "OPER"));
		else if (!this->checkHostnameList(client.getHostname()))
			this->sendReply(client, generateErrorReply(this->_servername, ERR_NOOPERHOST, client.getNickname(), "OPER"));
		else
		{
			client.setOperatorStatus();
			this->sendReply(client, generateNormalReply(this->_servername, RPL_YOUREOPER, client.getNickname(), "OPER"));
		}
	}
}

