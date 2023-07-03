NAME		:= webserv
OBJFILES	:= webserv.o Connection.o \
				Webserver.o Request.o Response.o CGI.o \
				Server.o Location.o ConfigParser.o
DEPFILES	:= $(OBJFILES:.o=.d)
ASAN		:= -fsanitize=address
CXXFLAGS	:= -Wall -Wextra -Werror -Wshadow -std=c++98 -MMD $(ASAN) -Isrcs 
# CXXFLAGS  	+= -O3 -flto
# CXXFLAGS  	+= -g3
LDLIBS		:= -lstdc++ $(ASAN)
VPATH		:= srcs

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
testx: $(NAME)
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

test: $(NAME) tester test_dir test_conf
	./$(NAME) YoupiBanane.conf 2>&1 > webserv.log &
	./tester http://localhost:8080 || ./ubuntu_tester http://localhost:8080

tester:
	curl -LO https://cdn.intra.42.fr/document/document/17624/tester
	curl -LO https://cdn.intra.42.fr/document/document/17625/ubuntu_cgi_tester
	curl -LO https://cdn.intra.42.fr/document/document/17626/cgi_tester
	curl -LO https://cdn.intra.42.fr/document/document/17627/ubuntu_tester
	chmod +x tester ubuntu_cgi_tester cgi_tester ubuntu_tester

test_dir:
	mkdir -p YoupiBanane
	echo z > YoupiBanane/youpi.bad_extension 
	touch YoupiBanane/youpi.bla
	mkdir -p YoupiBanane/nop
	touch YoupiBanane/nop/youpi.bad_extension YoupiBanane/nop/other.pouic
	mkdir -p YoupiBanane/Yeah
	echo x > YoupiBanane/Yeah/not_happy.bad_extension

test_conf:
	echo "\
server {\n\
    listen 0.0.0.0:8080;\n\
    server_name localhost;\n\
    client_max_body_size 100m;\n\
    location / {\n\
        root ./YoupiBanane;\n\
        index youpi.bad_extension;\n\
        methods GET;\n\
    }\n\
    location /put_test {\n\
        root ./YoupiBanane;\n\
        methods PUT;\n\
    }\n\
    location /post_body {\n\
        root ./YoupiBanane/post_body;\n\
        methods POST;\n\
    }\n\
    location /directory {\n\
        root ./YoupiBanane;\n\
        index youpi.bad_extension;\n\
        autoindex off;\n\
        cgi_extensions .bla;\n\
        methods GET POST;\n\
    }\n\
}" > YoupiBanane.conf
