#include <stdio.h>

#include "mem.h"

static
unsigned long int passed = 0, failed = 0;

#define DEBUG_SPEC(spec) spec, #spec

static
void fail_get_mem(const char *spec, const char *debug) {
	void *ptr = NULL;
	size_t align = 0;
	size_t result = get_mem(spec, &ptr, &align);
	if (result || ptr || align) {
		fprintf(stderr, "get_mem(%s) failed: expected error, got %lu bytes with alignment %lu\n", debug, (unsigned long int)result, (unsigned long int)align);
		failed++;
	} else {
		passed++;
	}
}

static
void test_get_mem(const char *spec, const char *debug, size_t expected_size, size_t expected_align) {
	void *ptr = NULL;
	size_t align = 0;
	size_t result = get_mem(spec, &ptr, &align);
	if (result != expected_size || !ptr || align != expected_align) {
		fprintf(stderr, "get_mem(%s) failed: expected %lu bytes with alignment %lu, got %lu bytes with alignment %lu\n", debug, (unsigned long int)expected_size, (unsigned long int)expected_align, (unsigned long int)result, (unsigned long int)align);
		failed++;
	} else {
		passed++;
	}
}

static
void test_get_mem_arr(const char *spec, const char *debug, size_t dim, size_t expected_size, size_t expected_align) {
	void *ptr = NULL;
	size_t align = 0;
	size_t result = get_mem(spec, &ptr, &align, dim);
	if (result != expected_size || !ptr || align != expected_align) {
		fprintf(stderr, "get_mem(%s) failed: expected %lu bytes with alignment %lu, got %lu bytes with alignment %lu\n", debug, (unsigned long int)expected_size, (unsigned long int)expected_align, (unsigned long int)result, (unsigned long int)align);
		failed++;
	} else {
		passed++;
	}
}

int main()
{
	fail_get_mem(DEBUG_SPEC(""));
	fail_get_mem(DEBUG_SPEC("\x40"));
	fail_get_mem(DEBUG_SPEC("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"));
	fail_get_mem(DEBUG_SPEC("\x80\x00"));
	fail_get_mem(DEBUG_SPEC("\x21"));
	fail_get_mem(DEBUG_SPEC("\xB2"));
	fail_get_mem(DEBUG_SPEC("\x72"));
	fail_get_mem(DEBUG_SPEC("\x15"));

	test_get_mem(DEBUG_SPEC("\x10"), 1, 1);
	test_get_mem(DEBUG_SPEC("\x90\x10"), 16, 1);
	test_get_mem(DEBUG_SPEC("\x90\x03" "\x11"), 6, 2);
	test_get_mem(DEBUG_SPEC("\xFF\x03"), (unsigned int)(3 * sizeof(void *)), (unsigned int)(sizeof(void *)));
	test_get_mem(DEBUG_SPEC(
		"\xA0\x02" /* 8x2 = 16 */
			"\x91\x02" /* A: 2x2 = 4 */
			"\x90\x03" /* B: 1x3 = 3 */
			"\x00" /* 7 ~> 8 */
		"\x10" /* C: 1 */
		/* 17 ~> 18 */), 18, 2);
	/* AAaaBbB_AAaaBbB_c_ */
	/* internal fragmentation 3/18 = 17% */

	test_get_mem(DEBUG_SPEC("\x90\x81\x01"), 129, 1);

	test_get_mem_arr(DEBUG_SPEC("\x30\x10"), 2, 3, 1);
	test_get_mem_arr(DEBUG_SPEC("\x3F\x11"), 2, sizeof(size_t) * 2, sizeof(size_t));

	fprintf(stdout, "%lu of %lu tests passed.\n", passed, passed + failed);

	return failed;
}
