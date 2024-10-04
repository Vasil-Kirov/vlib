#pragma once
#include "vlib.h"
#include <memory.h>

typedef struct StringBuilder StringBuilder;
typedef struct String String;

void *VLibMemDup(void *, size_t);
bool VLibStringsMatch(String a, String b);
struct String VLibMakeString(const char *str, size_t len);
struct String VLibBuilderToString(StringBuilder *b);
void VLibPushBuilder(StringBuilder *b, String str);
void VLibFreeBuilderMemory(StringBuilder *b);

struct StringBuilder {
	char *VLibArray;
};

struct String {
	const char *data;
	size_t len;
#if defined(__cplusplus)
	bool operator==(String &other)
#if defined(VLIB_IMPLEMENTATION)
	{
		return VLibStringsMatch(*this, other);
	}
#else
		;
#endif

#endif
};

#define STR_LIT(STR) VLibMakeString(STR, sizeof(STR) - 1)

#if !defined(VLIB_NO_SHORT_NAMES)
#define make_string VLibMakeString
#define strings_match VLibStringsMatch
#define mem_dup VLibMemDup
#define builder_to_string VLibBuilderToString
#define push_builder VLibPushBuilder
#define free_builder VLibFreeBuilderMemory
#endif


#if defined(VLIB_IMPLEMENTATION)

void *MemDup(void *mem, size_t size)
{
	void *result = malloc(size);
	memcpy(result, mem, size);
	return result;
}

bool VLibStringsMatch(String a, String b)
{
	if(a.len != b.len)
		return false;
	return memcmp(a.data, b.data, a.len) == 0;
}

String VLibMakeString(const char *str, size_t len)
{
	String result;
	result.data = str;
	result.len = len;
	return result;
}

String VLibBuilderToString(StringBuilder *b)
{
	char Null = '\0';
	VLibArrPush(b->VLibArray, Null);
	return VLibMakeString(b->VLibArray, VLibArrLen(b->VLibArray)-1);
}

void VLibPushBuilder(StringBuilder *b, String str)
{
	if(b->VLibArray == NULL)
		b->VLibArray = VLibArrCreate(char);
	for(int i = 0; i < str.len; ++i)
	{
		VLibArrPush(b->VLibArray, str.data[i]);
	}
}

void VLibFreeBuilderMemory(StringBuilder *b)
{
	if(b->VLibArray != NULL)
		VLibArrFree(b->VLibArray);

}

#endif

