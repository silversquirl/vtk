.POSIX:
.PHONY: all clean

CFLAGS := -Wall -Werror -I.. $(shell pkg-config --cflags --libs cairo-xlib xi)

all: hello-world mouse scroll transparency

clean:
	rm -f hello-world

%: %.c ../libvtk.a
	$(CC) -o $@ $^ $(CFLAGS)
