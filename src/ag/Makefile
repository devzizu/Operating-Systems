OS := $(shell uname)
ifeq ($(OS),Linux)
    CFLAGS= `pkg-config --cflags libxml-2.0` `pkg-config --cflags glib-2.0` -Wall -g -O2
else
    CFLAGS= -g -pedantic -std=c11 -O2
endif

LIBS = `pkg-config --libs libxml-2.0` `pkg-config --libs glib-2.0`
ODIR=obj
CC=gcc
GLOBAL=../GLOBAL_SOURCE/obj/global.o

SOURCES=$(wildcard *.c)
OBJECTS=$(foreach o, $(patsubst %.c, %.o, $(SOURCES)), $(ODIR)/$o)

print-% : ; @echo $* = $($*)

$(ODIR)/%o :
	$(CC) $(CFLAGS) -c -o $@ $(patsubst %.o, %.c, $(patsubst $(ODIR)/%, %, $@))

ag: clean $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(GLOBAL) -o ag $(LIBS)

clean:
	rm -f $(ODIR)/*.o
