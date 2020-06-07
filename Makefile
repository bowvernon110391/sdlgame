# default compiler flags
SDL_CFL := $(shell pkg-config SDL2 --cflags --libs)
GL_CFL := -lGL -lGLU
SDL_CFL := $(filter-out -mwindows,$(SDL_CFL))
EXE_FILENAME := sdltest
BIN_DIR := bin

# when debug, disable optimization
# and output debug symbol
ifeq ($(BUILD),debug)
  CFLAGS += -O0 -g
else
  CFLAGS += -O3 -s -DNDEBUG
endif

# if non windows is used
ifeq ($(OS),Windows_NT)
  EXE_FILENAME := sdltest.exe
  GL_CFL := -lopengl32 -lglu32
endif


all:
#	@echo $(SDL_CFL)
	@echo compiling on: $(OS), $(GL_CFL)
	g++ $(CFLAGS) src/*.cpp -o $(BIN_DIR)/$(EXE_FILENAME) $(SDL_CFL) $(GL_CFL)

debug:
	make "BUILD=debug"

clean:
	$(RM) $(BIN_DIR)/*.exe