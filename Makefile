.PHONY = all, clean
cc = g++
CFLAGS = -Werror -lncurses -g
SOURCES = processreader_worker.cpp class_Process.cpp scrollable_table.cpp processreader.cpp
OBJS    = $(SOURCES:%.cpp=%.o)




all: bashfreeze
bashfreeze: bashfreeze.o $(OBJS)
				$(cc) $(CFLAGS) -o $@ $^


table: scrollable_table.o class_Process.o processreader.o processreader.h class_Process.h 
				$(cc) $(CFLAGS) -o $@ $^ 

bashfreeze.o: processreader.h class_Process.h scrollable_table.h processreader_worker.h
scrollable_table.o:  processreader.h class_Process.h 
class_Process.o:  processreader.h

processreader.o: class_Process.h 

%.o: %.cpp
				$(cc) $(CFLAGS) -c $< 


clean:
				rm -f bashfreeze bashfreeze.o $(OBJS) 
