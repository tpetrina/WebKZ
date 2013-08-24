#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>


typedef unsigned char byte;
#include "SDK\mathlib.h"
#include "SDK\bspfile.h"

using namespace std;

ostream &operator<<(ostream &os, dheader_t const& header)
{
	cout << "bsp_header" << endl;
	cout << "version - " << header.version << endl;
	for (int i = 0; i < HEADER_LUMPS; ++i)
		cout << header.lumps[i].fileofs << ", " << header.lumps[i].filelen << endl;
	return os;
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cout << "Usage: loader bspfile" << endl;
		return 0;
	}

	cout << "Welcome to BSP loader." << endl << endl;
	ifstream f("nfs_longjump.bsp", ios::in | ios::binary);
	
	dheader_t header;
	f.read(reinterpret_cast<char*>(&header), sizeof(dheader_t));
	
	cout << header << endl;
	LoadBSPFile("nfs_longjump.bsp");
	return 0;
}
