#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include "vtypes.h"

#define ARR_LEN(ARR) (sizeof((ARR)) / sizeof((ARR)[0]))
#ifndef VAlloc
#define VAlloc  AllocateMemory
#endif
#ifndef VFree
#define VFree   free
#endif

#define VStrCat strcat
#define VStrCmp VStrCompare
#define VStrLen strlen

#if defined(_WIN32)

#include <Windows.h>

#define VMAX_PATH MAX_PATH

#else

#include <sys/mman.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <time.h>

timespec StartCounter;
timespec Resolution;
clockid_t CPUClockID;

#define VMAX_PATH PATH_MAX

#endif

#if defined __cplusplus
extern "C" {
#endif

typedef enum
{
	T_ID,
	T_PUNCT,
	T_NUM,
	T_KEYWORD
} token_type;

typedef struct
{
	const char *Str;
	i64 Len;
} token_id;

typedef struct
{
	token_type Type;
	token_id ID;
	i32 Keyword;
} token;

typedef struct
{
	void *Data;
	int Size;
} entire_file;

typedef struct
{
	const char *Name;
	i64 Start;
	i64 End;
} timer_group;

#define KB(N) (((unsigned long long)N) << 10)
#define MB(N) (((unsigned long long)N) << 20)
#define GB(N) (((unsigned long long)N) << 30)

#define RET_EMPTY(TYPE) { TYPE __EMPTY_S__ = {0}; return __EMPTY_S__; }

void *AllocateVirtualMemory(size_t Size);
b32 InitVLib();
void FreeVirtualMemory(void *Memory);
void *AllocateMemory(size_t Size);
void *_VLibArrCreate(size_t TypeSize);
void _VLibArrPush(void **Array, void *Item);
bool VStrCompare(char *str1, char *str2);
int StringToNum(char *String);
entire_file ReadEntireFile(const char *FileName);
char *ChangeFileExtension(const char *FileName, const char *NewExt);
char *GetAbsolutePath(const char *RelativePath);
bool GetActiveDirectory(char *Out);
void FreeFileList(const char **List);
char **GetFileList(const char *DirectoryPath);
bool StringEndsWith(char *String, char *End);
i64 _VLibClock(i64 Factor);
i32 _VLibTokinizeAddKeyword(const char *Keyword, i32 Len);
token *VLibTokinizeString(const char *Str);
i64 VLibClockNs();
i64 VLibClockMs();
i64 VLibClockS();
timer_group VLibStartTimer(const char *Name);
void VLibStopTimer(timer_group *Group);
void VLibCompareTimers(timer_group A, timer_group B);

typedef struct
{
	size_t TypeSize;
	size_t Capacity;
	size_t Used;
	size_t Len;
} arr_header;

#define ARR_HEAD(ARR) (((arr_header *)ARR) - 1)
#define VLibArrCreate(TYPE) (TYPE *)_VLibArrCreate(sizeof(TYPE))
#define VLibArrPush(ARR, ITEM) _VLibArrPush((void **)&ARR, (void *)&ITEM)
#define VLibArrLen(ARR) ARR_HEAD(ARR)->Len
#define VLibArrFree(ARR) VFree(ARR_HEAD(ARR))
#define VLibTokinizeAddKeyword(k) _VLibTokinizeAddKeyword(k, sizeof(k) - 1)

#ifndef VLIB_NO_SHORT_NAMES
#define ArrCreate VLibArrCreate
#define ArrPush   VLibArrPush
#define ArrLen    VLibArrLen
#define ArrFree   VLibArrFree
#define ClockNs VLibClockNs 
#define ClockUs VLibClockUs 
#define ClockMs VLibClockMs 
#define ClockS  VLibClockS 
#define TokinizeAddKeyword VLibTokinizeAddKeyword
#define TokinizeString VLibTokinizeString
#endif

#if defined VLIB_IMPLEMENTATION

i64 StartCounter;
i64 PerfFrequency;

b32 IsVLibInit = false;

// @Note: Only needed for timers
b32 InitVLib()
{
#if defined(_WIN32)
	LARGE_INTEGER PerfFreqLarge;
	LARGE_INTEGER PerfCountLarge;
	if(QueryPerformanceFrequency(&PerfFreqLarge) == 0)
		return false;
	
	if(QueryPerformanceCounter(&PerfCountLarge) == 0)
		return false;

	StartCounter = PerfCountLarge.QuadPart;
	PerfFrequency = PerfFreqLarge.QuadPart;
	IsVLibInit = true;
	return true;
#else

	clock_getcpuclockid(0, &CPUClockID);
	clock_getres(CPUClockID, &Resolution);
	clock_gettime(CPUClockID, &StartCounter);
	IsVLibInit = true;
	return true;

#endif
}

void *AllocateVirtualMemory(size_t Size)
{
#if defined(_WIN32)
	return VirtualAlloc(NULL, Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
	void *Result = mmap(NULL, Size + sizeof(size_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	*(size_t *)Result = Size;
	return (size_t *)Result + 1;
#endif
}

void FreeVirtualMemory(void *Memory)
{
#if defined(_WIN32)
	VirtualFree(Memory, 0, MEM_RELEASE);
#else
	munmap(Memory, ((size_t *)Memory)[-1]);
#endif
}

// @NOTE: Default allocator
void *AllocateMemory(size_t Size)
{
	void *Result = malloc(Size);
	memset(Result, 0, Size);
	return Result;
}

// **************************************************************
// *
// *
// *                       Dynamic Array
// *
// *
// **************************************************************

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

// **************************************************************
// *
// *
// *                       General stuff
// *
// *
// **************************************************************

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

entire_file ReadEntireFile(const char *FileName)
{
	entire_file Result = {0};
	FILE *f;
	f = fopen(FileName, "rb");
	if(f == NULL)
	{
		fprintf(stderr, "Couldn't open file %s! Error %s.\n", FileName, strerror(errno));
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

char *
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

char *GetAbsolutePath(const char *RelativePath)
{
#if defined(_WIN32)
	char *FullPath = (char *)VAlloc(VMAX_PATH);
	if(GetFullPathNameA(RelativePath, VMAX_PATH, FullPath, NULL) == 0)
	{
		VFree(FullPath);
		return NULL;
	}
	return FullPath;
#else
	char *FullPath = (char *)VAlloc(VMAX_PATH);
	if(realpath(RelativePath, FullPath) == NULL)
	{
		printf("%s %s %d\n", RelativePath, strerror(errno), errno);
		VFree(FullPath);
		return NULL;
	}
	return FullPath;
#endif
}

bool
GetActiveDirectory(char *Out)
{
#if defined(_WIN32)
	return GetCurrentDirectoryA(VMAX_PATH, Out) != 0;
#else
	return getcwd(Out, PATH_MAX) != NULL;
#endif
}

void
FreeFileList(const char **List)
{
	int ListLen = VLibArrLen(List);
	for(int i = 0; i < ListLen; ++i)
	{
		VFree((void *)List[i]);
	}
	VLibArrFree(List);
}

char **
GetFileList(const char *DirectoryPath)
{
	char *DirectoryFullPath = (char *)GetAbsolutePath((char *)DirectoryPath);
	char **Result = VLibArrCreate(char *);
	if(DirectoryFullPath == NULL)
		return NULL;
#if defined(_WIN32)
	VStrCat(DirectoryFullPath, "\\");
	size_t DirLen = VStrLen(DirectoryFullPath);

	char *Search = (char *)VAlloc(VStrLen((char *)DirectoryPath) + 4);
	VStrCat(Search, (char *)DirectoryFullPath);
	VStrCat(Search, "*");

	WIN32_FIND_DATAA Data = {0};
	HANDLE FindHandle = FindFirstFileA(Search, &Data);
	if(FindHandle == INVALID_HANDLE_VALUE)
		return Result;
	{
		VStrCat(DirectoryFullPath, Data.cFileName);
		int FullLen = VStrLen(DirectoryFullPath);
		char *FullPath = (char *)VAlloc(FullLen + 1);
		memcpy(FullPath, DirectoryFullPath, FullLen);
		VLibArrPush(Result, FullPath);
	}

	while(true)
	{

		if(FindNextFileA(FindHandle, &Data))
		{
			VStrCat(DirectoryFullPath, Data.cFileName);
			int FullLen = VStrLen(DirectoryFullPath);
			char *FullPath = (char *)VAlloc(FullLen + 1);
			memcpy(FullPath, DirectoryFullPath, FullLen);
			VLibArrPush(Result, FullPath);

			DirectoryFullPath[DirLen] = 0;
		}
		else
			break;
	}

	FindClose(FindHandle);
	return Result;
#else
	VStrCat(DirectoryFullPath, "/");
	auto DirLen = VStrLen(DirectoryFullPath);
	DIR *Dir = opendir(DirectoryFullPath);
	while(true)
	{
		dirent *DirInfo = readdir(Dir);
		if(DirInfo == NULL)
			break;

		VStrCat(DirectoryFullPath, DirInfo->d_name);
		int FullLen = VStrLen(DirectoryFullPath);
		char *FullPath = (char *)VAlloc(FullLen + 1);
		memcpy(FullPath, DirectoryFullPath, FullLen);
		VLibArrPush(Result, FullPath);

		DirectoryFullPath[DirLen] = 0;
	}
	closedir(Dir);
	return Result;
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
// *                       Timers
// *
// *
// **************************************************************

i64
_VLibClock(i64 Factor)
{
	if(!IsVLibInit)
		return 0;
#if defined(_WIN32)
	LARGE_INTEGER PerformanceCounter;
	if(QueryPerformanceCounter(&PerformanceCounter) == 0)
		return 0;
	
	return (PerformanceCounter.QuadPart - StartCounter) * (Factor) / PerfFrequency;
#else

	timespec Counter;
	clock_gettime(CPUClockID, &Counter);
	
	return (Counter.tv_nsec - StartCounter.tv_nsec) * (1000000000 / Factor);
#endif
}

i64
VLibClockNs()
{
	return _VLibClock(1000000000);
}

i64
VLibClockUs()
{
	return _VLibClock(1000000);
}

i64
VLibClockMs()
{
	return _VLibClock(1000);
}

i64
VLibClockS()
{
	return _VLibClock(1);
}

timer_group
VLibStartTimer(const char *Name)
{
	timer_group Group;
	Group.Name  = Name;
	Group.Start = VLibClockUs();
	Group.End   = 0;
	return Group;
}

void
VLibStopTimer(timer_group *Group)
{
	Group->End = VLibClockUs();
}

void
VLibCompareTimers(timer_group A, timer_group B)
{
	i64 ATime = A.End - A.Start;
	i64 BTime = B.End - B.Start;
	timer_group *Winner = NULL;
	timer_group *Loser = NULL;
	if(ATime > BTime)
	{
		Winner = &B;
		Loser = &A;
	}
	else if(BTime > ATime)
	{
		Winner = &A;
		Loser = &B;
	}
	else
	{
		printf("It's draw with both %s and %s taking %lld microseconds", A.Name, B.Name, ATime);
		return;
	}
	i64 WinnerTimer = Winner->End - Winner->Start;
	i64 LoserTimer = Loser->End - Loser->Start;
	printf("%s wins with a time of %lldus\n%s has %lldus, they lost by %lldus", Winner->Name, WinnerTimer, Loser->Name, LoserTimer, LoserTimer - WinnerTimer);
}


token_id VLibKeywords[128] = {};
i32 VLibKeywordCount = 0;
i32 _VLibTokinizeAddKeyword(const char *Keyword, i32 Len)
{
	VLibKeywords[VLibKeywordCount].Str = Keyword;
	VLibKeywords[VLibKeywordCount].Len = Len;
	i32 result = VLibKeywordCount++;
	return result;
}

// Returns an array allocated with ArrCreate
// use ArrFree after you're done
// id's use pointers in the provided str
token *VLibTokinizeString(const char *Str)
{
	token *Tokens = ArrCreate(token);
	while(*Str != 0)
	{
		while(isspace(*Str)) Str++;
		if(*Str == 0)
			break;

		const char *Start = Str;
		if(isalpha(*Str))
		{
			while(isalnum(*Str) || *Str == '_') Str++;
			token Token = {
				.Type = T_ID,
				.ID = {
					.Str = Start,
					.Len = Str - Start,
				},
				.Keyword = -1,
			};
			for(i32 i = 0; i < VLibKeywordCount; ++i)
			{
				if(Token.ID.Len == VLibKeywords[i].Len)
				{
					if(memcmp(Token.ID.Str, VLibKeywords[i].Str, Token.ID.Len) == 0)
					{
						Token.Type = T_KEYWORD;
						Token.Keyword = i;
						break;
					}
				}
			}
			ArrPush(Tokens, Token);
		}
		else if(isdigit(*Str) || (*Str == '-' && isdigit(Str[1])))
		{
			Str++;
			b32 FoundDot = false;
			while(isdigit(*Str) || *Str == '.')
			{
				if(*Str == '.')
				{
					if(FoundDot)
						break;
					FoundDot = true;
				}
				Str++;
			}
			token Token = {
				.Type = T_NUM,
				.ID = {
					.Str = Start,
					.Len = Str - Start,
				},
				.Keyword = -1,
			};
			ArrPush(Tokens, Token);
		}
		else if(ispunct(*Str))
		{
			Str++;
			token Token = {
				.Type = T_PUNCT,
				.ID = {
					.Str = Start,
					.Len = 1,
				},
				.Keyword = -1,
			};
			ArrPush(Tokens, Token);
		}
		else 
			break;

	}
	return Tokens;
}

#endif // Implementation

#if defined __cplusplus
}
#endif

