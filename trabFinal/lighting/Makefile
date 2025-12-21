CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude -Iglad/include
LDFLAGS = -lglfw -ldl -lGL -lX11 -lpthread

SRC = main.cpp glad/src/glad.c include/tiny_obj_loader.cpp
OBJ = $(SRC:.cpp=.o)
OBJ := $(OBJ:.c=.o)

EXEC = MAZE

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all clean
