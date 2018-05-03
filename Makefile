.POSIX:
.PHONY: all clean install uninstall

CFLAGS := -Wall -Werror $(shell pkg-config --cflags cairo-xlib)
ifdef DEBUG
CFLAGS += -g -DDEBUG
endif

all: libvtk.a

clean:
	rm -f *.o
	rm -f libvtk.a

PREFIX ?= /usr
LIB_DIR ?= $(PREFIX)/lib
PC_DIR ?= $(PREFIX)/lib/pkgconfig
INC_DIR ?= $(PREFIX)/include

install: libvtk.a vtk.pc vtk.h
	install -m 644 -t $(LIB_DIR) libvtk.a
	install -m 644 -t $(PC_DIR) vtk.pc
	install -m 644 -t $(INC_DIR) vtk.h

uninstall:
	rm -f $(LIB_DIR)/libvtk.a
	rm -f $(PC_DIR)/vtk.pc
	rm -f $(INC_DIR)/vtk.h

libvtk.a: error.o event.o key.o root.o window.o
	ar rcs $@ $^

%.o: %.c %.h
	$(CC) -c $< $(CFLAGS)
