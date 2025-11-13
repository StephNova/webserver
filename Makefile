#Program name
NAME = webserv

#Compiler
CC = c++

#Flags
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g

#Object files folder
OBJDIR = obj

#Cleanup
RM = rm -rf

#Source files and objects
SRCS = main.cpp \
		src/DuplicateSocketCheck.cpp \
		src/ConfigFile.cpp src/ConfigFileChecks.cpp src/ConfigFileVariables.cpp \
		src/Router.cpp \
		src/Server.cpp src/ServerSocket.cpp \
		src/ServerLoop.cpp src/ServerUtils.cpp \
		src/Request.cpp src/RequestChecks.cpp src/RequestParsing.cpp\
		src/Response.cpp \
		src/CGI.cpp \
		src/Utils.cpp \
		src/Cluster.cpp

OBJ = $(patsubst %.cpp, $(OBJDIR)/%.o, $(SRCS))

#Targets
all: $(NAME)

#Link object files into target executable
$(NAME): $(OBJ)
	$(CC) $(CXXFLAGS) $(OBJ) -o $@

#Object file generation
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c $< -o $@

#Clean up
clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all

#Phony targets
.PHONY: all clean fclean re
