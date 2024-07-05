#pragma once
#include <memory.h>

void *VLibMemDup(void *, size_t);

typedef struct String {
	const char *data;
	size_t len;
#if defined(__cplusplus)
	bool operator==(String &other)
#if defined(VLIB_IMPLEMENTATION)
	{
		if(len != other.len)
			return false;
		return memcmp(data, other.data, len) == 0;
	}
#else
		;
#endif

#endif
} String;

#define STR_LIT(STR) VLibMakeString(STR, sizeof(STR) - 1)

String VLibMakeString(const char *str, size_t len);
void VLibDeleteString(String *str);

#if !defined(VLIB_NO_SHORT_NAMES)
#define MakeString VLibMakeString
#define MemDup VLibMemDup
#endif


#if defined(VLIB_IMPLEMENTATION)

void *MemDup(void *mem, size_t size)
{
	void *result = malloc(size);
	memcpy(result, mem, size);
	return result;
}

String VLibMakeString(const char *str, size_t len)
{
	String result;
	result.data = (const char *)VLibMemDup((void *)str, len);
	result.len = len;
	return result;
}

void VLibDeleteString(String *str)
{
	free((void *)str->data);
	str->data = NULL;
	str->len = 0;
}

#endif

