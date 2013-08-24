/*
Example for using OpenDemo

Copyright (c) 2010, eDark
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

#include <windows.h>
#include <wchar.h>
#include <fstream>
#include "ODExample.h"
#include "OpenDemoMain.h"
#include "OpenDemo.h"
#include "..\bsploader\json.h"

HANDLE hThread;

// sends the loading-status to the main windows
void updateStatus(int Status) 
{
	g_Status = Status;
	InvalidateRect(g_Hwnd, NULL, FALSE);
	PostMessage(g_Hwnd, WM_PAINT, 0, 0);
}

// Example for using the OD_DEMO struct
long WINAPI checkDemo() 
{
	WCHAR fileName[MAX_PATH] = L"";

	// get the filename with the standard windows OPENFILENAME struct
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = L"Demo Files (*.dem)\0*.dem\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = fileName;
	ofn.lpstrTitle = L"Open cstrike demo";
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = L"dem";
	ofn.hwndOwner = g_Hwnd;

	if(GetOpenFileName(&ofn) == TRUE) {
		for(size_t i = wcslen(fileName); i > 0; i--) {
			if(fileName[i] == L'\\') {
				wmemset(g_FileName, 0, MAX_PATH);
				wmemcpy_s(g_FileName, MAX_PATH, &fileName[i + 1], wcslen(fileName) - i - 5);
				break;
			}
		}
	}
	else {
		updateStatus(100);
		return 0;
	}

	// initialize demo
	OpenDemo::Demo demo;

	// load demo
	bool ret = demo.Open(fileName);

	// check if demo loaded successfully
	if(ret != true) {
		MessageBox(NULL, L"Loading Demo Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	{
		ofstream pathOutput("path.json", ios::out);
		jsonObject jo;

		if (!pathOutput)
			MessageBox(NULL, L"Cannot open file for path output", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		else
		{
			jsonArray<jsonObject> frames;
			for (size_t i = 0; i < demo.GetNumberOfFrames(); ++i)
			{
				float frameLength;
				OpenDemo::PlayerData playerData;
				OpenDemo::CameraData cameraData;

				if (demo.GetFrameLength(i, &frameLength) &&
					demo.GetPlayerData(i, &playerData) &&
					demo.GetCameraData(i, &cameraData))
				{
					jsonObject frame;

					frame.addProperty("position",
						jsonArray<float>::from(
						playerData.position.x,
						playerData.position.z,
						-playerData.position.y));

					frame.addProperty("orientation",
					jsonArray<float>::from(
					cameraData.orientation.x,
					cameraData.orientation.z,
					-cameraData.orientation.y));

					frame.addProperty("length", frameLength);

					frames.add(frame);
				}
			}

			jo.addProperty("frames", frames);

			pathOutput << jo;
		}
	}

	//size_t numberOfFramse = demo.GetNumberOfFrames();
	vec3_t pos;
	OpenDemo::ErrorCode err;
	size_t frames = demo.GetNumberOfFrames();

	updateStatus(100);
	return 0;
}