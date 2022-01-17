NAME		=	ircserv

CFLAGS		=	-Wall -Wextra -Werror -std=c++98

CC			=	clang++

INCLUDES	=	include/Channel.hpp include/Client.hpp include/Reply.hpp \
				include/Server.hpp include/Socket.hpp include/TemplateRun.hpp

SRCS		=	source/main.cpp source/Client.cpp source/Reply.cpp source/Server.cpp \
				source/Socket.cpp source/TemplateRun.cpp source/Commands.cpp source/ServerUtils.cpp

all : $(NAME)

$(NAME) : $(SRCS) $(INCLUDES)
		@$(CC) -o $(NAME) $(CFLAGS) $(SRCS) -I.

clean :
		@rm -rf $(NAME)

fclean : clean
		@rm -rf $(NAME)

re : fclean all

.PHONY: all clean fclean re