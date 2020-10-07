ALLEGRO_FLAGS := $(shell pkg-config allegro-5 allegro_font-5 allegro_image-5 --libs --cflags)
CC = gcc
FILE = RTOS

all: buildTemApp

buildTemApp: $(FILE).c
		$(CC) $(FILE).c -o $(FILE) $(ALLEGRO_FLAGS)
		./$(FILE)

clean:
	rm $(FILE)


# gcc v2.c -o v2 $(pkg-config allegro-5 allegro_font-5 allegro_image-5  --libs --cflags)