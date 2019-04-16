NAME = ExpertSystem

GCC = g++

CXXFLAGS=-std=c++11 -Wall -Wextra -Werror

SRC =	main.c

OBJ = $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
		$(GCC) -o $(NAME) $(OBJ)

%.o: %.c
	$(GCC) -c -o $@ $<

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all
