#
# Cross Platform Makefile
# Compatible with MSYS2/MINGW, Ubuntu 14.04.1 and Mac OS X
#
# Linux:
#   apt-get install freeglut3-dev
#

#CXX = g++
#CXX = clang++

EXE = seamCarver
IMGUI_DIR = ../
SOURCES =  main.cpp
SOURCES += $(IMGUI_DIR)/imguiLib/imgui.cpp $(IMGUI_DIR)/imguiLib/imgui_demo.cpp $(IMGUI_DIR)/imguiLib/imgui_draw.cpp $(IMGUI_DIR)/imguiLib/imgui_tables.cpp $(IMGUI_DIR)/imguiLib/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_glut.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl2.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
UNAME_S := $(shell uname -s)

CXXFLAGS = -std=c++11 -I$(IMGUI_DIR)/imguiLib -I$(IMGUI_DIR)/backends
CXXFLAGS += -g -Wall -Wformat -w
LIBS =

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LIBS += -lGL -lglut -lGL -lGLU -lOpenImageIO -lm
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS += -framework OpenGL -framework GLUT -framework OpenGL -lOpenImageIO -lm
	LIBS += -L/usr/local/lib -L/opt/local/lib

	CXXFLAGS += -I/usr/local/include -I/opt/local/include
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"
	LIBS += -lgdi32 -lopengl32 -limm32 -lglut -lGL -lGLU -lOpenImageIO -lm
ifeq ($(shell pkg-config freeglut --exists 2> /dev/null && echo yes || echo no),yes)
	CXXFLAGS += $(shell pkg-config freeglut --cflags)
	LIBS += $(shell pkg-config freeglut --libs)
else
	LIBS += -lglut 
endif
	CFLAGS = $(CXXFLAGS)
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

# main.o: utils.cpp main.cpp
# 	$(CXX) $(CXXFLAGS) utils.cpp -o utils.o -c
# 	$(CXX) $(CXXFLAGS) main.cpp -o main.o -c
# 	$(CXX) $(CXXFLAGS) -c utils.o main.o

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/imguiLib/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)
