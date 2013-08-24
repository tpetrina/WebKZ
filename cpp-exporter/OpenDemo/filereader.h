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

#include "memoryreader.h"
#include "types.h"

#include <cassert>
#include <cstdio>

namespace OpenDemo
{
    class FileReader : public MemoryReader
    {
    public:

        FileReader ();
        ~FileReader ();

        size_t GetFileSize () const;
        bool IsLoaded () const;

        bool Open (const wchar_t* fileName);
        void Reset ();
    };

    inline FileReader::FileReader ()
    : MemoryReader()
    {
    }

    inline FileReader::~FileReader ()
    {
        if(m_buffer != NULL)
        {
            delete[] m_buffer;
        }
    }
    
    inline size_t FileReader::GetFileSize () const
    {
        return MemoryReader::GetSize();
    }

    inline bool FileReader::IsLoaded () const
    {
        return MemoryReader::IsValid();
    }

    inline void FileReader::Reset ()
    {
        delete[] m_buffer;
        MemoryReader::Reset();
    }
}
