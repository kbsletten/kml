#include <stdio.h>
#include <assert.h>

#include "mem.h"

struct list_cons {
	struct list_cons *tail;
	I32 value;
};

static
const char *list_cons_spec = "\x7F\x12";

struct list_cons *cons(struct list_cons *car, I32 cdr)
{
	struct list_cons *result;
	void *ptr = car;
	size_t size;

       	size = get_mem(list_cons_spec, &ptr, NULL);

	if (size != sizeof(struct list_cons))
	{
		fprintf(stderr, "Expected %lu, got %lu.\n", (unsigned long int)sizeof(struct list_cons), (unsigned long int)size);
		assert(size == sizeof(struct list_cons));
	}

	result = ptr;
	result->tail = car;
	result->value = cdr;

	return result;
}

static
unsigned long int passed = 0, failed = 0;

int main()
{
	struct pin_block pin = { NULL, NULL, NULL };
	I32 i, ex_sum = 0, list_sum = 0;
	struct list_cons *list = NULL;

#ifdef DEBUG
	FILE *trace_file = fopen("stress_gc_trace.txt", "w+");

	set_error_file(stderr);
	set_trace_file(trace_file);
#endif

	pin_mem(&pin);

	for (i = 0; i < 1000; i++)
	{
		pin.pin = cons(pin.pin, i);
		ex_sum += i;
	}

	safe_point();

	for (list = pin.pin; list; list = list->tail)
	{
		list_sum += list->value;
	}

	if (ex_sum == list_sum) {
		passed++;
	} else {
		failed++;
		printf("Expected sum: %d, List sum: %d\n", ex_sum, list_sum);
	}

	unpin_mem(&pin);

#ifdef DEBUG
	fclose(trace_file);
#endif

	fprintf(stdout, "%lu of %lu tests passed.\n", passed, passed + failed);

	return failed;
}
