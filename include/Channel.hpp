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
		std::vector<std::string>	_speakers;

		bool						_isInviteOnly;
		// bool						_isPrivate;
		// bool						_isSecret;
		// bool						_opTopicOnly;
		bool						_noMessagesFromOutside;
		bool						_isModerated;
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
		bool				have_member(const std::string& nickname);
		bool				have_operator(const std::string& nickname);
		bool				have_speaker(const std::string& nickname);
		bool				empty() const;

		void				add_member(Client* client);
		void				remove_member(Client* client);

		//getters
		const std::string&	get_name() const;
		const std::string&	get_topic() const;
		const bool&			get_invite_status() const;
		const bool& 		get_outside_status() const;
		const bool& 		get_moder_status() const;
		std::vector<Client *> &get_members();
};

