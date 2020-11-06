NAME = Webserv
INCLUDES = -I include
CC = clang++
CFLAGS = -Wall -Wextra -Werror -std=c++11 -fsanitize=address -g
RM = rm -rf
# 1 일 때 디폴트 에러 로그, 2일 때 trace로그 추가
DEBUG = -D DEBUG=2
STDOUT = -D STDOUT=1

# MAIN_FILES = setRouteAndLocationInfo_test
MAIN_FILES = Cgi_test PageGenerator Log utils ServerManager ServerGenerator Server Response Request UriParser Exception
# MAIN_FILES = PageGenerator_test PageGenerator

SRCS_PATH = $(MAIN_FILES)
VPATH := .:srcs:tests
SRCS = $(addsuffix .cpp, $(SRCS_PATH))

OBJDIR = ./objs
OBJS = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(MAIN_FILES)))
LOGDIR = ./log
LOGS = $(LOGDIR)/access_log $(LOGDIR)/error_log

# TEST_FILES = split_test
# TEST_NAME = test
# TESTDIR = ./tests
# TESTS_PATH = $(TEST_FILES)
# TESTS = $(addprefix $(TESTDIR)/, $(addsuffix .cpp, $(TESTS_PATH)))
# TEST_OBJS = $(addprefix $(TESTDIR)/, $(addsuffix .o, $(TESTS_FILES)))

GREEN = \033[0;32m
RED = \033[0;31m
RESET = \033[0m

all: $(NAME)

${NAME}: ${OBJS}
	@echo "$(GREEN)Making START$(RESET)"
	@${CC} ${CFLAGS} ${INCLUDES} ${OBJS} -o ${NAME}
	@echo "$(GREEN)DONE"

$(OBJDIR)/%.o : %.cpp
	@mkdir -p $(LOGDIR)
	@mkdir -p $(OBJDIR)
	@${CC} ${CFLAGS} ${INCLUDES} ${DEBUG} ${STDOUT} -c $< -o $@

# test: $(TEST_NAME)

# ${TEST_NAME}: ${TEST_OBJS}
# 	@echo "Test your functions"
# 	@${CC} ${INCLUDES} ${TEST_OBJS} -o ${TEST_NAME}
# 	@./test

# $(TESTDIR)/%.o : %.cpp
# 	@${CC} ${INCLUDES} -c $< -o $@

clean:
	@echo "$(RESET)Cleaning your object files$(RESET)"
	@${RM} $(OBJDIR) $(LOGS)
	@echo "$(RED)clean DONE"

fclean: clean
	@echo "$(RESET)Removing program"
	@${RM} $(NAME)
	@echo "$(RED)fclean DONE"

re: fclean all

.PHONY:	all clean fclean re bonus
