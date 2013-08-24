/*
	OpenDemo loading function and examples

	Copyright (c) 2010, eDark & Vee
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

#include "OpenDemo.h"
#include <algorithm>

using namespace OpenDemo;

Demo::Demo()
{
	m_errorInfo.code = OD_ERROR_NO_FILE;
    m_errorInfo.offset = 0;
    m_errorInfo.macroId = 0xFF;
}

Demo::~Demo()
{
}

bool Demo::Open(wchar_t *wcFileName)
{
    m_segments.clear();
    m_f.Reset();
	m_wsFileName = wcFileName;

	if(!m_f.Open(m_wsFileName.c_str()))
	{
		SetLastError(OD_ERROR_NO_FILE);
		return false;
	}

	if(m_f.CanRead(sizeof(DemoHeader)) == false)
	{			
		SetLastError(OD_ERROR_NO_DEMO);
		return false;
	}

	// get m_header

	m_header = m_f.GetData<DemoHeader> ();

	// check if everything is ok for loading

	if(strcmp(m_header->magic, "HLDEMO") != 0)
	{
		SetLastError(OD_ERROR_NO_DEMO);
		return false;
	}

	if(strcmp(m_header->game_dll, "cstrike") != 0)
	{
		SetLastError(OD_ERROR_NO_DEMO);
		return false;
	}

	if(m_header->demo_protocol < DEMOPROTOCOL_MIN || m_header->network_protocol < NETWORKPROTOCOL_MIN)
	{
		SetLastError(OD_ERROR_OLD_PROTOCOL);
		return false;
	}

	// get DemoDirectoryEntries

	m_f.SetCursorPosition(m_header->directory_offset);

	uint32 dir_count;

	if(m_f.ReadU32(&dir_count) == false)
	{
		SetLastError(OD_ERROR_CORRUPT_FILE);
		return false;
	}

	if(dir_count == 0 || m_f.CanRead((dir_count) * sizeof(DemoDirectoryEntry)) == false)
	{
		SetLastError(OD_ERROR_CORRUPT_FILE);
		return false;
	}

	m_segments.resize(dir_count);

	vector<DemoSegment>::iterator it;
	size_t nextDirectoryEntry;

	for ( it = m_segments.begin() ; it < m_segments.end(); ++it )
	{
        DemoSegment& segment = (*it);
		const DemoDirectoryEntry* dirEntry = m_f.GetData<DemoDirectoryEntry> ();
        segment.dir_entry = dirEntry;
		nextDirectoryEntry = m_f.GetCursorPosition();

		// load segment

		m_f.SetCursorPosition(dirEntry->offset);

		if(m_f.CanRead(dirEntry->length) == false)
		{
			SetLastError(OD_ERROR_CORRUPT_FILE);
			return false;
		}

		// preprocessing

		size_t numOfMacros = 0;
        MemoryReader segReader;
        segReader.Initialize(m_f.GetCursor(), dirEntry->length);

		while(true)
		{
			const DemoMacroHeader* header = segReader.PeekData<DemoMacroHeader> ();

			if(header->type == MACROID_UNUSED)
			{
				segReader.Skip(sizeof(DemoMacroHeader));
				continue;
			}

			++numOfMacros;

            switch(header->type)
            {
                case MACROID_LOADDATA:
                    segReader.Skip(sizeof(DemoMacro_GameData) + ((DemoMacro_GameData *)header)->chunklength);
                    break;
                case MACROID_GAMEDATA:
                    segReader.Skip(sizeof(DemoMacro_GameData) + ((DemoMacro_GameData *)header)->chunklength);
                    break;
                case MACROID_CLIENTCOMMAND:
                    segReader.Skip(sizeof(DemoMacro_ClientCommand));
                    break;
                case MACROID_CLIENTDATA:
                    segReader.Skip(sizeof(DemoMacro_ClientData));
                    break;
                case MACROID_ENDOFSEGMENT:
                    segReader.Skip(sizeof(DemoMacro_LastInSegment));
                    break;
                case MACROID_EVENT:
                    segReader.Skip(sizeof(DemoMacro_Event));
                    break;
                case MACROID_WEAPONANIM:
                    segReader.Skip(sizeof(DemoMacro_WeaponAnim));
                    break;
                case MACROID_PLAYSOUND:
                    segReader.Skip(sizeof(DemoMacro_PlaySound) + ((DemoMacro_PlaySound *)header)->sound_name_length + sizeof(DemoMacro_PlaySound_2));
                    break;
                case MACROID_DEMOBUFFER:
                    segReader.Skip(sizeof(DemoMacro_DemoBuffer) + ((DemoMacro_DemoBuffer *)header)->chunklength);
                    break;
                default:
                    SetErrorInfo(header);
                    return false;
            }

			if(!segReader.CanRead(sizeof(DemoHeader)))
            {
				break;
            }
		}
            
		// getting macro positions

		segReader.SetCursorPosition(0);

		segment.frames.resize(dirEntry->frames, NULL);
		segment.buttonFlag.resize(dirEntry->frames, BF_NULL);
		segment.macroAllocator.Initialize(numOfMacros);

        DemoMacro* macro = NULL;
		uint32 frameButtonFlags = BF_NULL;
		uint32 frameScrollFlags = BF_NULL;

        while(true)
        {
            const DemoMacroHeader* header = segReader.PeekData<DemoMacroHeader> ();

            if(header->type == MACROID_UNUSED)
            {
                segReader.Skip(sizeof(DemoMacroHeader));
                continue;
            }
                
            if (macro != NULL)
            {
                DemoMacro* next = segment.macroAllocator.New();
                macro->nextMacro = next;
                macro = next;
            }
            else
            {
                macro = segment.macroAllocator.New();
                segment.firstMacro = macro;
            }

            macro->header = header;

            if(header->frame < dirEntry->frames && segment.frames[header->frame] == NULL)
            {
                segment.frames[header->frame] = macro;
				if (header->frame > 0)
				{
					segment.buttonFlag[header->frame - 1] = frameButtonFlags;
				}
				frameButtonFlags &= ~BF_SCROLLS;
				segment.buttonFlag[header->frame] = frameButtonFlags;
				frameScrollFlags = BF_NULL;
            }

            switch(header->type)
            {
                case MACROID_LOADDATA:
                    segReader.Skip(sizeof(DemoMacro_GameData) + ((DemoMacro_GameData *)header)->chunklength);
                    break;
                case MACROID_GAMEDATA:
                    segReader.Skip(sizeof(DemoMacro_GameData) + ((DemoMacro_GameData *)header)->chunklength);
                    break;
                case MACROID_CLIENTCOMMAND:
					if (strcmp(((DemoMacro_ClientCommand *)header)->cmd, "+jump") == 0)
					{
						frameButtonFlags |= BF_JUMP;
						frameScrollFlags |= BF_JUMP;
					}
					else if (strcmp(((DemoMacro_ClientCommand *)header)->cmd, "-jump") == 0)
					{
						frameButtonFlags &= ~BF_JUMP;
						if (frameScrollFlags & BF_JUMP)
						{
							frameButtonFlags |= BF_JUMP_SCROLL;
						}
					}
					else if (strcmp(((DemoMacro_ClientCommand *)header)->cmd, "+duck") == 0)
					{
						frameButtonFlags |= BF_DUCK;
						frameScrollFlags |= BF_DUCK;
					}
					else if (strcmp(((DemoMacro_ClientCommand *)header)->cmd, "-duck") == 0)
					{
						frameButtonFlags &= ~BF_DUCK;
						if (frameScrollFlags & BF_DUCK)
						{
							frameButtonFlags |= BF_DUCK_SCROLL;
						}
					}
					else if (strcmp(((DemoMacro_ClientCommand *)header)->cmd, "+forward") == 0)
					{
						frameButtonFlags |= BF_FORWARD;
					}
					else if (strcmp(((DemoMacro_ClientCommand *)header)->cmd, "-forward") == 0)
					{
						frameButtonFlags &= ~BF_FORWARD;
					}
					else if (strcmp(((DemoMacro_ClientCommand *)header)->cmd, "+back") == 0)
					{
						frameButtonFlags |= BF_BACK;
					}
					else if (strcmp(((DemoMacro_ClientCommand *)header)->cmd, "-back") == 0)
					{
						frameButtonFlags &= ~BF_BACK;
					}
					else if (strcmp(((DemoMacro_ClientCommand *)header)->cmd, "+moveleft") == 0)
					{
						frameButtonFlags |= BF_MOVELEFT;
					}
					else if (strcmp(((DemoMacro_ClientCommand *)header)->cmd, "-moveleft") == 0)
					{
						frameButtonFlags &= ~BF_MOVELEFT;
					}
					else if (strcmp(((DemoMacro_ClientCommand *)header)->cmd, "+moveright") == 0)
					{
						frameButtonFlags |= BF_MOVERIGHT;
					}
					else if (strcmp(((DemoMacro_ClientCommand *)header)->cmd, "-moveright") == 0)
					{
						frameButtonFlags &= ~BF_MOVERIGHT;
					}
                    segReader.Skip(sizeof(DemoMacro_ClientCommand));
                    break;
                case MACROID_CLIENTDATA:
                    segReader.Skip(sizeof(DemoMacro_ClientData));
                    break;
                case MACROID_ENDOFSEGMENT:
                    segReader.Skip(sizeof(DemoMacro_LastInSegment));
                    break;
                case MACROID_EVENT:
                    segReader.Skip(sizeof(DemoMacro_Event));
                    break;
                case MACROID_WEAPONANIM:
                    segReader.Skip(sizeof(DemoMacro_WeaponAnim));
                    break;
                case MACROID_PLAYSOUND:
                    segReader.Skip(sizeof(DemoMacro_PlaySound) + ((DemoMacro_PlaySound *)header)->sound_name_length + sizeof(DemoMacro_PlaySound_2));
                    break;
                case MACROID_DEMOBUFFER:
                    segReader.Skip(sizeof(DemoMacro_DemoBuffer) + ((DemoMacro_DemoBuffer *)header)->chunklength);
                    break;
                default:
                    segment.macroAllocator.Clear();
                    SetLastError(OD_ERROR_LOADING);
                    return false;
            }

			if(!segReader.CanRead(sizeof(DemoHeader)))
			{
                macro->nextMacro = NULL;
                break;
            }
		}

		m_f.SetCursorPosition(nextDirectoryEntry);
	}

	m_errorInfo.code = OD_SUCCESS;

	return true;
}

void Demo::SetErrorInfo (const DemoMacroHeader* header)
{
    m_errorInfo.code = OD_ERROR_LOADING;
    m_errorInfo.offset = (m_f.GetCursorPosition() - sizeof(DemoMacroHeader));
    m_errorInfo.macroId = header->type;
}

bool Demo::GetFirstMacroOfFrame(size_t frame, DemoMacro** macro)
{
	if (frame < m_segments[1].frames.size())
	{
		*macro = m_segments[1].frames[frame];
		return true;
	}

	SetLastError(OD_ERROR_NO_FRAME);
	return false;
}

bool Demo::GetMacroOfFrame(size_t frame, DemoMacro** macro, MacroID macroId)
{
	if(!GetFirstMacroOfFrame(frame, macro))
	{
		return false;
	}

	while (*macro != NULL && (*macro)->header->frame == frame)
	{
		if ((*macro)->header->type == macroId)
		{
			return true;
		}

		*macro = (*macro)->nextMacro;
	}

	SetLastError(OD_ERROR_NO_MACRO);
	return false;
}

bool Demo::GetMacroOfFrame(size_t frame, DemoMacro** macro, MacroID macroId1, MacroID macroId2)
{
	if(!GetFirstMacroOfFrame(frame, macro))
	{
		return false;
	}

	while (*macro != NULL && (*macro)->header->frame == frame)
	{
		if ((*macro)->header->type == macroId1 || (*macro)->header->type == macroId2)
		{
			return true;
		}

		*macro = (*macro)->nextMacro;
	}

	SetLastError(OD_ERROR_NO_MACRO);
	return false;
}

bool Demo::GetFrameTime(size_t frame, float* frameTime)
{
	macro = NULL;
	if(!GetFirstMacroOfFrame(frame, &macro))
	{
		return false;
	}

	*frameTime = macro->header->time;

	return true;
}

bool Demo::GetPlayerPosition(size_t frame, vec3_t* playerPos)
{
	macro = NULL;
	if (!GetMacroOfFrame(frame, &macro, MACROID_GAMEDATA, MACROID_CLIENTDATA))
	{
		return false;
	}

	*playerPos = macro->header->type == MACROID_GAMEDATA ? ((DemoMacro_GameData*)macro->header)->playerposition : ((DemoMacro_ClientData*)macro->header)->playerposition;

	return true;
}

bool Demo::GetPlayerVelocity(size_t frame, vec3_t* playerVel)
{
	macro = NULL;
	if (!GetMacroOfFrame(frame, &macro, MACROID_GAMEDATA))
	{
		return false;
	}

	*playerVel = ((DemoMacro_GameData*)macro->header)->playervelocity;

	return true;
}

bool Demo::GetPlayerOrientation(size_t frame, vec3_t* playerRot)
{
	macro = NULL;
	if (!GetMacroOfFrame(frame, &macro, MACROID_CLIENTDATA))
	{
		return false;
	}

	*playerRot = ((DemoMacro_ClientData*)macro->header)->playerorientation;

	return true;
}

bool Demo::GetPlayerData(size_t frame, PlayerData* playerData)
{
	macro = NULL;
	if (!GetMacroOfFrame(frame, &macro, MACROID_GAMEDATA))
	{
		return false;
	}

	*playerData->velocity = ((DemoMacro_GameData*)macro->header)->playervelocity;

	macro = NULL;
	if (!GetMacroOfFrame(frame, &macro, MACROID_CLIENTDATA))
	{
		return false;
	}

	*playerData->position = ((DemoMacro_ClientData*)macro->header)->playerposition;
	*playerData->orientation = ((DemoMacro_ClientData*)macro->header)->playerorientation;

	return true;
}

bool Demo::GetCameraPosition(size_t frame, vec3_t* cameraPos)
{
	macro = NULL;
	if (!GetMacroOfFrame(frame, &macro, MACROID_GAMEDATA))
	{
		return false;
	}

	*cameraPos = ((DemoMacro_GameData*)macro->header)->camera.position;

	return true;
}

bool Demo::GetCameraOrientation(size_t frame, vec3_t* cameraRot)
{
	macro = NULL;
	if (!GetMacroOfFrame(frame, &macro, MACROID_GAMEDATA))
	{
		return false;
	}

	*cameraRot = ((DemoMacro_GameData*)macro->header)->camera.orientation;

	return true;
}

bool Demo::GetCameraData(size_t frame, CameraData* camera)
{
	macro = NULL;
	if (!GetMacroOfFrame(frame, &macro, MACROID_GAMEDATA))
	{
		return false;
	}

	*camera = ((DemoMacro_GameData*)macro->header)->camera;

	return true;
}

bool Demo::GetFrameLength(size_t frame, float* frameLength)
{
	macro = NULL;
	if (!GetMacroOfFrame(frame, &macro, MACROID_GAMEDATA))
	{
		return false;
	}

	*frameLength = ((DemoMacro_GameData*)macro->header)->framelength;

	return true;
}

bool Demo::GetServerTime(size_t frame, float* serverTime)
{
	macro = NULL;
	if (!GetMacroOfFrame(frame, &macro, MACROID_GAMEDATA))
	{
		return false;
	}

	*serverTime = ((DemoMacro_GameData*)macro->header)->servertime;

	return true;
}

bool Demo::GetGroundFlag(size_t frame, uint32* groundFlag)
{
	macro = NULL;
	if (!GetMacroOfFrame(frame, &macro, MACROID_GAMEDATA))
	{
		return false;
	}

	*groundFlag = ((DemoMacro_GameData*)macro->header)->groundflag;

	return true;
}

bool Demo::GetScreenResolution(size_t frame, short* screen_width, short* screen_height, short* screen_depth)
{
	macro = NULL;
	if (!GetMacroOfFrame(frame, &macro, MACROID_GAMEDATA))
	{
		return false;
	}

	*screen_width = ((DemoMacro_GameData*)macro->header)->screen_width;
	*screen_height = ((DemoMacro_GameData*)macro->header)->screen_height;
	*screen_depth = ((DemoMacro_GameData*)macro->header)->screen_depth;

	return true;
}

bool Demo::GetEngineFrameLength(size_t frame, short* engineFrameLength)
{
	macro = NULL;
	if (!GetMacroOfFrame(frame, &macro, MACROID_GAMEDATA))
	{
		return false;
	}

	*engineFrameLength = ((DemoMacro_GameData*)macro->header)->engine_framelength;

	return true;
}

bool Demo::GetForwardmove(size_t frame, float* forwardmove)
{
	macro = NULL;
	if (!GetMacroOfFrame(frame, &macro, MACROID_GAMEDATA))
	{
		return false;
	}

	*forwardmove = ((DemoMacro_GameData*)macro->header)->forwardmove;

	return true;
}

bool Demo::GetSidemove(size_t frame, float* sidemove)
{
	macro = NULL;
	if (!GetMacroOfFrame(frame, &macro, MACROID_GAMEDATA))
	{
		return false;
	}

	*sidemove = ((DemoMacro_GameData*)macro->header)->sidemove;

	return true;
}

bool Demo::GetSettings(size_t frame, Settings* settings)
{
	macro = NULL;
	if (!GetMacroOfFrame(frame, &macro, MACROID_GAMEDATA))
	{
		return false;
	}

	*settings = ((DemoMacro_GameData*)macro->header)->settings;

	return true;
}

bool Demo::GetSkyName(size_t frame, char* sky_name)
{
	macro = NULL;
	if (!GetMacroOfFrame(frame, &macro, MACROID_GAMEDATA))
	{
		return false;
	}

	memcpy_s(sky_name, 40, ((DemoMacro_GameData*)macro->header)->sky_name, 40);

	return true;
}

bool GetCommands(size_t frame, char** commands, size_t numCommands); // [64]


bool Demo::GetNumberOfCommands(size_t frame, size_t* numCommands)
{
	macro = NULL;
	if(!GetFirstMacroOfFrame(frame, &macro))
	{
		return false;
	}

	*numCommands = 0;

	while (macro != NULL && macro->header->frame == frame)
	{
		if (macro->header->type == MACROID_CLIENTCOMMAND)
		{
			(*numCommands)++;
		}

		macro = macro->nextMacro;
	}

	return true;
}

bool Demo::GetCommands(size_t frame, char** commands, size_t numCommands)
{
	macro = NULL;
	if(!GetFirstMacroOfFrame(frame, &macro))
	{
		return false;
	}

	size_t c = 0;

	while (macro != NULL && macro->header->frame == frame && c < numCommands)
	{
		if (macro->header->type == MACROID_CLIENTCOMMAND)
		{
			memcpy_s(commands[c], 64, ((DemoMacro_ClientCommand*)macro->header)->cmd, 64);
			c++;
		}

		macro = macro->nextMacro;
	}

	return true;
}

bool Demo::GetButtonFlags(size_t frame, uint32* flags)
{
	if (frame < m_segments[1].frames.size())
	{
		*flags = m_segments[1].buttonFlag[frame];
		return true;
	}

	SetLastError(OD_ERROR_NO_FRAME);
	return false;
}

