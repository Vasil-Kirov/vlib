#pragma once
#include "vtypes.h"
#include <string.h>
#include <malloc.h>

typedef struct Allocator Allocator;

typedef enum {
	Alloc_DynamicFree = BIT(0),
	Alloc_Realloc     = BIT(1),

} AllocatorSupported;

struct Allocator {
	void *(*alloc)(size_t size, Allocator *);
	void (*free)(void *to_free, Allocator *);
	void (*realloc)(void *old_ptr, size_t new_size, Allocator *);
	u32 flags; // AllocatorSupported
	void *pass_data;
};

struct ArenaAlloc {
	Allocator alloc;
	void *start;
	void *at;
	void *end;
};

#define VLibDefaultArenaMem KB(1)

ArenaAlloc *VLibCreateArenaAllocator(size_t MaxMem);
void VLibArenaFree(ArenaAlloc *arena);
void VLibArenaDestroy(ArenaAlloc *arena);

#if !defined(VLIB_NO_SHORT_NAMES)
#define arena_create VLibCreateArenaAllocator
#define arena_free   VLibArenaFree
#define arena_destroy VLibArenaDestroy
#define arena_default_mem VLibDefaultArenaMem
#endif

#if defined(VLIB_IMPLEMENTATION)

void *VLibArenaAlloc(size_t size, Allocator *alloc)
{
	ArenaAlloc *arena = (ArenaAlloc *)alloc->pass_data;
	if(arena->at >= arena->end)
		return NULL;

	void *res = arena->at;
	arena->at = (u8 *)arena->at + size;
	memset(res, 0, size);

	return res;
}

ArenaAlloc *VLibCreateArenaAllocator(size_t MaxMem)
{
	ArenaAlloc *res = (ArenaAlloc *)malloc(sizeof(ArenaAlloc));
	res->alloc.pass_data = res;
	res->alloc.alloc = VLibArenaAlloc;
	res->start = malloc(MaxMem);
	res->at = res->start;
	res->end = (u8 *)res->start + MaxMem;
	return res;
}

void VLibArenaFree(ArenaAlloc *arena)
{
	memset(arena->start, 0, (u8 *)arena->at - (u8 *)arena->start);
	arena->at = arena->start;
}

void VLibArenaDestroy(ArenaAlloc *arena)
{
	free(arena->start);
	arena->start = arena->at = arena->end = NULL;
	free(arena);
}

#endif

