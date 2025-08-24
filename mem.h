#ifndef NDEBUG
#include <assert.h>
#endif

#ifdef DEBUG
#include <stdio.h>
#else
#undef TRACE_GC
#endif

#include "types.h"

/*
 * `spec` is a string that describes the memory layout.
 */
size_t get_mem(const char *spec, interior_ptr_t *ptr, size_t *align_ptr, ...);

struct pin_block
{
	struct pin_block *next, *prev;
	interior_ptr_t pin;
};

/*
 * pin the specified pointer as a gc root
 */
void pin_mem(struct pin_block *pin);

/*
 * remove the specified pointer
 */
void unpin_mem(struct pin_block *pin);

/*
 * Check to see if there is work to be done on the gc
 */
void safe_point(void);

#ifdef DEBUG

void set_error_file(FILE *file);

#ifdef TRACE_GC

void set_trace_file(FILE *file);

#endif

#endif
