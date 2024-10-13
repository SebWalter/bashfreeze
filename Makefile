.PHONY = all, clean
cc = g++
CFLAGS = -I./include 
LIBS = -L libs/FTXUI -lftxui-component -lftxui-dom -lftxui-screen
all: bashfreeze
bashfreeze:	bashfreeze.o
				$(cc) bashfreeze.o -o bashfreeze $(LIBS) $(CFLAGS)
bashfreeze.o: bashfreeze.cpp
				$(cc) -c bashfreeze.cpp $(CFLAGS)


clean:
				rm -f bashfreeze bashfreeze.o
