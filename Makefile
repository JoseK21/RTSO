INCLUDE_DIR = ./include
BIND_DIR = ./bin
ALLEGRO_FLAGS := $(shell pkg-config allegro-5 allegro_font-5 allegro_image-5 --libs --cflags)
CC = gcc
APP_NAME = RTOS

all: buildTemApp

buildTemApp: $(APP_NAME).c
		gcc -I $(INCLUDE_DIR) $(APP_NAME).c -o $(BIND_DIR)/$(APP_NAME) $(ALLEGRO_FLAGS) 

clean:
	rm $(BIND_DIR)/$(APP_NAME)
