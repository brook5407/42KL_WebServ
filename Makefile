NAME		:= webserv
OBJFILES	:= webserv.o Connection.o \
				Webserver.o Request.o Response.o CGI.o \
				server.o location.o ConfigParser.o
DEPFILES	:= $(OBJFILES:.o=.d)
ASAN		:= -fsanitize=address
CXXFLAGS	:= -Wall -Wextra -Werror -Wshadow -std=c++98 -MMD $(ASAN) -g -Isrc -I.
LDLIBS		:= -lstdc++ $(ASAN)
VPATH		:= src

# for Kishyan wsl without g++
ifeq ($(shell which g++), )
	CXX			:= clang
	CXXFLAGS	+= -fPIE
endif

.PHONY: all clean fclean re test

all: $(NAME)

clean:
	$(RM) $(OBJFILES) $(DEPFILES)

fclean: clean
	$(RM) $(NAME)

re: fclean all

-include $(DEPFILES)

$(NAME): $(OBJFILES)

PORT := 8888
test: $(NAME)
	./$(NAME) $(PORT)&
	@sleep 1
	curl localhost:$(PORT)
	curl -H "Transfer-Encoding: chunked" -d "key=value&name=text" localhost:8080 -v
	curl -H "Transfer-Encoding: chunked" -H "Content-Disposition: attachment; filename=\"filename.png\"" --data-binary @Makefile localhost:8080 -v
	curl -v -F key1=value1 -F upload=@Makefile localhost:9999 -v
	@echo PING | nc localhost $(PORT)
	@echo PONG | nc localhost $(PORT)
	@printf "" | nc localhost $(PORT)
	@# pkill $(NAME)