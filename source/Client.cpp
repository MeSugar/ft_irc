#include "../include/Client.hpp"

Client::Client()
: _isRegistered(false), _channelsLimit(10)
{}

Client::~Client() {}

// getters
bool	Client::getRegistrationStatus() const { return this->_isRegistered; }


// setters
void    Client::setPassword(std::string const &pass) { this->_password = pass; }