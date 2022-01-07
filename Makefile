NAME		=	ircserv

CFLAGS		=	-Wall -Wextra -Werror -std=c++98

CC			=	clang++

INCLUDES	=	include/Channel.hpp include/Client.hpp include/Message.hpp \
				include/Reply.hpp include/Server.hpp

SRCS		=	source/main.cpp source/Client.cpp source/Reply.cpp source/Server.cpp 

all : $(NAME)

$(NAME) : $(SRCS) $(INCLUDES)
		@$(CC) -o $(NAME) $(CFLAGS) $(SRCS) -I.

clean :
		@rm -rf $(NAME)

fclean : clean
		@rm -rf $(NAME)

re : fclean all

.PHONY: all clean fclean re