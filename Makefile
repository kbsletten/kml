CFLAGS=-ansi -pedantic -Wall -Wextra
SFLAGS=-fverbose-asm
LFLAGS=

MODE=bin
ARCH=86-64

.PHONY: all clean

all: $(MODE)/$(ARCH)/test $(MODE)/$(ARCH)/stress

clean:
	rm -rf debug/ bin/
	rm -f *.gcda *.gcno *gc_trace.txt

ifeq ($(MODE),bin)
CFLAGS+=-O2
endif

ifeq ($(MODE),debug)
CFLAGS+=-Og -DDEBUG -DTRACE_GC -ggdb --coverage -fsanitize=undefined
endif


$(MODE)/$(ARCH)/%.o: %.c %.h
	mkdir -p $(MODE)/$(ARCH)
	$(CC) -c $< -o $@ $(CFLAGS) $(DFLAGS)

$(MODE)/$(ARCH)/test: $(MODE)/$(ARCH)/mem.o mem.h test.c
	$(CC) test.c $(MODE)/$(ARCH)/mem.o -o $(MODE)/$(ARCH)/test $(CFLAGS) $(LFLAGS)

$(MODE)/$(ARCH)/stress: $(MODE)/$(ARCH)/mem.o mem.h stress.c
	$(CC) stress.c $(MODE)/$(ARCH)/mem.o -o $(MODE)/$(ARCH)/stress $(CFLAGS) $(LFLAGS)

