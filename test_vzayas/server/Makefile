# NAME #
NAME = webserv

# FLAGS #
CXX = c++
CXXFLAGS = -Wall -Werror -Wextra #-std=c++98

# ASCII COLORS #
RED=\033[0;31m
GREEN=\033[0;32m
BLUE=\033[0;34m
YELLOW=\033[0;33m
MAG=\033[0;35m
END=\033[0m

define ART

$(MAG)
██╗    ██╗███████╗██████╗     ███████╗███████╗██████╗ ██╗   ██╗███████╗██████╗ 
██║    ██║██╔════╝██╔══██╗    ██╔════╝██╔════╝██╔══██╗██║   ██║██╔════╝██╔══██╗
██║ █╗ ██║█████╗  ██████╔╝    ███████╗█████╗  ██████╔╝██║   ██║█████╗  ██████╔╝
██║███╗██║██╔══╝  ██╔══██╗    ╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝██╔══╝  ██╔══██╗
╚███╔███╔╝███████╗██████╔╝    ███████║███████╗██║  ██║ ╚████╔╝ ███████╗██║  ██║
 ╚══╝╚══╝ ╚══════╝╚═════╝     ╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝  ╚══════╝╚═╝  ╚═╝
$(END)     
                                                                     
endef
export ART

# SRCS #
SRCDIR := src/

MAIN = main

PLAIN_SRCS =	$(addsuffix .cpp, $(addprefix main/, $(MAIN)))\
				
SRCS := $(addprefix $(SRCDIR), $(PLAIN_SRCS))

# OBJS #
OBJDIR := obj/
OBJS := $(addprefix $(OBJDIR), $(PLAIN_SRCS:.cpp=.o))

# COMPILATION #
.SILENT:
all: $(NAME)

$(NAME): $(OBJS)
	echo "$(MAG)༺ Creating folders...༻$(END)"
	$(CXX) $(CXXFLAGS) $^ -o $(NAME)
	echo "$(BLUE)༺ Program compiled༻$(END)"
	echo "$$ART"

$(OBJDIR)%.o: $(SRCDIR)%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	echo "$(GREEN)༺ Objects compiled༻$(END)"

clean:
	$(RM) -rf $(OBJDIR)
	echo "$(RED)༺ Objs deleted༻$(END)"

fclean: clean
	$(RM) $(NAME)
	echo "$(YELLOW)༺ Executable deleted༻$(END)"

re: fclean all

.PHONY: all clean fclean re