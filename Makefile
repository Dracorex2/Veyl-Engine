SRC = $(wildcard src/*.c src/*/*.c)
OBJ = $(SRC:.c=.o)
OBJ := $(OBJ:src/%=%)
OBJ := $(addprefix obj/, $(OBJ))

CC = gcc
LD = gcc


CFLAGS = -Wall -Wextra -Iinclude -g
LDFLAGS = -lSDL3 -lSDL3_image -lcglm -lm

NAME = cube_game

all: $(NAME)

$(NAME): $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) -o $@

obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

shader:
	shadercross shaders/src/shader.vert.hlsl -o shaders/build/shader.vert.spv
	shadercross shaders/src/shader.frag.hlsl -o shaders/build/shader.frag.spv

clean:
	rm -rf obj/

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: re fclean clean all