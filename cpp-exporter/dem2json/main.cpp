#include <Windows.h>
#include <iostream>
#include <fstream>

#include "wjson.h"
#include "..\OpenDemo\OpenDemo.h"

using namespace std;

int wmain(int argc, wchar_t *argv[])
{
	if (argc != 3)
	{
		cout << "Usage: dem2json inputfile.dem outputfile.json" << endl;
		return 0;
	}

	// initialize demo
	OpenDemo::Demo demo;

	// load demo
	bool ret = demo.Open(argv[1]);

	// check if demo loaded successfully
	if(ret != true) {
		MessageBox(NULL, L"Loading Demo Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	{
		wofstream pathOutput(argv[2], ios::out);
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

					frame.addProperty(L"position",
						jsonArray<float>::from(
						playerData.position.x,
						playerData.position.z,
						-playerData.position.y));

					frame.addProperty(L"orientation",
					jsonArray<float>::from(
					cameraData.orientation.x,
					cameraData.orientation.z,
					-cameraData.orientation.y));

					frame.addProperty(L"length", frameLength);

					frames.add(frame);
				}
			}

			jo.addProperty(L"frames", frames);

			pathOutput << jo;
		}
	}

	//size_t numberOfFramse = demo.GetNumberOfFrames();
	vec3_t pos;
	OpenDemo::ErrorCode err;
	size_t frames = demo.GetNumberOfFrames();

	return 0;
}