CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude -Iglad/include
LDFLAGS = -lglfw -ldl -lGL -lX11 -lpthread

OBJDIR = obj

SRC = main.cpp glad/src/glad.c include/tiny_obj_loader.cpp
OBJ = $(patsubst %.cpp,$(OBJDIR)/%.o,$(filter %.cpp,$(SRC))) \
      $(patsubst %.c,$(OBJDIR)/%.o,$(filter %.c,$(SRC)))

EXEC = MAZE

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) $(LDFLAGS)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(EXEC)

.PHONY: all clean
