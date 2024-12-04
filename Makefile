.PHONY = all, clean
cc = g++
CFLAGS = -Werror -fanalyzer -lncurses


all: bashfreeze
bashfreeze: bashfreeze.o
				$(cc) $(CFLAGS) bashfreeze.o -o $@ $^


table: scrollable_table.o class_Process.o processreader.o processreader.h class_Process.h 
				$(cc) $(CFLAGS) -o $@ $^ 

scrollable_table.o: scrollable_table.cpp processreader.h class_Process.h 
class_Process.o: class_Process.cpp processreader.h

processreader.o: processreader.cpp class_Process.h 

%.o: %.c
				$(cc) $(CFLAGS) -c $< 


clean:
				rm -f bashfreeze bashfreeze.o
