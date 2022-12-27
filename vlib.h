#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#define _CRT_SECURE_NO_WARNINGS

#define VAlloc  AllocateMemory
#define VFree   free
#define VStrCat strcat
#define VStrCmp VStrCompare
#define VStrLen strlen

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef uint32_t b32;
typedef float    f32;
typedef double   f64;

#if defined(_WIN32)
#include <Windows.h>
#endif

#if !defined __cplusplus
#endif

typedef struct
{
	void *Data;
	int Size;
} entire_file;

#define KB(N) (((unsigned long long)N) << 10)
#define MB(N) (((unsigned long long)N) << 20)
#define GB(N) (((unsigned long long)N) << 30)

#define RET_EMPTY(TYPE) { TYPE __EMPTY_S__ = {0}; return __EMPTY_S__; }

inline void *AllocateVirtualMemory(unsigned long long Size)
{
#if defined(_WIN32)
	return VirtualAlloc(NULL, Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
#error AllocateVirtualMemory not implemented
#endif
}

inline void *AllocateMemory(int Size)
{
	void *Result = malloc(Size);
	memset(Result, 0, Size);
	return Result;
}

bool
VStrCompare(char *str1, char *str2)
{
	if(!str1 || !str2)
		return str1 == str2;

	while(*str1 == *str2)
	{
		if(*str1 == 0 && *str2 == 0) return true;
		
		++str1;
		++str2;
	}
	return false;
}

int
StringToNum(char *String)
{
	char *Scan = String;
	while(*Scan != '\0' && *Scan >= '0' && *Scan <= '9') ++Scan;
	char Save = '\0';
	if(*Scan != '\0')
	{
		Save = *Scan;
		*Scan = '\0';
	}
	int Result = atoi(String);
	*Scan = Save;
	return Result;
}

inline entire_file ReadEntireFile(const char *FileName)
{
	entire_file Result = {0};
	FILE *f;
	errno_t err = fopen_s(&f, FileName, "rb");
	if(err != 0 || f == NULL)
	{
		fprintf(stderr, "Couldn't open file %s! Error %s.\n", FileName, strerror(err));
		RET_EMPTY(entire_file);
	}

	fseek(f, 0, SEEK_END);
	Result.Size = ftell(f);
	Result.Data = VAlloc(Result.Size);
	rewind(f);
	if(fread(Result.Data, Result.Size, 1, f) != 1)
		RET_EMPTY(entire_file);
	fclose(f);
	return Result;
}

inline char *
ChangeFileExtension(const char *FileName, const char *NewExt)
{
	size_t len = VStrLen(FileName);
	size_t new_ext_len = VStrLen(NewExt);
	const char *scanner = FileName + len;
	while(*scanner != '.') scanner--;
	size_t name_len = scanner - FileName;
	char *result = (char *)VAlloc(name_len + 1 + new_ext_len + 1);
	memcpy(result, FileName, name_len);
	*(result + name_len) = '.';
	memcpy(result + name_len + 1, NewExt, new_ext_len);
	return result;
}

inline char *GetAbsolutePath(const char *RelativePath)
{
#if defined(_WIN32)
	char *FullPath = (char *)VAlloc(MAX_PATH);
	if(GetFullPathNameA(RelativePath, MAX_PATH, FullPath, NULL) == 0)
		return NULL;
	return FullPath;
#else
#error GetAbsolutePath not implemented
#endif
}

bool
GetProgramDirectory(char *Out)
{
#if defined(_WIN32)
	return GetCurrentDirectory(MAX_PATH, Out) != 0;
#else
#error GetProgramDirectory not implemented
#endif
}

inline void
FreeFileList(const char **List)
{
	int ListLen = arrlen(List);
	for(int i = 0; i < ListLen; ++i)
	{
		VFree((void *)List[i]);
	}
	arrfree(List);
}

inline char **
GetFileList(const char *Directory)
{
#if defined(_WIN32)
	char *ActualDir = (char *)GetAbsolutePath((char *)Directory);
	VStrCat(ActualDir, "\\");
	auto DirLen = VStrLen(ActualDir);

	char *Search = (char *)VAlloc(VStrLen((char *)Directory) + 4);
	VStrCat(Search, (char *)ActualDir);
	VStrCat(Search, "*");
	char **Result = NULL;


	WIN32_FIND_DATAA Data = {0};
	HANDLE FindHandle = FindFirstFileA(Search, &Data);
	if(FindHandle == INVALID_HANDLE_VALUE)
		return Result;
	{
		VStrCat(ActualDir, Data.cFileName);
		int FullLen = VStrLen(ActualDir);
		char *FullPath = VAlloc(FullLen + 1);
		memcpy(FullPath, ActualDir, FullLen);
		arrpush(Result, FullPath);
	}

	while(true)
	{

		if(FindNextFileA(FindHandle, &Data))
		{
			VStrCat(ActualDir, Data.cFileName);
			int FullLen = VStrLen(ActualDir);
			char *FullPath = VAlloc(FullLen + 1);
			memcpy(FullPath, ActualDir, FullLen);
			arrpush(Result, FullPath);

			ActualDir[DirLen] = 0;
		}
		else
			break;
	}

	FindClose(FindHandle);
	return Result;
#else
#error GetFileList not implemented
#endif
}

bool
StringEndsWith(char *String, char *End)
{
	size_t Len = VStrLen(String);
	size_t EndLen = VStrLen(End);
	String += (Len - EndLen);
	return VStrCmp(String, End);
}

// **************************************************************
// *
// *
// *                       Dynamic Array
// *
// *
// **************************************************************

typedef struct
{
	size_t TypeSize;
	size_t Capacity;
	size_t Used;
	size_t Len;
} arr_header;

#define ARR_HEAD(ARR) (((arr_header *)ARR) - 1)
#define VLibArrCreate(TYPE) (TYPE *)_VLibArrCreate(sizeof(TYPE))
#define VLibArrPush(ARR, ITEM) _VLibArrPush(&ARR, &ITEM)
#define VLibArrLen(ARR) ARR_HEAD(ARR)->Len

#ifndef VLIB_NO_SHORT_NAMES
#define ArrCreate VLibArrCreate
#define ArrPush VLibArrPush
#define ArrLen VLibArrLen;
#endif

void *
_VLibArrCreate(size_t TypeSize)
{
	size_t CurrentlyCommited = TypeSize * 8 + sizeof(arr_header);
	void *Result = VAlloc(CurrentlyCommited);
	arr_header *Header = (arr_header *)Result;
	Header->TypeSize = TypeSize;
	Header->Capacity = CurrentlyCommited - sizeof(arr_header);
	return Header + 1;
}

void
_VLibArrPush(void **Array, void *Item)
{
	void *ArrayPtr = *Array;
	int TypeSize = ARR_HEAD(ArrayPtr)->TypeSize;
	if (ARR_HEAD(ArrayPtr)->Used + TypeSize > ARR_HEAD(ArrayPtr)->Capacity)
	{
		size_t NewSize = ARR_HEAD(ArrayPtr)->Capacity * 1.5;
		void *NewPtr = VAlloc(NewSize + sizeof(arr_header));
		if (NewPtr == 0)
		{
			// @TODO: ADD LOGGER
			// @TODO: ADD LOGGER
			// @TODO: ADD LOGGER
			fprintf(stderr, "Out of memory, got NULL when trying to allocate %zd bytes!", NewSize);
			exit(1);
		}
		arr_header *CopyStart = (arr_header *)ArrayPtr - 1;
		int SizeToCopy = ARR_HEAD(ArrayPtr)->Used + sizeof(arr_header);
		memcpy(NewPtr, CopyStart, SizeToCopy);

		VFree((arr_header *)ArrayPtr - 1);
		*Array = (arr_header *)NewPtr + 1;
		ArrayPtr = *Array;
		ARR_HEAD(ArrayPtr)->Capacity = NewSize;
	}
	void *NewItemLocation = (char *)ArrayPtr + ARR_HEAD(ArrayPtr)->Used;
	memcpy(NewItemLocation, Item, TypeSize);

	ARR_HEAD(ArrayPtr)->Len++;
	ARR_HEAD(ArrayPtr)->Used += TypeSize;
}