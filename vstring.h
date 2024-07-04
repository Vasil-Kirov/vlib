#pragma once
#include <memory.h>

typedef struct String {
	const char *data;
	size_t len;
#if defined(__cplusplus)
	bool operator==(String &other)
#if !defined(VLIB_IMPLEMENTATION)
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

#define STR_LIT(STR) make_string(STR, sizeof(STR) - 1)

String make_string(const char *str, size_t len);


#if defined(VLIB_IMPLEMENTATION)

String make_string(const char *str, size_t len)
{
	String result;
	result.data = str;
	result.len = len;
	return result;
}

#endif

