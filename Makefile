CFLAGS=-ansi -pedantic -Wall -Wextra
DFLAGS=-Og -DDEBUG -DTRACE_GC -ggdb --coverage
PFLAGS=-O2
SFLAGS=-fverbose-asm
LFLAGS=

.PHONY: all clean

all: test.exe debug/mem.s obj/mem.s

clean:
	rm -f test.exe debug/* obj/*

debug/%.o: %.c %.h
	$(CC) -c $< -o $@ $(CFLAGS) $(DFLAGS)

debug/%.s: %.c %.h
	$(CC) -S $< -o $@ $(CFLAGS) $(DFLAGS) $(SFLAGS)

obj/%.o: %.c %.h
	$(CC) -c $< -o $@ $(CFLAGS) $(PFLAGS)

obj/%.s: %.c %.h
	$(CC) -S $< -o $@ $(CFLAGS) $(PFLAGS) $(SFLAGS)

test.exe: debug/mem.o mem.h test.c
	$(CC) test.c debug/mem.o -o test $(CFLAGS) $(DFLAGS) $(LFLAGS)
