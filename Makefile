.POSIX:
.PHONY: all clean

CFLAGS := -Wall -Werror $(shell pkg-config --cflags cairo-xlib)
ifdef DEBUG
CFLAGS += -g -DDEBUG
endif

all: libvtk.a

clean:
	rm -f *.o
	rm -f libvtk.a

libvtk.a: error.o event.o root.o window.o
	ar rcs $@ $^

%.o: %.c %.h
	$(CC) -c $< $(CFLAGS)
