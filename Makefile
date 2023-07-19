NAME		:= webserv
OBJFILES	:= webserv.o Webserver.o \
				MimeType.o ReasonPhrase.o Util.o \
				Connection.o Request.o Response.o Middleware.o \
				Server.o Location.o ConfigParser.o ParserError.o \
				HttpException.o Middleware.o Pipeline.o ErrorHandler.o \
				AllowMethodHandler.o AutoIndexHandler.o CGIHandler.o CGI.o \
				IndexHandler.o RedirectHandler.o StaticFileHandler.o UploadHandler.o \
				LimitRequestBodyHandler.o SessionHandler.o KeepAliveHandler.o \

OBJDIR		:= obj
DEPFILES	:= $(OBJFILES:.o=.d) $(OBJFILES:%.o=$(OBJDIR)/%.d)
# ASAN		:= -fsanitize=address
CXXFLAGS	:= -Wall -Wextra -Werror -Wshadow -std=c++98 -MMD $(ASAN) -Isrcs -Isrcs/middlewares
CXXFLAGS  	+= -O3
# CXXFLAGS  	+= -g3
LDLIBS		:= -lstdc++ $(ASAN)
VPATH		:= srcs srcs/middlewares $(OBJDIR) test
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

.PHONY: all clean fclean re test make_test_dir make_test_conf test_config_run

all: $(NAME) $(OBJDIR)

clean:
	$(RM) $(OBJFILES) $(DEPFILES)
	rm -rf $(OBJDIR)
	rm -rf test/test_config.o test/test_config.d

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

test: $(NAME) make_test_dir make_test_conf test_config_run test_cases
	pkill $(NAME) || true
	./$(NAME) YoupiBanane.conf 180 2>&1 > webserv.log &
	time ./tester http://localhost:$(PORT) || bash -c "time ./ubuntu_tester http://localhost:$(PORT)"

tester:
	curl -LO https://cdn.intra.42.fr/document/document/18780/tester
	curl -LO https://cdn.intra.42.fr/document/document/18781/ubuntu_cgi_tester
	curl -LO https://cdn.intra.42.fr/document/document/18782/cgi_tester
	curl -LO https://cdn.intra.42.fr/document/document/18783/ubuntu_tester
	chmod +x tester ubuntu_cgi_tester cgi_tester ubuntu_tester

make_test_dir:
	rm -rf YoupiBanane
	mkdir -p YoupiBanane
	echo z > YoupiBanane/youpi.bad_extension
	touch YoupiBanane/youpi.bla YoupiBanane/youpla.bla
	mkdir -p YoupiBanane/nop
	touch YoupiBanane/nop/youpi.bad_extension YoupiBanane/nop/other.pouic
	mkdir -p YoupiBanane/Yeah
	echo x > YoupiBanane/Yeah/not_happy.bad_extension

make_test_conf:
	echo "\
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
}\n\
server {\n\
    listen 0.0.0.0:$(PORT);\n\
    server_name localhost;\n\
    client_max_body_size 100m;\n\
    add_ext .bad_extension text/plain;\n\
    add_ext .pouic text/plain;\n\
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
}" > YoupiBanane.conf

test_config: test/test_config.o Server.o Location.o ConfigParser.o ParserError.o MimeType.o

test_config_run: test_config
	./$< YoupiBanane.conf
	@printf "\n\nTEST CONFIG COMPLETED SUCCESSFUL. Press ENTER to continue test cases" && read var_x && clear

test_cases:
	./$(NAME) no_extension 2>&1 | grep "Wrong file extension"
	./$(NAME) missing_file.conf 2>&1 | grep "Cannot open file"
	./$(NAME) test/no_server.conf 2>&1 | grep "No server in the configuration file"
	./$(NAME) test/no_ip.conf 2>&1 | grep "No ip in the configuration file"
	./$(NAME) test/no_root.conf 2>&1 | grep "No root in the configuration file"
	./$(NAME) test/invalid_location.conf 2>&1 | grep "Invalid prefix in location"
	./$(NAME) test/invalid_method.conf 2>&1 | grep "Invalid method in location"
	# ./$(NAME) test/conflict_port.conf 2>&1 |grep "Address already in use"

	(pkill $(NAME) || true) && ./$(NAME) test/session.conf 2>&1 > webserv.log &
	sleep 1 \
	&& rm -f cookie \
	&& curl -b cookie -c cookie -v localhost:8080/session.sh 2>&1 | grep "Set-Cookie" \
	&& grep webserv < cookie \
	&& curl -b cookie -c cookie -v localhost:8080/session.sh 2>&1 | grep "HTTP_COOKIE=webserv=" \
	&& curl -b cookie -c cookie -v -d data=hello localhost:8080/session.sh 2>&1 | grep "HTTP_SESSION=" \
	&& curl -b cookie -c cookie -v -d data=world -H "Transfer-Encoding: chunked" localhost:8080/session.sh 2>&1 | grep "HTTP_SESSION=data=hello" \
	&& curl -b cookie -c cookie -v -d data= localhost:8080/session.sh 2>&1 | grep "HTTP_SESSION=data=world" \
	&& curl -b cookie -c cookie -v localhost:8080/session.sh 2>&1 | grep "HTTP_SESSION=" \
	&& grep webserv < cookie \

	(pkill $(NAME) || true) && ./$(NAME) test/location.conf 2>&1 > webserv.log &
	sleep 1 \
	&& curl -s -o - localhost:8080 | grep 404 \
	&& curl -s -o - localhost:8080/ | grep 404 \
	&& curl -s -o - localhost:8080/whatever | grep 404 \
	&& curl -s -o - localhost:8080/dir/ | grep "Index of" \
	&& curl -v -s -o - localhost:8080/dir/ 2>&1  | grep "HTTP/1.1 200 OK" \
	# && curl -v -s -o - localhost:8080/dir 2>&1 | grep "301" \
	# && curl -v -s -o - localhost:8080/dir2/ 2>&1 | grep "403" \

	(pkill $(NAME) || true) && ./$(NAME) test/multi_server.conf 2>&1 > webserv.log &
	sleep 1 \
	&& curl -s -o - --resolve host0.com:8080:127.0.0.1 host0.com:8080 | grep one \
	&& curl -s -o - --resolve host1.com:8080:127.0.0.1 host1.com:8080 | grep one \
	&& curl -s -o - --resolve host2.com:8080:127.0.0.1 host2.com:8080 | grep two \
	&& curl -s -o - --resolve host2.org:8080:127.0.0.1 host2.org:8080 | grep two \
	&& curl -s -o - --resolve host3.com:8081:127.0.0.1 host3.com:8081 | grep three \
	&& curl -s -o - --resolve host3.org:8081:127.0.0.1 host3.org:8081 | grep three \
	&& curl -s -o - --resolve host3.com:8080:127.0.0.1 host3.com:8080 | grep one \

	(pkill $(NAME) || true) && ./$(NAME) test/method.conf  2>&1 > webserv.log &
	sleep 1 \
	&& curl -v -X GET -s -o - localhost:8080/get 2>&1 | grep "REQUEST_METHOD=GET" \
	&& curl -v -X POST -s -o - localhost:8080/post 2>&1 | grep "REQUEST_METHOD=POST" \
	&& curl -v -X PUT -s -o - localhost:8080/put 2>&1 | grep "REQUEST_METHOD=PUT" \
	&& curl -v -X DELETE -s -o - localhost:8080/delete 2>&1 | grep "REQUEST_METHOD=DELETE" \
	&& curl -v -X POST -s -o - localhost:8080/get 2>&1 | grep "HTTP/1.1 405 Method Not Allowed" \
	&& curl -v -X PUT -s -o - localhost:8080/post 2>&1 | grep "HTTP/1.1 405 Method Not Allowed" \
	&& curl -v -X DELETE -s -o - localhost:8080/put 2>&1 | grep "HTTP/1.1 405 Method Not Allowed" \
	&& curl -v -X GET -s -o - localhost:8080/delete 2>&1 | grep "HTTP/1.1 405 Method Not Allowed" \

	(pkill $(NAME) || true) && ./$(NAME) test/post_delete.conf  2>&1 > webserv.log &
	sleep 1 \
	&& mkdir -p wwwroot/upload \
	&& rm -f wwwroot/upload/purple.png \
	&& test ! -f wwwroot/upload/purple.png \
	&& curl -F 'upload=@wwwroot/purple.png' http://localhost:8080/upload 2>&1 | grep "Upload Successful" \
	&& test -f wwwroot/upload/purple.png \
	&& curl -X DELETE http://localhost:8080/upload/purple.png 2>&1 | grep "has been deleted" \
	&& test ! -f wwwroot/upload/purple.png \
	&& curl -v -F 'upload=@wwwroot/purple.png' http://localhost:8080/small 2>&1 | grep "HTTP/1.1 413 Payload Too Large" \

	(pkill $(NAME) || true) && ./$(NAME) test/CGI.conf  2>&1 > webserv.log &
	sleep 1 \
	&& curl -v localhost:8080/cookie/cookie.py 2>&1 | grep "403 Forbidden" \
	&& curl -v localhost:8080/cookie/file_does_not_exist.sh 2>&1 | grep "404 Not Found" \
	&& curl -v localhost:8080/cookie/non_executable.sh 2>&1 | grep "hello" \
	&& curl -v localhost:8080/cookie/timeout.sh 2>&1 | grep "Process has timed out" \
	&& curl -v localhost:8080/cookie/test_envp.sh 2>&1 | grep "PWD" \
	&& curl -v localhost:8080/cookie/cookie.sh?query=string 2>&1 | grep "QUERY_STRING=query=string" \
	&& curl -v -X POST -d "check=input" localhost:8080/cookie/test_input.sh 2>&1 | grep "Input: check=input" \

	(pkill $(NAME) || true) && ./$(NAME) test/redirect.conf 2>&1 > webserv.log &
	sleep 1 \
	&& curl -v localhost:8080 2>&1 | grep "HTTP/1.1 301 Moved Permanently" \
	&& curl -v localhost:8080 2>&1 | grep "Location: https://www.google.com/search?q=banana" \
	&& curl -v localhost:8080/ 2>&1 | grep "HTTP/1.1 301 Moved Permanently" \
	&& curl -v localhost:8080/ 2>&1 | grep "Location: https://www.google.com/search?q=banana" \
	&& curl -v localhost:8080/b 2>&1 | grep "HTTP/1.1 302 Found" \
	&& curl -v localhost:8080/b 2>&1 | grep "Location: https://www.google.com/search?q=durian" \
	&& curl -v localhost:8080/c 2>&1 | grep "HTTP/1.1 404 Not Found" \

	@printf "\n\nTEST CASES COMPLETED SUCCESSFUL. Press ENTER to continue 42 tester" && read var_x && clear

