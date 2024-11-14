.PHONY = all, clean
cc = g++
CFLAGS = -Werror -fanalyzer -lncurses
all: bashfreeze
bashfreeze: bashfreeze.o
				$(cc) $(CFLAGS) bashfreeze.o -o $@ $^
%.o: %.c
				$(cc) $(CFLAGS) -c $< 


clean:
				rm -f bashfreeze bashfreeze.o
