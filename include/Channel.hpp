#pragma once

#include "Client.hpp"

class Client;

class Channel
{
	private:
		std::string					_name;
		std::string					_topic;
		std::string					_key;

		std::vector<Client *>		_members;
		std::vector<Client *>		_channelOperators;
		std::vector<std::string>	_bans;

		bool						_isInviteOnly;
		size_t						_user_limit;

		
		Channel();
		Channel(Channel const &other);
		Channel &operator=(Channel const &other);
	public:
		Channel(std::string name, Client* first);
		~Channel();

		bool				check_bans(const Client& client);
		bool				have_key() const;
		bool				check_key(std::string key) const;
		bool				under_user_limit() const;
		bool				have_member(const Client& client);

		void				add_member(Client* client);

		//getters
		const std::string&	get_name() const;
		const std::string&	get_topic() const;
		const bool&			get_invite_status() const;
};

