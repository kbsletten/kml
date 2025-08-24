#include <limits.h>
#include <stddef.h>

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

#define VOID_PTR(x) ((void *)(x))
#define SZ_ONE ((size_t)1)
#define LMASK(x) ((SZ_ONE << x) - 1)

#ifndef PTR_BITS
#ifdef INT_64
#define PTR_BITS 64
#else
#define PTR_BITS 32
#endif
#endif

#define PTR_SIZE (PTR_BITS / 8)

#if PTR_BITS == 64

#ifndef INTERIOR_BITS
#define INTERIOR_BITS 16
#endif

#define BASE_BITS (PTR_BITS - INTERIOR_BITS)

typedef U64 ptr_t;

typedef struct {
	ptr_t interior_ptr;
} interior_ptr_t;

#define BASE_PTR(p) VOID_PTR((p).interior_ptr & LMASK(BASE_BITS))
#define MEM_PTR(p) VOID_PTR( \
	((p).interior_ptr & LMASK(BASE_BITS)) \
	+ (((p).interior_ptr & ~LMASK(BASE_BITS)) >> BASE_BITS) \
)

#elif PTR_BITS == 32

typedef U32 ptr_t;

typedef struct {
	ptr_t base_ptr;
	ptr_t offset;
} interior_ptr_t;

#define BASE_PTR(p) VOID_PTR((p).base_ptr)
#define MEM_PTR(p) VOID_PTR((p).base_ptr + (p).offset)

#endif