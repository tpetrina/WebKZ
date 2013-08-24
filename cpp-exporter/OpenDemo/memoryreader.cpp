/*
	Copyright (c) 2010, Vee
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
		* Redistributions of source code must retain the above copyright
		  notice, this list of conditions and the following disclaimer.
		* Redistributions in binary form must reproduce the above copyright
		  notice, this list of conditions and the following disclaimer in the
		  documentation and/or other materials provided with the distribution.
		* Neither the name eDark nor the
		  names of its contributors may be used to endorse or promote products
		  derived from this software without specific prior written permission.
		* The source code may not be used in any commercial applications.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL eDark BE LIABLE FOR ANY
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "filereader.h"

#include <cassert>
#include <io.h>
#include <memory.h>

using namespace OpenDemo;

void const* MemoryReader::GetData (size_t size)
{
    assert(CanRead(size));
    const void* ptr = m_cursor;
    m_cursor += size;
    return ptr;
}

const char* MemoryReader::GetString (size_t size)
{
    assert(CanRead(size));
    const char* ptr = (const char*)m_cursor;
    m_cursor += size;
    return ptr;
}

void MemoryReader::Initialize (const void* start, size_t size)
{
    m_buffer = (uint8*)start;
    m_cursor = m_buffer;
    m_end = (m_buffer + size);
    m_size = size;
}

void const* MemoryReader::PeekData (size_t size)
{
    assert(CanRead(size));
    return m_cursor;
}

uint8 MemoryReader::ReadU8 ()
{
    assert(CanRead(sizeof(uint8)));
    uint8 u8 = (*m_cursor);
    m_cursor++;
    return u8;
}

bool MemoryReader::ReadU8 (uint8* u8)
{
    if (!CanRead(sizeof(uint8)))
    {
        return false;
    }

    *u8 = (*m_cursor);
    m_cursor++;
    return true;
}

uint16 MemoryReader::ReadU16 ()
{
    assert(CanRead(sizeof(uint16)));
    uint16 u16 = *(uint16*)m_cursor;
    m_cursor += sizeof(uint16);
    return u16;
}

bool MemoryReader::ReadU16 (uint16* u16)
{
    if (!CanRead(sizeof(uint16)))
    {
        return false;
    }

    *u16 = *(uint16*)m_cursor;
    m_cursor += sizeof(uint16);
    return true;
}

uint32 MemoryReader::ReadU32 ()
{
    assert(CanRead(sizeof(uint8)));
    uint32 u32 = *(uint32*)m_cursor;
    m_cursor += sizeof(uint32);
    return u32;
}

bool MemoryReader::ReadU32 (uint32* u32)
{
    if (!CanRead(sizeof(uint32)))
    {
        return false;
    }

    *u32 = *(uint32*)m_cursor;
    m_cursor += sizeof(uint32);
    return true;
}

float MemoryReader::ReadFloat ()
{
    assert(CanRead(sizeof(float)));
    float f = *(float*)m_cursor;
    m_cursor += sizeof(float);
    return f;
}

bool MemoryReader::ReadFloat (float* f)
{
    if (!CanRead(sizeof(float)))
    {
        return false;
    }

    *f = *(float*)m_cursor;
    m_cursor += sizeof(float);
    return true;
}

bool MemoryReader::ReadData (void* buffer, size_t size)
{
    if (!CanRead(size))
    {
        return false;
    }

    if (memcpy_s(buffer, size, m_cursor, size) != 0)
    {
        return false;
    }

    m_cursor += size;
    return true;
}

bool MemoryReader::Skip (size_t count)
{
    if (!CanRead(count))
    {
        return false;
    }

    m_cursor += count;
    return true;
}
