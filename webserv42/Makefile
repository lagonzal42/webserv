NAME		:= webserv

LIBS_TARGET	:= 
LIBS		:= 

INCS		:= inc

#------------------------------------------------#
#   FILES                                        #
#------------------------------------------------#
MAIN		= main2 Parser Request Utils WebServer ResponseGenerator_GET ResponseGenerator_POST ResponseGenerator_DELETE MimeDict 


SRC_DIR		:= src
SRCS		:= $(addsuffix .cpp, $(MAIN)) 
# if we want to put files in differ folders:
# $(addprefix <folder root>, $(addsuffix .cpp, $(<folder name above>)))
SRCS		:= $(SRCS:%=$(SRC_DIR)/%)

BUILD_DIR	:= .build
OBJS		:= $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS		:= $(OBJS:.o=.d)

CC			:= c++
CFLAGS		:= -Wall -Wextra -Werror -std=c++98 -pedantic -g
CPPFLAGS	:= $(addprefix -I,$(INCS)) -MMD -MP
LDFLAGS		:= $(addprefix -L,$(dir $(LIBS_TARGET)))
LDLIBS		:= $(addprefix -l,$(LIBS))

RM			:= rm -f
RF			:= rm -rf
DIR_DUP		= mkdir -p $(@D)

#------------------------------------------------#
#   ASCII ART                                    #
#------------------------------------------------#
CLR_RMV		:= \033[0m
RED		    := \033[1;31m
GREEN		:= \033[1;32m
YELLOW		:= \033[1;33m
BLUE		:= \033[1;34m
CYAN 		:= \033[1;36m

define	ART
$(YELLOW)
\___________________ ______________
                   V
        /^-^\         /^-----^\\
       / o o \        V  o o  V
      /   ▼   \        |  ▽  | 
      V \ ∧ / V         \ Q /  
        / - \\          / - \\ 
       /    | ========= |    \\	 
      /     |  webserv  |     \\     
 (===/___) || ========= || (___\\====)$(CLR_RMV)
endef
export	ART

all: $(NAME)

$(NAME): $(OBJS) 
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(NAME)
	$(info CREATED $(NAME))
	@echo "$(YELLOW) __________________________________\n/$(CLR_RMV)"
	@grep "TODO:" log.md
	@echo "$$ART"
	@echo "$(BLUE)"
	@grep "NEXT:" log.md
	@echo "$(CLR_RMV)"

$(LIBS_TARGET):
	$(MAKE) -C $(@D)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(DIR_DUP)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<
	$(info CREATED $@)

-include $(DEPS)

clean:
	$(RM) $(OBJS) $(DEPS)

fclean: clean
	$(RM) $(NAME)
	$(RF) $(BUILD_DIR)
	$(info ELIMINATED $(NAME))

re:
	$(MAKE) fclean
	$(MAKE) all

#------------------------------------------------#
#   SPEC                                         #
#------------------------------------------------#

.PHONY: all clean fclean re
.SILENT:
