#include <iostream>
#include <list>
#include <string>
#include <fstream>
#include <cstdint>
#include <sstream>
#include <algorithm>
#include <memory>
#include <vector>

#include "json.h"

typedef unsigned char byte;

extern "C"
{
#include "..\\bspsdk\\SDK\\mathlib.h"
#include "..\\bspsdk\\SDK\\bspfile.h"
};

template <typename T, typename Callable>
bool contains(T container, Callable callable)
{
	return find_if(begin(container), end(container), callable) != end(container);
}

#pragma comment(lib, "..\\Debug\\bspsdk.lib")

using namespace std;

ostream &operator<<(ostream &os, dheader_t const& header)
{
	cout << "bsp_header" << endl;
	cout << "version - " << header.version << endl;
	for (int i = 0; i < HEADER_LUMPS; ++i)
		cout << header.lumps[i].fileofs << ", " << header.lumps[i].filelen << endl;
	return os;
}

#define PROPERTY(x) cout << #x << " = " << x << endl;

void test()
{
	ofstream jsonOut("test.json", ios::out);

	jsonObject obj("root");
	jsonArray<jsonObject> children;
	jsonObject child;
	jsonArray<float> arr;
	arr.add(1.0f);
	child.addProperty("arr", arr);
	children.add(child);
	obj.addProperty("children", children);

	jsonOut << obj;
}

struct hello
{
	int i;

	hello(int i)
		: i(i)
	{}

	~hello()
	{
		cout << "!!! destroyed" << endl;
	}

	friend ostream &operator <<(ostream &os, hello const &h)
	{
		os << "hello: " << h.i;
		return os;
	}
};

int main(int argc, char *argv[])
{	
#if 0
	test();
	return 0;
#endif

	cout << "Welcome to BSP loader." << endl << endl;
	if (argc < 3)
	{
		cout << "No map specified" << endl;
		return 0;
	}

	ifstream f(argv[1], ios::in | ios::binary);

	dheader_t header;
	f.read(reinterpret_cast<char*>(&header), sizeof(dheader_t));

	cout << header << endl;
	LoadBSPFile(argv[1]);

	PROPERTY(nummodels);
	PROPERTY(visdatasize);
	PROPERTY(lightdatasize);
	PROPERTY(texdatasize);
	PROPERTY(entdatasize);
	PROPERTY(numleafs);
	PROPERTY(numplanes);
	PROPERTY(numvertexes);
	PROPERTY(numnodes);
	PROPERTY(numtexinfo);
	PROPERTY(numfaces);
	PROPERTY(numclipnodes);
	PROPERTY(numedges);
	PROPERTY(nummarksurfaces);
	PROPERTY(numsurfedges);

	ofstream jsonOut(argv[2], ios::out);

	jsonObject jo;
	jo.addProperty("version", header.version);

	jo.addProperty("numvertexes", numvertexes);
	jsonArray<jsonArray<float>> vertices;
	for (long i = 0; i < numvertexes; ++i)
	{
		//vertices.add(dvertexes[i].point[0]);
		//vertices.add(dvertexes[i].point[1]);
		//vertices.add(dvertexes[i].point[2]);
		vertices.add(jsonArray<float>::from(
			dvertexes[i].point[0],
			dvertexes[i].point[1],
			dvertexes[i].point[2]
		));
	}
	jo.addProperty("vertices", vertices);

	//jo.addProperty("numedges", numedges);
	//jsonArray<int> edges;
	//for (long i = 0; i < numedges; ++i)
	//{
	//	edges.add(dedges[i].v[0]);
	//	edges.add(dedges[i].v[1]);
	//}
	//jo.addProperty("edges", edges);

	// parse textuers
	//auto textures = (dmiptexlump_t*)dtexdata;
	//auto name = ((miptex_t*)&dtexdata[textures[0].dataofs[0]])->name;

	uint32_t numTextures = *((uint32_t*)dtexdata);
	int32_t *textureOffsets = (int32_t*)(dtexdata + sizeof(uint32_t));
	vector<miptex_t*> textures;
	for (uint32_t i = 0; i < numTextures; ++i)
	{
		miptex_t *tex = (miptex_t*)(dtexdata + textureOffsets[i]);
		textures.push_back(tex);
		cout << "texture: '" << tex->name << "'" << endl;
	}

	jsonArray<jsonObject> faces;
	cout << endl << "Faces: " << endl;

	for (long i = 0; i < numfaces; ++i)
	{
		vector<int> vs;
		for (long j = dfaces[i].firstedge; j < (dfaces[i].firstedge + dfaces[i].numedges); ++j)
		{
			int edgeIndex = abs(dsurfedges[j]);
			auto v0 = dedges[edgeIndex].v[0];
			auto v1 = dedges[edgeIndex].v[1];

			if (v0 == v1)
				cout << "null edge" << endl;

			if (dsurfedges[j] < 0) swap(v0, v1);
			if (!vs.empty() && vs.back() != v0)
				__asm int 3;

			if (vs.empty())
				vs.push_back(v0);
			vs.push_back(v1);
		}

		if (!contains(vs, [](int i){return i != 0; }))
		{
			cout << "invalid face, all vertices are 0" << endl;
			continue;
		}
		else if (vs.back() != vs.front())
		{
			cout << "invalid face, doesn't wrap around" << endl;
			continue;
		}
		else if (vs.size() < 3)
		{
			cout << "invalid face, not enough vertices" << endl;
			continue;
		}

		// last == first (triangle fan)
		vs.pop_back();

		// texture coordinates
		vector<float> ts;

		auto tinfo = texinfo[dfaces[i].texinfo];
		for (int j = 0; j < vs.size(); ++j)
		{
			float x = dvertexes[vs[j]].point[0];
			float y = dvertexes[vs[j]].point[1];
			float z = dvertexes[vs[j]].point[2];
			float *u_axis = tinfo.vecs[0];
			float *v_axis = tinfo.vecs[1];

			float u = x * u_axis[0] + y * u_axis[1] + z * u_axis[2] + u_axis[3];
			float v = x * v_axis[0] + y * v_axis[1] + z * v_axis[2] + v_axis[3];

			u /= textures[tinfo.miptex]->width;
			v /= textures[tinfo.miptex]->height;

			ts.push_back(u);
			ts.push_back(v);
		}

		auto plane = dplanes[dfaces[i].planenum];
		for (int j = 1; j < vs.size() - 1; ++j)
		{
			jsonObject face;

			jsonArray<float> faceVertices;
			faceVertices.add(vs[0], vs[j], vs[j + 1]);
			face.addProperty("vertices", faceVertices);

			face.addProperty("side", dfaces[i].side);

			jsonArray<float> faceNormal;
			faceNormal.add(plane.normal[0], plane.normal[1], plane.normal[2]);
			face.addProperty("normal", faceNormal);

			jsonArray<float> faceUVs;
			faceUVs.add(ts[0], ts[1]);
			faceUVs.add(ts[j * 2], ts[j * 2 + 1]);
			faceUVs.add(ts[(j + 1) * 2], ts[(j + 1) * 2 + 1]);
			face.addProperty("faceUVs", faceUVs);

			auto index = dfaces[i].texinfo;
			index = texinfo[index].miptex;
			if (textures.size() > index && index >= 0)
				face.addProperty("texture", string((const char *)textures[index]->name));
			else
				face.addProperty("texture", string("unknown"));

			faces.add(face);
		}
	}

	jo.addProperty("numfaces", faces.values.size());
	jo.addProperty("faces", faces);

	jsonOut << jo;

	jsonOut.close();
	return 0;
}
