NAME		:= webserv
OBJFILES	:= webserv.o
DEPFILES	:= $(OBJFILES:.o=.d)
# ASAN		:= -fsanitize=address
CXXFLAGS	:= -Wall -Wextra -Werror -Wshadow -std=c++98 -pedantic -MMD $(ASAN)
LDLIBS		:= -lstdc++ $(ASAN)
VPATH		:= srcs

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
	@echo PING | nc localhost $(PORT)
	@echo PONG | nc localhost $(PORT)
	@printf "" | nc localhost $(PORT)
	@# pkill $(NAME)
