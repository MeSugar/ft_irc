# Description

The goal of this project is to write an [IRC (Internet Relay Chat)](https://datatracker.ietf.org/doc/html/rfc1459) server to work with a real IRC client.

From subject:
```
Internet Relay Chat or IRC is a text-based communication protocol on the Internet.
It offers real-time messaging that can be either public or private. Users can exchange
direct messages and join group channels.
IRC clients connect to IRC servers in order to join channels. IRC servers are connected
together to form a network.
```

# Usage
```
git clone https://github.com/MeSugar/ft_irc.git
cd ft_irc
make
```

Run executable as follows:
```
./ircserv <port> <password>
```

**port**: The port number on which your IRC server will be listening to for incoming IRC connections.

**password**: The connection password. It will be needed by any IRC client that tries to connect to your server.


To ensure that your server correctly processes everything that you send to it, the following simple test using **nc** can be done:
```
\$> nc 127.0.0.1 6667
command
\$>
```

# Supported client commands
[Here](https://en.wikipedia.org/wiki/List_of_Internet_Relay_Chat_commands) you can find description and syntax

- PASS
- NICK
- USER
- OPER
- QUIT
- JOIN
- PART
- MODE
- TOPIC
- NAMES
- LIST
- INVITE
- KICK
- PRIVMSG
- NOTICE
- KILL
- AWAY
