#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>

#include "mem.h"

#ifdef DEBUG

#include <stdio.h>
static
FILE *error_file = NULL;

void set_error_file(FILE *file)
{
	error_file = file;
}

#endif

static
struct mem_info {
	size_t size;
	size_t align;
} mem_zero;

static
struct field_info {
	struct mem_info mem;
	size_t count;
} field_zero;

struct array_info {
	size_t size;
	size_t length;
};

#define ARRAY_MAX 8

static
struct arrays_info {
	unsigned int count;
	struct array_info arrays[ARRAY_MAX];
} arrays_zero;

static
struct mem_info get_spec_size(struct arrays_info *arrays, const char **current, va_list *args);

static
struct field_info get_field_size(struct arrays_info *arrays, const char **current, va_list *args);

static
size_t round_up(size_t value, size_t alignment)
{
	return (value + alignment - 1) & ~(alignment - 1);
}

static
char advance(const char **current)
{
	char c = **current;
	*current += 1;
	return c;
}

static
size_t get_field_type_size(char field_type)
{
	if ((field_type & SPEC_SIZE) == SPEC_SIZE_PTR) {
		return sizeof(void *);
	} else {
		return 1 << (field_type & SPEC_SIZE);
	}
}

static
size_t max_value(size_t size) {
	return (1 << (size * 8)) - 1;
}

static
struct field_info get_array_size(size_t length_size, struct arrays_info *arrays, const char **current, va_list *args)
{
	struct field_info field;
	struct array_info *array;

	if (arrays->count >= ARRAY_MAX) {
#ifdef DEBUG
		fprintf(error_file, "Error: arrays->count exceeds ARRAY_MAX in get_array_size\n");
#endif
		return field_zero;
	}

	array = &arrays->arrays[arrays->count++];

	array->size = length_size;
	array->length = va_arg(*args, size_t);

	if (array->size < sizeof(size_t) && array->length > max_value(array->size)) {
#ifdef DEBUG
		fprintf(error_file, "Error: array->length exceeds SIZE_MAX / array->size in get_array_size\n");
#endif
		return field_zero;
	}

	field = get_field_size(arrays, current, args);

	if (!field.mem.size) {
#ifdef DEBUG
		fprintf(error_file, "Error: field.mem.size is zero in get_array_size\n");
#endif
		return field_zero;
	}

	field.count *= array->length;

	return field;
}

static
struct field_info get_field_size(struct arrays_info *arrays, const char **current, va_list *args)
{
	struct field_info field = { { 0, 0 }, 1 };
	char field_type;

	field_type = advance(current);

	if (field_type & SPEC_MULT) {
		field.count = 0;

		while (**current & SPEC_MULT) {
			field.count += advance(current) & 0x7F;

			if (field.count > SIZE_MAX >> 7) {
#ifdef DEBUG
				fprintf(error_file, "Error: field.count overflow in get_field_size\n");
#endif
				return field_zero;
			}

			field.count <<= 7;
		}

		field.count += advance(current);

		if (!field.count) {
#ifdef DEBUG
			fprintf(error_file, "Error: field.count is zero in get_field_size\n");
#endif
			return field_zero;
		}
	}

	switch (field_type & SPEC_TYPE) {
		case SPEC_TYPE_DAT:
			field.mem.align = field.mem.size = get_field_type_size(field_type);
			break;

		case SPEC_TYPE_STR:
			if (field_type & SPEC_SIZE) {
#ifdef DEBUG
				fprintf(error_file, "Error: field_type & SPEC_SIZE nonzero for SPEC_TYPE_STR in get_field_size\n");
#endif
				return field_zero;
			}

			field.mem = get_spec_size(arrays, current, args);

			if (advance(current)) {
#ifdef DEBUG
				fprintf(error_file, "Error: advance(current) nonzero for SPEC_TYPE_STR in get_field_size\n");
#endif
				return field_zero;
			}
			break;

		case SPEC_TYPE_ARR:
			if (field.count > 1) {
#ifdef DEBUG
				fprintf(error_file, "Error: field.count > 1 for SPEC_TYPE_ARR in get_field_size\n");
#endif
				return field_zero;
			}

			field = get_array_size(get_field_type_size(field_type), arrays, current, args);
			break;

		case SPEC_TYPE_PTR:
			if ((field_type & SPEC_SIZE) != SPEC_SIZE_PTR) {
#ifdef DEBUG
				fprintf(error_file, "Error: SPEC_TYPE_PTR but not SPEC_SIZE_PTR in get_field_size\n");
#endif
				return field_zero;
			}

			field.mem.align = field.mem.size = sizeof(void *);
			break;

		default:
#ifdef DEBUG
			fprintf(error_file, "Error: Unknown SPEC_TYPE in get_field_size\n");
#endif
			return field_zero;
	}

	if (!field.mem.size) {
#ifdef DEBUG
		fprintf(error_file, "Error: field.mem.size is zero in get_field_size\n");
#endif
		return field_zero;
	}

	return field;
}

static
struct mem_info get_spec_size(struct arrays_info *arrays, const char **current, va_list *args)
{
	struct mem_info mem = mem_zero;
	struct field_info field;

	while (**current) {
		field = get_field_size(arrays, current, args);

		if (!field.mem.size) {
#ifdef DEBUG
			fprintf(error_file, "Error: field.mem.size is zero in get_spec_size\n");
#endif
			return mem_zero;
		}

		mem.size = round_up(mem.size, field.mem.align);
	       	mem.size += field.mem.size * field.count;
		mem.size += (round_up(field.mem.size, field.mem.align) - field.mem.size) * (field.count - 1);
		if (field.mem.align > mem.align) {
			mem.align = field.mem.align;
		}
	}

	return mem;
}

size_t get_mem(const char *spec, void **ptr, size_t *align_ptr, ...)
{
	const char *current = spec;
	struct mem_info mem, length = mem_zero;
	struct arrays_info arrays = arrays_zero;
	va_list args;
	unsigned int i;

	va_start(args, align_ptr);

	mem = get_spec_size(&arrays, &current, &args);

	va_end(args);

	if (!mem.size) {
		return 0;
	}

	for (i = 0; i < arrays.count; i++) {
		length.size = round_up(length.size, arrays.arrays[i].size) + arrays.arrays[i].size;
		if (arrays.arrays[i].size > length.align) {
			length.align = arrays.arrays[i].size;
		}
	}

	mem.size += round_up(length.size, mem.align);
	if (length.align > mem.align) {
		mem.align = length.align;
	}

	*ptr = malloc(mem.size);

	if (align_ptr) {
		*align_ptr = mem.align;
	}

	return mem.size;
}
