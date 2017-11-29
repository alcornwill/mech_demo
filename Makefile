

OBJS = cube.c
CC = gcc
INCLUDE_PATHS = -Iinclude\SDL2 -Iinclude
LIBRARY_PATHS = -Llib
# -w suppresses all warnings
COMPILER_FLAGS = -w
# -Wl,-subsystem,windows gets rid of the console window
#COMPILER_FLAGS += -Wl,-subsystem,windows
COMPILER_FLAGS += -Dmain=SDL_main
LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2 -lOpenGL32 -lglew32
OBJ_NAME = bin/cube

all : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)