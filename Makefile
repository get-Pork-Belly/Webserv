NAME = Webserv
INCLUDES = -I include
CC = clang++
CFLAGS = -Wall -Wextra -Werror -std=c++11 -fsanitize=address -g
RM = rm -rf

# 1 일 때 디폴트 에러 로그, 2일 때 trace로그 추가
DEBUG = -D DEBUG=1
STDOUT = -D STDOUT=1

MAIN_FILES = main PageGenerator Log utils ServerManager ServerGenerator Server Response Request UriParser Exception Base64

SRCS_PATH = $(MAIN_FILES)
SRCS = $(addsuffix .cpp, $(SRCS_PATH))

OBJDIR = ./objs
OBJS = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(MAIN_FILES)))
LOGDIR = ./log
LOGS = $(LOGDIR)/access_log $(LOGDIR)/error_log

PW_GENERATOR_NAME = htpassword_generator
PW_GENERATOR_FILES = Base64 utils passwd
PW_GENERATOR_OBJS = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(PW_GENERATOR_FILES)))

VPATH := .:srcs:tests:srcs/bonus


GREEN = \033[0;32m
RED = \033[0;31m
RESET = \033[0m

all: $(NAME)

$(NAME): $(OBJS)
	@echo "$(GREEN)Making START$(RESET)"
	@$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $(NAME)
	@echo "$(GREEN)DONE"

pwgenerator: $(PW_GENERATOR_NAME)

$(PW_GENERATOR_NAME): $(PW_GENERATOR_OBJS)
	@echo "$(GREEN)Making htpassword_generator START$(RESET)"
	@$(CC) $(CFLAGS) $(INCLUDES) $(PW_GENERATOR_OBJS) -o $(PW_GENERATOR_NAME)

$(OBJDIR)/%.o : %.cpp
	@mkdir -p $(LOGDIR)
	@mkdir -p $(OBJDIR)
	@$(CC) $(CFLAGS) $(INCLUDES) $(DEBUG) $(STDOUT) -c $< -o $@

clean:
	@echo "$(RESET)Cleaning your object files$(RESET)"
	@$(RM) $(OBJDIR) $(LOGS) $(PW_GENERATOR_OBJS)
	@echo "$(RED)clean DONE"

fclean: clean
	@echo "$(RESET)Removing program"
	@$(RM) $(NAME) $(PW_GENERATOR_NAME)
	@echo "$(RED)fclean DONE"

re: fclean all

.PHONY:	all clean fclean re bonus
