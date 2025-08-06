#include <stdio.h>
#include <assert.h>

#include "mem.h"

struct list_cons {
	struct list_cons *tail;
	int value;
};

static
const char *list_cons_spec = "\x7F\x13";

struct list_cons *cons(struct list_cons *car, int cdr)
{
	struct list_cons *result;
	void *ptr = car;
	size_t size;

       	size = get_mem(list_cons_spec, &ptr, NULL);

	assert(size == sizeof(struct list_cons));

	result = ptr;
	result->tail = car;
	result->value = cdr;

	return result;
}

int main()
{
	struct pin_block pin = { NULL, NULL, NULL };
	FILE *trace_file = fopen("stress_gc_trace.txt", "w+");
	int i;

	set_error_file(stderr);
	set_trace_file(trace_file);

	pin_mem(&pin);

	for (i = 0; i < 1000; i++)
	{
		pin.pin = cons(pin.pin, i);
	}

	safe_point();

	unpin_mem(&pin);

	return 0;
}
