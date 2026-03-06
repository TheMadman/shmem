SDL_SOURCE_DIR = ./SDL
SDL_BUILD_DIR ?= $(SDL_SOURCE_DIR)/build

all: client server sdl-client sdl-server

sdl-client sdl-server: %: %.c $(SDL_BUILD_DIR)/libSDL3.so
	# assuming GCC things here, sorry
	$(LINK.c) -Wl,-rpath=$(SDL_BUILD_DIR) -L$(SDL_BUILD_DIR) -I$(SDL_SOURCE_DIR)/include $(OUTPUT_OPTION) $< $(LDLIBS) -lSDL3 

$(SDL_BUILD_DIR)/libSDL3.so: | $(SDL_BUILD_DIR)
	cd $(SDL_BUILD_DIR) && cmake .. && cmake --build . -j

$(SDL_BUILD_DIR):
	mkdir -p $(SDL_BUILD_DIR)
