#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>

#include "mem.h"

static
size_t get_spec_size(const char **current, size_t *align, va_list *args);

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
size_t get_field_size(const char **current, size_t *align, va_list *args)
{
	size_t field_size, field_count = 1, index_size = 0, item_align = 0;
	char field_type;

	field_type = advance(current);

	if (field_type & SPEC_MULT) {
		field_count = 0;

		while (**current & SPEC_MULT) {
			field_count += advance(current) & 0x7F;

			if (field_count > SIZE_MAX >> 7) {
				return 0;
			}

			field_count <<= 7;
		}

		field_count += advance(current);

		if (!field_count) {
			return 0;
		}
	}

	switch (field_type & SPEC_TYPE) {
		case SPEC_TYPE_DAT:
			*align = field_size = get_field_type_size(field_type);

			if (!field_size) {
				return 0;
			}

			break;

		case SPEC_TYPE_STR:
			if (get_field_type_size(field_type)) {
				return 0;
			}

			field_size = get_spec_size(current, align, args);

			if (advance(current)) {
				return 0;
			}
			
			if (!field_size) {
				return 0;
			}
			break;

		case SPEC_TYPE_ARR:
			index_size = get_field_type_size(field_type);

			if (!index_size) {
				return 0;
			}

			field_size = get_field_size(current, &item_align, args);

			if (!field_size) {
				return 0;
			}

			*align = item_align > index_size ? item_align : index_size;

			return round_up(index_size, item_align) + field_size * field_count * va_arg(*args, size_t);

		case SPEC_TYPE_PTR:
			if ((field_type & SPEC_SIZE) != SPEC_SIZE_PTR) {
				return 0;
			}

			*align = field_size = sizeof(void *);
			break;

		default:
			return 0;
	}

	return field_size * field_count;
}

static
size_t get_spec_size(const char **current, size_t *align, va_list *args)
{
	size_t size = 0, field_size, field_align = 0, length_fields = 0;

	while (**current) {
		field_size = get_field_size(current, &field_align, args);

		size = round_up(size, field_align) + field_size;
		if (*align < field_align) {
			*align = field_align;
		}
	}

	return round_up(round_up(length_fields * sizeof(size_t), *align) + size, *align);
}

size_t get_mem(const char *spec, void **ptr, size_t *align_ptr, ...)
{
	const char *current = spec;
	size_t size = 0, align = 0;
	va_list args;
	va_start(args, align_ptr);

	size = get_spec_size(&current, &align, &args);

	va_end(args);

	if (!size) {
		*ptr = NULL;
		if (align_ptr) {
			*align_ptr = 0;
		}
		return 0;
	}

	*ptr = malloc(size);

	// TODO: set array lengths from va_args, locations and values will need to be calculated in get_spec_size.
	// I need to think about how this will work with nested arrays.

	if (align_ptr) {
		*align_ptr = align;
	}

	return size;
}
