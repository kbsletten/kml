CFLAGS=-ansi -pedantic -Wall -Wextra
DFLAGS=-Og -DDEBUG -DTRACE_GC -ggdb --coverage -fsanitize=undefined
PFLAGS=-O2
SFLAGS=-fverbose-asm
LFLAGS=

.PHONY: all clean

all: bin/test.exe bin/stress.exe debug/test.exe debug/stress.exe debug/mem.s bin/mem.s

clean:
	rm -f test test.exe stress stress.exe *.gcda *.gcno *gc_trace.txt debug/* bin/*

debug/%.o: %.c %.h
	mkdir -p debug/
	$(CC) -c $< -o $@ $(CFLAGS) $(DFLAGS)

debug/%.s: %.c %.h
	mkdir -p debug/
	$(CC) -S $< -o $@ $(CFLAGS) $(DFLAGS) $(SFLAGS)

bin/%.o: %.c %.h
	mkdir -p bin/
	$(CC) -c $< -o $@ $(CFLAGS) $(PFLAGS)

bin/%.s: %.c %.h
	mkdir -p bin/
	$(CC) -S $< -o $@ $(CFLAGS) $(PFLAGS) $(SFLAGS)

debug/test.exe: debug/mem.o mem.h test.c
	$(CC) test.c debug/mem.o -o debug/test $(CFLAGS) $(DFLAGS) $(LFLAGS)

debug/stress.exe: debug/mem.o mem.h stress.c
	$(CC) stress.c debug/mem.o -o debug/stress $(CFLAGS) $(DFLAGS) $(LFLAGS)

bin/test.exe: bin/mem.o mem.h test.c
	$(CC) test.c bin/mem.o -o bin/test $(CFLAGS) $(PFLAGS) $(LFLAGS)

bin/stress.exe: bin/mem.o mem.h stress.c
	$(CC) stress.c bin/mem.o -o bin/stress $(CFLAGS) $(PFLAGS) $(LFLAGS)
