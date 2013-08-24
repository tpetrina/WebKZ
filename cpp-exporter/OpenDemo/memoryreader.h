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

#pragma once

#include "types.h"

#include <cassert>
#include <cstdio>

namespace OpenDemo
{
    class MemoryReader
    {
    public:

        MemoryReader ();
        ~MemoryReader ();

        bool CanRead (size_t count) const;

        const uint8* GetCursor () const;
        size_t GetCursorPosition () const;
        template<class Type>
        Type const* GetData ();
        void const* GetData (size_t size);
        uint32 GetReadPercentage () const;
        size_t GetSize () const;
        const char* GetString (size_t size);

        bool IsEmpty () const;
        bool IsValid () const;
        void Initialize (const void* start, size_t size);

        template<class Type>
        Type const* PeekData ();
        void const* PeekData (size_t size);

        uint8 ReadU8 ();
        bool ReadU8 (uint8* u8);
        uint16 ReadU16 ();
        bool ReadU16 (uint16* u16);
        uint32 ReadU32 ();
        bool ReadU32 (uint32* u32);
        float ReadFloat ();
        bool ReadFloat (float* f);
        bool ReadData (void* buffer, size_t size);

        void Reset ();

        void SetCursorPosition (size_t position);
        bool Skip (size_t count);

    protected:

        const uint8* m_buffer;
        const uint8* m_cursor;
        const uint8* m_end;
        size_t m_size;
    };

    inline MemoryReader::MemoryReader ()
    {
        m_buffer = NULL;
        m_cursor = NULL;
        m_end = NULL;
    }

    inline MemoryReader::~MemoryReader ()
    {
    }

    inline bool MemoryReader::CanRead (size_t count) const
    {
        return ((m_cursor + count) <= m_end);
    }

    inline const uint8* MemoryReader::GetCursor () const
    {
        return m_cursor;
    }

    inline size_t MemoryReader::GetCursorPosition () const
    {
        return (size_t)(m_cursor - m_buffer);
    }

    template<class Type>
    inline Type const* MemoryReader::GetData ()
    {
        return (Type const*)GetData(sizeof(Type));
    }

    inline size_t MemoryReader::GetSize () const
    {
        return m_size;
    }

    inline uint32 MemoryReader::GetReadPercentage () const
    {
        return ((GetCursorPosition() * 100) / GetSize());
    }

    inline bool MemoryReader::IsEmpty () const
    {
        assert(IsValid());
        return ((m_end - m_cursor) == 0);
    }

    inline bool MemoryReader::IsValid () const
    {
        return (m_buffer != NULL);
    }

    template<class Type>
    inline Type const* MemoryReader::PeekData ()
    {
        return (Type const*)PeekData(sizeof(Type));
    }
    
    inline void MemoryReader::Reset ()
    {
        m_buffer = NULL;
        m_cursor = NULL;
        m_end = NULL;
        m_size = 0;
    }

    inline void MemoryReader::SetCursorPosition (size_t position)
    {
        assert(position <= GetSize());
        m_cursor = (m_buffer + position);
    }
}
