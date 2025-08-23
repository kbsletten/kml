#include <stdio.h>
#include <assert.h>

#include "mem.h"

struct list_cons {
	interior_ptr_t tail;
	I32 value;
};

static
const char *list_cons_spec = "\x7F\x12";

interior_ptr_t cons(interior_ptr_t car, I32 cdr)
{
	interior_ptr_t ptr;
	struct list_cons *list;
	size_t size;

	size = get_mem(list_cons_spec, &ptr, NULL);

	if (size != sizeof(struct list_cons))
	{
		fprintf(stderr, "Expected %lu, got %lu.\n", (unsigned long int)sizeof(struct list_cons), (unsigned long int)size);
		assert(size == sizeof(struct list_cons));
	}

	list = MEM_PTR(ptr);
	list->tail = car;
	list->value = cdr;

	return ptr;
}

static
unsigned long int passed = 0, failed = 0;

int main(void)
{
	struct pin_block pin = { 0 };
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

	for (list = MEM_PTR(pin.pin); list; list = MEM_PTR(list->tail))
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
