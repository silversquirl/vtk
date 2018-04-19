.POSIX:
.PHONY: all clean

CFLAGS := -Wall -Werror $(shell pkg-config --cflags cairo-xlib)

all: libvtk.a

clean:
	rm -f *.o
	rm -f libvtk.a

libvtk.a: error.o root.o window.o
	ar rcs $@ $^

%.o: %.c %.h
	$(CC) -c $< $(CFLAGS)
