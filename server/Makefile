# $@ is the name of the target being generated, and $< the first prerequisite (usually a source file).
# For example, consider the following declaration:
# all: library.cpp main.cpp
# In this case:
#   $@ evaluates to all
#   $< evaluates to library.cpp
#   $^ evaluates to library.cpp main.cpp

CPPFLAGS=$(EXTRA_CPPFLAGS)
EXTRA_CPPFLAGS=-std=c++11
LDLIBS=$(EXTRA_LDLIBS)
EXTRA_LDLIBS?=-lGL -lGLEW -lglfw -lpng -lz -lm -lrt -ldl -lX11 -lpthread 
DEPS=cube.h cube_client.h
OBJ=cube_objects.o cube_engine.o cube_data.o cube_simulation.o shader_utils.o base64.o
OBJS=server/mtserver/server.o server/tcpsockets/tcpacceptor.o server/tcpsockets/tcpconnector.o server/tcpsockets/tcpstream.o server/threads/thread.o
all: cube

%.o: %.cpp $(DEPS)
	g++ $(CPPFLAGS) -c -o $@ $<
cube: cube.o $(OBJ)
	g++ -o cube cube.o $(OBJ) $(OBJS) $(LDLIBS)

clean:
	rm -f *.o cube
.PHONY: all clean

