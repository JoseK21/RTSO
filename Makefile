INCLUDE_DIR = ./include
BIND_DIR = ./bin

ALLEGRO_FLAGS := $(shell pkg-config allegro-5 allegro_font-5 allegro_image-5 allegro_dialog-5 --libs --cflags)
P_THREAD = -pthread
CC = gcc
APP_NAME = testO

all: buildApp

buildApp: $(APP_NAME).c
		gcc -I $(INCLUDE_DIR) $(APP_NAME).c -o $(BIND_DIR)/$(APP_NAME) $(P_THREAD) $(ALLEGRO_FLAGS)

clean:
	rm $(BIND_DIR)/$(APP_NAME)
