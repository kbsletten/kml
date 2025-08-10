#include <limits.h>
#include <stddef.h>

#ifdef DEBUG
#include <stdio.h>
#else
#undef TRACE_GC
#endif

#define SPEC_END        0x00 /* 00000000 */

#define SPEC_SIZE       0x0F /* 00001111 */

/* Single byte value: bool, char, byte */
#define SPEC_SIZE_8     0x00 /* 00000000 */

/* Two-byte value: short */
#define SPEC_SIZE_16    0x01 /* 00000001 */

/* Four-byte value: int, float */
#define SPEC_SIZE_32    0x02 /* 00000010 */

/* Eight-byte value: long, double */
#define SPEC_SIZE_64    0x03 /* 00000011 */

/* Sixteen-byte value: decimal */
#define SPEC_SIZE_128   0x04 /* 00000100 */

/* Pointer-size value: pointer, index */
#define SPEC_SIZE_PTR   0x0F /* 00001111 */

#define SPEC_TYPE       0x70 /* 01110000 */

/* Plain data, lower bits specify size */
#define SPEC_TYPE_DAT   0x10 /* 00010000 */

/*
	Logical structure.
	The lower bits must be zero.
	The shape of the struct is defined by the following bit pattern ending in SPEC_END.
*/
#define SPEC_TYPE_STR   0x20 /* 00100000 */

/*
	Runtime-defined array value.
	The length is taken from va_args.
	The lower bits specify length field size, allowing for short-indexed arrays.
	The shape of the array items is defined by the following byte.
*/
#define SPEC_TYPE_ARR   0x30 /* 00110000 */

typedef unsigned char U8;
typedef signed char I8;
typedef unsigned short int U16;
typedef short int I16;
typedef unsigned int U32;
typedef signed int I32;
#if SIZE_MAX > UINT_MAX
#define INT_64
typedef size_t U64;
typedef ptrdiff_t I64;
#elif ULONG_MAX > UINT_MAX
#define INT_64
typedef unsigned long int U64;
typedef long int I64;
#endif

/*
	Pointer value.
	The lower bits must be 0x0F.
*/
#define SPEC_TYPE_PTR   0x70 /* 01110000 */

/*
	Flag to define inline arrays.
	The number of items is specified by the following byte.
	If the following byte also has SPEC_MULT set, the number continues into the next byte and so on.
	Overflowing SIZE_MAX results in an error.
*/
#define SPEC_MULT       0x80 /* 10000000 */

/*
 * `spec` is a string that describes the memory layout.
 */
size_t get_mem(const char *spec, void **ptr, size_t *align_ptr, ...);

struct pin_block
{
	struct pin_block *next, *prev;
	void *pin;
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
void safe_point();

#ifdef DEBUG

void set_error_file(FILE *file);

#ifdef TRACE_GC

void set_trace_file(FILE *file);

#endif

#endif
