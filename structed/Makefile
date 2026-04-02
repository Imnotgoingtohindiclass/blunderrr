CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 $(shell sdl2-config --cflags)
LDFLAGS = $(shell sdl2-config --libs) -lSDL2_image

SRCS    = main.c render.c input.c game_logic.c
OBJS    = $(SRCS:.c=.o)
TARGET  = chess

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c chess.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean