.POSIX:
.PHONY: all clean

CFLAGS := -Wall -Werror -I.. $(shell pkg-config --cflags --libs cairo-xlib)

all: hello-world mouse transparency

clean:
	rm -f hello-world

%: %.c ../libvtk.a
	$(CC) -o $@ $^ $(CFLAGS)
