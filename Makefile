NAME		:= webserv
OBJFILES	:= webserv.o Webserver.o \
				Connection.o Request.o Response.o \
				Server.o Location.o ConfigParser.o ParserError.o \
				HttpException.o Middleware.o Pipeline.o ErrorHandler.o \
				AllowMethodHandler.o AutoIndexHandler.o CGIHandler.o CGI.o \
				IndexHandler.o RedirectHandler.o StaticFileHandler.o UploadHandler.o \
				LimitRequestBodyHandler.o SessionHandler.o

OBJDIR		:= obj
DEPFILES	:= $(OBJFILES:.o=.d) $(OBJFILES:%.o=$(OBJDIR)/%.d)
ASAN		:= -fsanitize=address
CXXFLAGS	:= -Wall -Wextra -Werror -Wshadow -std=c++98 -MMD $(ASAN) -Isrcs -Isrcs/middlewares
# CXXFLAGS  	+= -O3 -flto
CXXFLAGS  	+= -g3
LDLIBS		:= -lstdc++ $(ASAN)
VPATH		:= srcs srcs/middlewares $(OBJDIR)
PORT		:= 8080

ifeq ($(shell uname), Linux)
	CGI_TESTER	= ubuntu_cgi_tester
else
	CGI_TESTER	= cgi_tester
endif

# for Kishyan wsl without g++
ifeq ($(shell which g++), )
	CXX			:= clang
	CXXFLAGS	+= -fPIE
endif

.PHONY: all clean fclean re test

all: $(NAME) $(OBJDIR)

clean:
	$(RM) $(OBJFILES) $(DEPFILES)
	rm -rf $(OBJDIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

-include $(DEPFILES)

$(NAME): $(OBJFILES)

$(OBJDIR): $(OBJFILES)
	mkdir -p $(OBJDIR)
	mv *.d *.o $(OBJDIR)

testx: $(NAME)
	curl -H "Transfer-Encoding: chunked" -d "key=value&name=text" localhost:8080 -v
	curl -H "Transfer-Encoding: chunked" -H "Content-Disposition: attachment; filename=\"filename.png\"" --data-binary @Makefile localhost:8080 -v
	curl -v -F key1=value1 -F upload=@Makefile localhost:9999 -v
	@echo PING | nc localhost $(PORT)

test: $(NAME) tester test_dir test_conf test_cases
	pkill $(NAME) || true
	./$(NAME) YoupiBanane.conf 2>&1 > webserv.log &
	time ./tester http://localhost:$(PORT) || bash -c "time ./ubuntu_tester http://localhost:$(PORT)"

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
    listen 0.0.0.0:$(PORT);\n\
    server_name localhost;\n\
    client_max_body_size 100m;\n\
    location / {\n\
        root ./YoupiBanane;\n\
        autoindex on;\n\
        methods GET;\n\
    }\n\
    location /put_test/ {\n\
        root ./YoupiBanane;\n\
        methods PUT;\n\
    }\n\
    location /post_body {\n\
        root ./YoupiBanane/post_body;\n\
        methods POST;\n\
		client_max_body_size 100;\n\
    }\n\
    location /directory {\n\
        root ./YoupiBanane;\n\
        index youpi.bad_extension;\n\
        add_cgi .bla $(CGI_TESTER);\n\
        methods GET POST;\n\
    }\n\
}\n\
server {\n\
    listen 0.0.0.0:$(PORT);\n\
    server_name 127.0.0.1;\n\
    client_max_body_size 1m;\n\
    location / {\n\
        root ./wwwroot;\n\
        methods GET POST;\n\
        autoindex on;\n\
        add_cgi .sh /bin/sh;\n\
        add_cgi .cgi ./;\n\
        add_cgi .py /usr/bin/python3;\n\
    }\n\
}" > YoupiBanane.conf

test_cases:
	./$(NAME) no_extension 2>&1 | grep "Wrong file extension"
	./$(NAME) missing_file.conf 2>&1 | grep "Cannot open file"
	./$(NAME) test/no_server.conf 2>&1 | grep "No server in the configuration file"
	./$(NAME) test/no_ip.conf 2>&1 | grep "No ip in the configuration file"
	./$(NAME) test/no_root.conf 2>&1 | grep "No root in the configuration file"
	./$(NAME) test/invalid_location.conf 2>&1 | grep "Invalid prefix in location"
	./$(NAME) test/conflict_port.conf 2>&1 |grep "Address already in use"

	(pkill $(NAME) || true) && screen -dm ./$(NAME) test/location.conf && sleep 1 \
	&& curl -s -o - localhost:8080 | grep 404 \

	(pkill $(NAME) || true) && screen -dm ./$(NAME) test/multi_server.conf && sleep 1 \
	&& curl -s -o - --resolve host0.com:8080:127.0.0.1 host0.com:8080 | grep one \
	&& curl -s -o - --resolve host1.com:8080:127.0.0.1 host1.com:8080 | grep one \
	&& curl -s -o - --resolve host2.com:8080:127.0.0.1 host2.com:8080 | grep two \
	&& curl -s -o - --resolve host2.org:8080:127.0.0.1 host2.org:8080 | grep two \
	&& curl -s -o - --resolve host3.com:8081:127.0.0.1 host3.com:8081 | grep three \
	&& curl -s -o - --resolve host3.org:8081:127.0.0.1 host3.org:8081 | grep three \
	&& curl -s -o - --resolve host3.com:8080:127.0.0.1 host3.com:8080 | grep one \

	(pkill $(NAME) || true) && screen -dm ./$(NAME) test/redirect.conf && sleep 1 \
	&& curl -v localhost:8080 2>&1 | grep 301 \
	&& curl -v localhost:8080 2>&1 | grep banana \
	&& curl -v localhost:8080/b 2>&1 | grep 302 \
	&& curl -v localhost:8080/b 2>&1 | grep durian \
