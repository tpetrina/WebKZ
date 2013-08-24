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

bool FileReader::Open (const wchar_t* fileName)
{
    FILE* file;
    if (_wfopen_s(&file, fileName, L"rb") != 0)
    {
        return false;
    }

    if (m_buffer != NULL)
    {
        delete[] m_buffer;
    }

    size_t size = (size_t)_filelength(_fileno(file));
    uint8* buffer = new uint8[size];
    if (buffer == NULL)
    {
        fclose(file);
        return false;
    }

    if (_fread_nolock_s(buffer, size, sizeof(uint8), size, file) != size)
    {
        delete[] buffer;
        fclose(file);
        return false;
    }

    fclose(file);
    Initialize(buffer, size);
    return true;
}
