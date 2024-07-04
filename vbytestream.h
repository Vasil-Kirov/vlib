#pragma once
#include "vtypes.h"
#include "vstring.h"

typedef struct {
	u8 *data;
	size_t at;
	size_t size;
} BinaryReader;

BinaryReader VLibMakeReader(u8 *data, size_t size);

u8  VLibReadUInt8 (BinaryReader *);
u16 VLibReadUInt16(BinaryReader *);
u32 VLibReadUInt32(BinaryReader *);
u64 VLibReadUInt64(BinaryReader *);

i8  VLibReadInt8 (BinaryReader *);
i16 VLibReadInt16(BinaryReader *);
i32 VLibReadInt32(BinaryReader *);
i64 VLibReadInt64(BinaryReader *);

u8 *VLibReadBytes(BinaryReader *reader, size_t size);
String VLibReadStringNullTerminated(BinaryReader *reader); // @NOTE: Null terminated included in string
String VLibReadString(BinaryReader *reader, size_t size);

#if !defined(VLIB_NO_SHORT_NAMES)

#define MakeReader VLibMakeReader
#define ReadUInt8  VLibReadUInt8 
#define ReadUInt16 VLibReadUInt16
#define ReadUInt32 VLibReadUInt32
#define ReadUInt64 VLibReadUInt64
#define ReadInt8  VLibReadInt8 
#define ReadInt16 VLibReadInt16
#define ReadInt32 VLibReadInt32
#define ReadInt64 VLibReadInt64
#define ReadBytes VLibReadBytes
#define ReadStringNullTerminated VLibReadStringNullTerminated
#define ReadString VLibReadString

#endif

#if defined(VLIB_IMPLEMENTATION)

BinaryReader VLibMakeReader(u8 *data, size_t size)
{
	BinaryReader reader;
	reader.data = data;
	reader.size = size;
	reader.at = 0;
	return reader;
}

String VLibReadStringNullTerminated(BinaryReader *reader)
{
	u8 *start = &reader->data[reader->at];
	u8 *scan = start;
	while(*scan) scan++;
	return VLibReadString(reader, scan - start + 1);
}

String VLibReadString(BinaryReader *reader, size_t size)
{
	String result = VLibMakeString((const char *)&reader->data[reader->at], size);
	reader->at += size;
	return result;
}

u8 *VLibReadBytes(BinaryReader *reader, size_t size)
{
	u8 *result = &reader->data[reader->at];
	reader->at += size;
	return result;
}

u8 VLibReadUInt8 (BinaryReader *reader)
{
	u8 result = *VLibReadBytes(reader, 1);
	return result;
}

u16 VLibReadUInt16(BinaryReader *reader)
{
	u16 result = *(u16 *)VLibReadBytes(reader, 2);
	return result;
}

u32 VLibReadUInt32(BinaryReader *reader)
{
	u32 result = *(u32 *)VLibReadBytes(reader, 4);
	return result;
}

u64 VLibReadUInt64(BinaryReader *reader)
{
	u64 result = *(u64 *)VLibReadBytes(reader, 8);
	return result;
}

i8 VLibReadInt8 (BinaryReader *reader)
{
	i8 result = *VLibReadBytes(reader, 1);
	return result;
}

i16 VLibReadInt16(BinaryReader *reader)
{
	i16 result = *(i16 *)VLibReadBytes(reader, 2);
	return result;
}

i32 VLibReadInt32(BinaryReader *reader)
{
	i32 result = *(i32 *)VLibReadBytes(reader, 4);
	return result;
}

i64 VLibReadInt64(BinaryReader *reader)
{
	i64 result = *(i64 *)VLibReadBytes(reader, 8);
	return result;
}


#endif
