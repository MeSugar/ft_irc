#include "../include/Reply.hpp"

std::string generateErrorReply(std::string const &from, int const errorCode,
										std::string const &to,
										std::string const &arg1,
										std::string const &arg2)
{
	std::stringstream ss;
	ss << errorCode;
	std::string prefix(":" + from + " " + ss.str() + " " + to + " ");
	switch (errorCode)
	{
		case ERR_NOSUCHNICK:
			return (std::string(prefix + arg1 + " " + arg2 + " :No such nick/channel\n"));
		case ERR_NOSUCHSERVER:
			return (std::string(prefix + arg1 + " " + arg2 + " :No such server\n"));
		case ERR_NOSUCHCHANNEL:
			return (std::string(prefix + arg1 + " " + arg2 + " :No such channel\n"));
		case ERR_CANNOTSENDTOCHAN:
			return (std::string(prefix + arg1 + " " + arg2 + " :Cannot send to channel\n"));
		case ERR_TOOMANYCHANNELS:
			return (std::string(prefix + arg1 + " " + arg2 + " :You have joined too many channels\n"));
		case ERR_WASNOSUCHNICK:
			return (std::string(prefix + arg1 + " " + arg2 + " :There was no such nickname\n"));
		case ERR_TOOMANYTARGETS:
			return (std::string(prefix + arg1 + " " + arg2 + " :Duplicate recipients. No message delivered\n"));
		case ERR_NOORIGIN:
			return (std::string(prefix + arg1 + " " + arg2 + " :No origin specified\n"));
		case ERR_NORECIPIENT:
			return (std::string(prefix + " :No recipient given (" + arg1 + ")\n"));
		case ERR_NOTEXTTOSEND:
			return (std::string(prefix + arg1 + " " + arg2 + " :No text to send\n"));
		case ERR_NOTOPLEVEL:
			return (std::string(prefix + arg1 + " " + arg2 + " :No toplevel domain specified\n"));
		case ERR_WILDTOPLEVEL:
			return (std::string(prefix + arg1 + " " + arg2 + " :Wildcard in toplevel domain\n"));
		case ERR_UNKNOWNCOMMAND:
			return (std::string(prefix + arg1 + " " + arg2 + " :Unknown command\n"));
		case ERR_NOMOTD:
			return (std::string(prefix + arg1 + " " + arg2 + " :MOTD File is missing\n"));
		case ERR_NOADMININFO:
			return (std::string(prefix + arg1 + " " + arg2 + " :No administrative info available\n"));
		case ERR_FILEERROR:
			return (std::string(prefix + " :File error doing " + arg1 + " on " + arg2 + "\n"));
		case ERR_NONICKNAMEGIVEN:
			return (std::string(prefix + arg1 + " " + arg2 + " :No nickname given\n"));
		case ERR_ERRONEUSNICKNAME:
			return (std::string(prefix + arg1 + " " + arg2 + " :Erroneus nickname\n"));
		case ERR_NICKNAMEINUSE:
			return (std::string(prefix + arg1 + " " + arg2 + " :Nickname is already in use\n"));
		case ERR_NICKCOLLISION:
			return (std::string(prefix + arg1 + " " + arg2 + " :Nickname collision KILL\n"));
		case ERR_USERNOTINCHANNEL:
			return (std::string(prefix + arg1 + " " + arg2 + " :They aren't on that channel\n"));
		case ERR_NOTONCHANNEL:
			return (std::string(prefix + arg1 + " " + arg2 + " :You're not on that channel\n"));
		case ERR_USERONCHANNEL:
			return (std::string(prefix + arg1 + " " + arg2 + " :is already on channel\n"));
		case ERR_NOLOGIN:
			return (std::string(prefix + arg1 + " " + arg2 + " :User not logged in\n"));
		case ERR_SUMMONDISABLED:
			return (std::string(prefix + arg1 + " " + arg2 + " :SUMMON has been disabled\n"));
		case ERR_USERSDISABLED:
			return (std::string(prefix + arg1 + " " + arg2 + " :USERS has been disabled\n"));
		case ERR_NOTREGISTERED:
			return (std::string(prefix + arg1 + " " + arg2 + " :You have not registered\n"));
		case ERR_NEEDMOREPARAMS:
			return (std::string(prefix + arg1 + " " + arg2 + " :Not enough parameters\n"));
		case ERR_ALREADYREGISTRED:
			return (std::string(prefix + arg1 + " " + arg2 + " :You may not reregister\n"));
		case ERR_NOPERMFORHOST:
			return (std::string(prefix + arg1 + " " + arg2 + " :Your host isn't among the privileged\n"));
		case ERR_PASSWDMISMATCH:
			return (std::string(prefix + arg1 + " " + arg2 + " :Password incorrect\n"));
		case ERR_YOUREBANNEDCREEP:
			return (std::string(prefix + arg1 + " " + arg2 + " :You are banned from this server\n"));
		case ERR_KEYSET:
			return (std::string(prefix + arg1 + " " + arg2 + " :Channel key already set\n"));
		case ERR_CHANNELISFULL:
			return (std::string(prefix + arg1 + " " + arg2 + " :Cannot join channel (+l)\n"));
		case ERR_UNKNOWNMODE:
			return (std::string(prefix + arg1 + " " + arg2 + " :is unknown mode char to me\n"));
		case ERR_INVITEONLYCHAN:
			return (std::string(prefix + arg1 + " " + arg2 + " :Cannot join channel (+i)\n"));
		case ERR_BANNEDFROMCHAN:
			return (std::string(prefix + arg1 + " " + arg2 + " :Cannot join channel (+b)\n"));
		case ERR_BADCHANNELKEY:
			return (std::string(prefix + arg1 + " " + arg2 + " :Cannot join channel (+k)\n"));
		case ERR_NOPRIVILEGES:
			return (std::string(prefix + arg1 + " " + arg2 + " :Permission Denied- You're not an IRC operator\n"));
		case ERR_CHANOPRIVSNEEDED:
			return (std::string(prefix + arg1 + " " + arg2 + " :You're not channel operator\n"));
		case ERR_CANTKILLSERVER:
			return (std::string(prefix + arg1 + " " + arg2 + " :You cant kill a server!\n"));
		case ERR_NOOPERHOST:
			return (std::string(prefix + arg1 + " " + arg2 + " :No O-lines for your host\n"));
		case ERR_UMODEUNKNOWNFLAG:
			return (std::string(prefix + arg1 + " " + arg2 + " :Unknown MODE flag\n"));
		case ERR_USERSDONTMATCH:
			return (std::string(prefix + arg1 + " " + arg2 + " :Cant change mode for other users\n"));
		default:
			return (std::string(prefix + arg1 + " " + arg2 + " :Something bad happend, but we really don't know\n"));
	}
}

std::string generateNormalReply(std::string const &from, int const replyCode,
										std::string const &to,
										std::string const &arg1,
										std::string const &arg2,
										std::string const &arg3,
										std::string const &arg4,
										std::string const &arg5,
										std::string const &arg6,
										std::string const &arg7)
{
	std::stringstream ss;
	ss << replyCode;
	std::string prefix(":" + from + " " + ss.str() + " " + to + " ");
	switch (replyCode)
	{
		case RPL_USERHOST:
			return (std::string(prefix + ":" + arg1 + " " + arg2 + " " + arg3 + " " + arg4 + " " + arg5  + "\n"));
		case RPL_ISON:
			return (std::string(prefix + ":" + arg1 + "\n"));
		case RPL_AWAY:
			return (std::string(prefix + arg1 + " :" + arg2 + "\n"));
		case RPL_UNAWAY:
			return (std::string(prefix + ":You are no longer marked as being away\n"));
		case RPL_NOWAWAY:
			return (std::string(prefix + ":You have been marked as being away\n"));
		case RPL_WHOISUSER:
			return (std::string(prefix + arg1 + " " + arg2 + " " + arg3 + " * :" + arg4 + "\n"));
		case RPL_WHOISSERVER:
			return (std::string(prefix + arg1 + " " + arg2 + " :" + arg3 + "\n"));
		case RPL_WHOISOPERATOR:
			return (std::string(prefix + arg1 + " :is an IRC operator\n"));
		case RPL_WHOISIDLE:
			return (std::string(prefix + arg1 + " " + arg2 + " :seconds idle\n"));
		case RPL_ENDOFWHOIS:
			return (std::string(prefix + arg1 + " :End of /WHOIS list\n"));
		// case RPL_WHOISCHANNELS:
		// 	return (std::string(prefix + arg1 + "\n")); scarry shit
		case RPL_WHOWASUSER:
			return (std::string(prefix + arg1 + " " + arg2 + " " + arg3 + " * :" + arg4 + "\n"));
		case RPL_ENDOFWHOWAS:
			return (std::string(prefix + arg1 + " :End of WHOWAS\n"));
		case RPL_LISTSTART:
			return (std::string(prefix + "Channel :Users  Name\n"));
		case RPL_LIST:
			return (std::string(prefix + arg1 + " " + arg2 + " :" + arg3 + "\n"));
		case RPL_LISTEND:
			return (std::string(prefix + ":End of /LIST\n"));
		case RPL_CHANNELMODEIS:
			return (std::string(prefix + arg1 + " " + arg2 + " " + arg3 + "\n"));
		case RPL_NOTOPIC:
			return (std::string(prefix + arg1 + " :No topic is set\n"));
		case RPL_TOPIC:
			return (std::string(prefix + arg1 + " :" + arg2 + "\n"));
		case RPL_INVITING:
			return (std::string(prefix + arg1 + " " + arg2 + "\n"));
		case RPL_SUMMONING:
			return (std::string(prefix + arg1 + " :Summoning user to IRC\n"));
		case RPL_VERSION:
			return (std::string(prefix + arg1 + "." + arg2 + " " + arg3 + " :" + arg4 + "\n"));
		// case RPL_WHOREPLY:
		// 	return (std::string(prefix + arg1 + " " + arg2 + " " + arg3 + " " + arg4 + " " + arg5 + "\n")); scarry shit
		case RPL_ENDOFWHO:
			return (std::string(prefix + arg1 + " :End of /WHO list\n"));
		// case RPL_NAMREPLY:
		// 	return (std::string(prefix + arg1 + " :" + arg2 + "\n")); scarry shit
		case RPL_ENDOFNAMES:
			return (std::string(prefix + arg1 + " :End of /NAMES list\n"));
		case RPL_LINKS:
			return (std::string(prefix + arg1 + " " + arg2 + ": " + arg3 + " " + arg4 + "\n"));
		case RPL_ENDOFLINKS:
			return (std::string(prefix + arg1 + " :End of /LINKS list\n"));
		case RPL_BANLIST:
			return (std::string(prefix + arg1 + " " + arg2 + "\n"));
		case RPL_ENDOFBANLIST:
			return (std::string(prefix + arg1 + " :End of channel ban list\n"));
		case RPL_INFO:
			return (std::string(prefix + ":" + arg1 + "\n"));
		case RPL_ENDOFINFO:
			return (std::string(prefix + ":End of /INFO list\n"));
		case RPL_MOTDSTART:
			return (std::string(prefix + ":- " + arg1 + " Message of the day - \n"));
		case RPL_MOTD:
			return (std::string(prefix + ":- " + arg1 + "\n"));
		case RPL_ENDOFMOTD:
			return (std::string(prefix + ":End of /MOTD command\n"));
		case RPL_YOUREOPER:
			return (std::string(prefix + ":You are now an IRC operator\n"));
		case RPL_REHASHING:
			return (std::string(prefix + arg1 + " :Rehashing\n"));
		case RPL_TIME:
			return (std::string(prefix + arg1 + " :" + arg2 + "\n"));
		case RPL_USERSSTART:
			return (std::string(prefix + ":UserID   Terminal  Host\n"));
		case RPL_USERS:
			return (std::string(prefix + ":%-8s %-9s %-8s\n"));
		case RPL_ENDOFUSERS:
			return (std::string(prefix + ":End of users\n"));
		case RPL_NOUSERS:
			return (std::string(prefix + ":Nobody logged in\n"));
		case RPL_TRACELINK:
			return (std::string(prefix + "Link " + arg1 + " " + arg2 + " " + arg3 + "\n"));
		case RPL_TRACECONNECTING:
			return (std::string(prefix + "Try. " + arg1 + " " + arg2 + "\n"));
		case RPL_TRACEHANDSHAKE:
			return (std::string(prefix + "H.S. " + arg1 + " " + arg2 + "\n"));
		case RPL_TRACEUNKNOWN:
			return (std::string(prefix + "???? " + arg1 + " " + arg2 + "\n"));
		case RPL_TRACEOPERATOR:
			return (std::string(prefix + "Oper " + arg1 + " " + arg2 + "\n"));
		case RPL_TRACEUSER:
			return (std::string(prefix + "User " + arg1 + " " + arg2 + "\n"));
		// case RPL_TRACESERVER:
		// 	return (std::string(prefix + "Serv " + arg1 + " " + arg2 + "S " + arg3 + "C " + arg4 + " " + arg5 + "@" + "\n")); scarry shit
		case RPL_TRACENEWTYPE:
			return (std::string(prefix + arg1 + " 0 " + arg2 + "\n"));
		case RPL_TRACELOG:
			return (std::string(prefix + "File " + arg1 + " " + arg2 + "\n"));
		case RPL_STATSLINKINFO:
			return (std::string(prefix + arg1 + " " + arg2 + " " + arg3 + " " + arg4 + " " + arg5 + " " + arg6 + " " + arg7 + "\n"));
		case RPL_STATSCOMMANDS:
			return (std::string(prefix + arg1 + " " + arg2 + "\n"));
		case RPL_STATSCLINE:
			return (std::string(prefix + "C " + arg1 + " * " + arg2 + " " + arg3 + " " + arg4 + "\n"));
		case RPL_STATSNLINE:
			return (std::string(prefix + "N " + arg1 + " * " + arg2 + " " + arg3 + " " + arg4 + "\n"));
		case RPL_STATSILINE:
			return (std::string(prefix + "I " + arg1 + " * " + arg2 + " " + arg3 + " " + arg4 + "\n"));
		case RPL_STATSKLINE:
			return (std::string(prefix + "K " + arg1 + " * " + arg2 + " " + arg3 + " " + arg4 + "\n"));
		case RPL_STATSYLINE:
			return (std::string(prefix + "Y " + arg1 + " " + arg2 + " " + arg3 + " " + arg4 + "\n"));
		case RPL_ENDOFSTATS:
			return (std::string(prefix + arg1 + " :End of /STATS report\n"));
		case RPL_STATSLLINE:
			return (std::string(prefix + "L " + arg1 + " * " + arg2 + " " + arg3 + "\n"));
		case RPL_STATSUPTIME:
			return (std::string(prefix + ":Server Up %d days %d:%02d:%02d\n"));
		case RPL_STATSOLINE:
			return (std::string(prefix + "O " + arg1 + " * " + arg2 + "\n"));
		case RPL_STATSHLINE:
			return (std::string(prefix + "H " + arg1 + " * " + arg2 + "\n"));
		case RPL_UMODEIS:
			return (std::string(prefix + arg1 + "\n"));
		case RPL_LUSERCLIENT:
			return (std::string(prefix + ":There are " + arg1 + " users and " + arg2 + " invisible on " + arg3 + " servers\n"));
		case RPL_LUSEROP:
			return (std::string(prefix + arg1 + " :operator(s) online\n"));
		case RPL_LUSERUNKNOWN:
			return (std::string(prefix + arg1 + " :unknown connection(s)\n"));
		case RPL_LUSERCHANNELS:
			return (std::string(prefix + arg1 + " :channels formed\n"));
		case RPL_LUSERME:
			return (std::string(prefix + ":I have " + arg1 + " clients and " + arg2 + " servers\n"));
		case RPL_ADMINME:
			return (std::string(prefix + arg1 + " :Administrative info\n"));
		case RPL_ADMINLOC1:
			return (std::string(prefix + arg1 + "\n"));
		case RPL_ADMINLOC2:
			return (std::string(prefix + arg1 + "\n"));
		case RPL_ADMINEMAIL:
			return (std::string(prefix + arg1 + "\n"));
		default:
			return (std::string(prefix + arg1 + " " + arg2 + " :Something happend, but we really don't know\n"));
	}
}