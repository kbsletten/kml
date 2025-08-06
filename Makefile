CFLAGS=-ansi -pedantic -Wall -Wextra
DFLAGS=-Og -DDEBUG -DTRACE_GC -ggdb --coverage
PFLAGS=-O2
SFLAGS=-fverbose-asm
LFLAGS=

.PHONY: all clean

all: test.exe stress.exe debug/mem.s obj/mem.s

clean:
	rm -f test test.exe stress stress.exe *.gcda *.gcno *gc_trace.txt debug/* obj/*

debug/%.o: %.c %.h
	mkdir -p debug/
	$(CC) -c $< -o $@ $(CFLAGS) $(DFLAGS)

debug/%.s: %.c %.h
	mkdir -p debug/
	$(CC) -S $< -o $@ $(CFLAGS) $(DFLAGS) $(SFLAGS)

obj/%.o: %.c %.h
	mkdir -p obj/
	$(CC) -c $< -o $@ $(CFLAGS) $(PFLAGS)

obj/%.s: %.c %.h
	mkdir -p obj/
	$(CC) -S $< -o $@ $(CFLAGS) $(PFLAGS) $(SFLAGS)

test.exe: debug/mem.o mem.h test.c
	$(CC) test.c debug/mem.o -o test $(CFLAGS) $(DFLAGS) $(LFLAGS)

stress.exe: debug/mem.o mem.h stress.c
	$(CC) stress.c debug/mem.o -o stress $(CFLAGS) $(DFLAGS) $(LFLAGS)
