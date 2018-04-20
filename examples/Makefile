.POSIX:
.PHONY: all clean

CFLAGS := -Wall -Werror -I.. -L.. $(shell pkg-config --cflags --libs cairo-xlib xi ../vtk.pc)

all: hello-world mouse scroll transparency

clean:
	rm -f hello-world

%: %.c
	$(CC) -o $@ $^ $(CFLAGS)
