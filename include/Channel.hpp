#pragma once

#include "Client.hpp"

#define MAX_MEMBERS 65535

class Client;
class Server;

class Channel
{
	private:
		Server*						_server;
		std::string					_name;
		std::string					_topic;
		std::string					_key;

		std::vector<Client *>		_members;
		std::vector<Client *>		_channelOperators;
		std::vector<std::string>	_bans;
		std::vector<std::string>	_speakers;

		bool						_isInviteOnly;
		bool						_isPrivate;
		bool						_isSecret;
		bool						_opTopicOnly;
		bool						_noMessagesFromOutside;
		bool						_isModerated;
		size_t						_user_limit;

		
		Channel();
		Channel(Channel const &other);
		Channel &operator=(Channel const &other);
	public:
		Channel(Server* server, std::string name, Client* first);
		~Channel();

		bool				check_bans(const Client& client);
		bool				have_banmask(const std::string& ban);
		bool				have_key() const;
		bool				check_key(std::string key) const;
		bool				under_user_limit() const;
		bool				have_member(const Client& client);
		bool				have_member(const std::string& nickname);
		bool				have_operator(const Client& client);
		bool				have_operator(const std::string& nickname);
		bool				have_speaker(const std::string& nickname);
		bool				empty() const;
		bool				operators_empty() const;

		void				send_message(const std::string& str);
		
		void				add_member(Client* client);
		void				remove_member(Client* client);
		void				add_operator(Client* client);
		void				add_operator(const std::string& nickname);
		void				remove_operator(Client* client);
		void				remove_operator(const std::string& nickname);
		void				make_any_operator(); //makes the first user a channel operator
		void				add_speaker(const std::string& nickname);
		void				remove_speaker(const std::string& nickname);
		void				add_banmask(const std::string& ban);
		void				remove_banmask(const std::string& ban);
		void				remove_key();

		//setters
		void				set_invite_status(bool status);
		void				set_private_status(bool status);
		void				set_secret_status(bool status);
		void				set_topic_status(bool status);
		void				set_outside_status(bool status);
		void				set_moder_status(bool status);
		void				set_user_limit(std::string n, char sign);
		void				set_key(const std::string& key);
		void				set_topic(const std::string& topic);
		
		//getters
		const std::string&	get_name() const;
		const std::string&	get_topic() const;
		const std::string&	get_key() const;
		const bool&			get_invite_status() const;
		const bool&			get_private_status() const;
		const bool&			get_secret_status() const;
		const bool&			get_topic_status() const;
		const bool&			get_outside_status() const;
		const bool&			get_moder_status() const;
		const size_t&		get_user_limit() const;
		const std::vector<std::string>&	get_banlist() const;
};

