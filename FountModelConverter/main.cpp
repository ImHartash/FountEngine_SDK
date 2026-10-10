#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include "headers/fntmdl_header.hpp"
#include "headers/fntmdl_vertex.hpp"

struct Vector2_t {
	float u, v;
	Vector2_t(float u_ = 0.f, float v_ = 0.f) : u(u_), v(v_) {}
};

struct Vector3_t {
	float x, y, z;
	Vector3_t(float x_ = 0.f, float y_ = 0.f, float z_ = 0.f) : x(x_), y(y_), z(z_) {}
};

std::vector<std::string> SplitString(const std::string& strSource) {
	std::vector<std::string> vecTokens;
	std::string strToken;

	for (char c : strSource) {
		if (c == ' ' || c == '\t') {
			vecTokens.push_back(strToken);
			strToken.clear();
		}
		else {
			strToken += c;
		}
	}

	if (!strToken.empty())
		vecTokens.push_back(strToken);

	return vecTokens;
}

bool ConvertObjectToFntmdl(const std::string& strInputPath, 
	const std::string& strOutputPath, const std::string& strMaterialPrefix,
	const std::unordered_map<std::string, std::string>* pMapping) 
{
	std::ifstream fin(strInputPath);
	if (!fin.is_open()) {
		std::cout << "Couldn't open source file " << strInputPath << std::endl;
		return false;
	}

	std::vector<Vector3_t> vecPositions;
	std::vector<Vector2_t> vecTexcoords;
	std::vector<Vector3_t> vecNormals;

	struct FaceVertex_t {
		int p;
		int t;
		int n;
	};
	struct Face_t {
		std::vector<FaceVertex_t> vecVertices;
		std::string strMaterial;
	};
	std::vector<Face_t> vecFaces;

	std::string strCurrentMaterial;	
	std::string strLine;
	int nLineNum = 0;
	while (std::getline(fin, strLine)) {
		nLineNum++;
		if (strLine.empty() || strLine[0] == '#') // Empty string or just a comment
			continue;

		std::vector<std::string> vecTokens = SplitString(strLine);
		if (vecTokens.empty())
			continue;

		const std::string& strCommand = vecTokens[0];
		if (strCommand == "v") { // Vertex
			if (vecTokens.size() < 4) {
				std::cerr << "[WARN] Malfored 'v' at line " << nLineNum << std::endl;
				continue;
			}

			float x = std::stof(vecTokens[1]);
			float y = std::stof(vecTokens[2]);
			float z = std::stof(vecTokens[3]);
			vecPositions.emplace_back(x, y, z);
		}
		else if (strCommand == "vt") { // Vertex Texture
			if (vecTokens.size() < 3) {
				std::cerr << "[WARN] Malfored 'vt' at line " << nLineNum << std::endl;
				continue;
			}

			float u = std::stof(vecTokens[1]);
			float v = std::stof(vecTokens[2]);
			vecTexcoords.emplace_back(u, v);
		}
		else if (strCommand == "vn") { // Vertex Normal
			if (vecTokens.size() < 4) {
				std::cerr << "[WARN] Malfored 'vn' at line " << nLineNum << std::endl;
				continue;
			}

			float x = std::stof(vecTokens[1]);
			float y = std::stof(vecTokens[2]);
			float z = std::stof(vecTokens[3]);
			vecNormals.emplace_back(x, y, z);
		}
		else if (strCommand == "usemtl") { // Material
			if (vecTokens.size() < 2) {
				std::cerr << "[WARN] Malfored 'usemtl' at line " << nLineNum << std::endl;
				continue;
			}

			strCurrentMaterial = vecTokens[1];
		}
		else if (strCommand == "f") {
			Face_t Face;
			for (size_t i = 1; i < vecTokens.size(); i++) {
				const std::string& strVertSpec = vecTokens[i];
				FaceVertex_t fv = { 0, 0, 0 };

				size_t nSlash1 = strVertSpec.find('/');
				size_t nSlash2 = strVertSpec.rfind('/');

				if (nSlash1 == std::string::npos) {
					fv.p = std::stoi(strVertSpec);
				}
				else {
					if (nSlash1 != nSlash2) {
						std::string pstr = strVertSpec.substr(0, nSlash1);
						if (!pstr.empty()) fv.p = std::stoi(pstr);
						std::string tstr = strVertSpec.substr(nSlash1 + 1, nSlash2 - nSlash1 - 1);
						if (!tstr.empty()) fv.t = std::stoi(tstr);
						std::string nstr = strVertSpec.substr(nSlash2 + 1);
						if (!nstr.empty()) fv.n = std::stoi(nstr);
					}
					else {
						std::string nstr = strVertSpec.substr(nSlash2 + 1);
						if (!nstr.empty()) fv.n = std::stoi(nstr);
					}
				}

				Face.vecVertices.push_back(fv);
			}

			if (Face.vecVertices.size() < 3) {
				std::cerr << "[WARN] Face with less than 3 vertices at line " << nLineNum << std::endl;
				continue;
			}

			Face.strMaterial = strCurrentMaterial;
			vecFaces.push_back(std::move(Face));
		}
	}

	if (vecPositions.empty()) {
		std::cerr << "[ERR] No positions found in file." << std::endl;
		return false;
	}

	bool bHasNormals = !vecNormals.empty();
	bool bHasTexcoords = !vecTexcoords.empty();

	std::vector<FNTMDL_VERTEX> vecVertices;
	std::vector<uint32_t> vecIndices;
	std::vector<FNTMDL_SUBMESH> vecSubmeshes;

	std::unordered_map<uint64_t, uint32_t> UniqueMap;
	auto PackKey = [](int p, int t, int n) -> uint64_t {
		uint64_t uKey = 0;
		uKey |= (static_cast<uint64_t>(p) & 0x1FFFFF) << 42;
		uKey |= (static_cast<uint64_t>(t) & 0x1FFFFF) << 21;
		uKey |= (static_cast<uint64_t>(n) & 0x1FFFFF);
		return uKey;
	};

	std::string strSubmeshMaterial;
	uint32_t nSubmeshIndexStart = 0;
	uint32_t nSubmeshIndexCount = 0;

	auto FlushSubmesh = [&]() {
		if (nSubmeshIndexCount == 0) return;
		FNTMDL_SUBMESH Submesh = {};
		Submesh.nIndexOffset = nSubmeshIndexStart;
		Submesh.nIndexCount = nSubmeshIndexCount;

		std::string strMaterialName = strSubmeshMaterial;
		if (strMaterialName.empty())
			strMaterialName = "default";

		std::string strPath;
		if (pMapping && pMapping->count(strMaterialName))
			strPath = pMapping->at(strMaterialName);
		else if (!strMaterialName.empty())
			strPath = strMaterialPrefix + strMaterialName + ".fntmat";

		memcpy(Submesh.szMaterialPath, strPath.c_str(), std::min<size_t>(strPath.size(), 259));
		vecSubmeshes.push_back(Submesh);
		std::cout << "Packed submesh. Material path - " << Submesh.szMaterialPath << std::endl;
	};

	for (const Face_t& Face : vecFaces) {
		if (Face.strMaterial != strSubmeshMaterial) {
			FlushSubmesh();
			strSubmeshMaterial = Face.strMaterial;
			nSubmeshIndexStart = static_cast<uint32_t>(vecIndices.size());
			nSubmeshIndexCount = 0;
		}

		for (const FaceVertex_t& FaceVertex : Face.vecVertices) {
			int nPosIndex = FaceVertex.p - 1;
			int nTexcoordIndex = (bHasTexcoords && FaceVertex.t > 0) ? FaceVertex.t - 1 : -1;
			int nNormalIndex = (bHasTexcoords && FaceVertex.n > 0) ? FaceVertex.n - 1 : -1;

			if (nPosIndex < 0 || nPosIndex >= static_cast<int>(vecPositions.size())) {
				std::cerr << "[ERROR] Position index out of range.\n";
				return false;
			}

			uint64_t nKey = PackKey(FaceVertex.p, FaceVertex.t, FaceVertex.n);
			auto pIterator = UniqueMap.find(nKey);
			if (pIterator != UniqueMap.end()) {
				vecIndices.push_back(pIterator->second);
			}
			else {
				uint32_t nNewIndex = static_cast<uint32_t>(vecVertices.size());

				FNTMDL_VERTEX Vertex = {};
				const Vector3_t& vecPosition = vecPositions[nPosIndex];
				Vertex.px = vecPosition.x;
				Vertex.py = vecPosition.y;
				Vertex.pz = vecPosition.z;

				if (bHasNormals && nNormalIndex >= 0) {
					const Vector3_t& vecNormal = vecNormals[nNormalIndex];
					Vertex.nx = vecNormal.x;
					Vertex.ny = vecNormal.y;
					Vertex.nz = vecNormal.z;
				}
				else {
					Vertex.nx = Vertex.ny = Vertex.nz = 0.f;
				}

				if (bHasTexcoords && nTexcoordIndex >= 0) {
					const Vector2_t& vecTexcoord = vecTexcoords[nTexcoordIndex];
					Vertex.tu = vecTexcoord.u;
					Vertex.tv = vecTexcoord.v;
				}
				else {
					Vertex.tu = Vertex.tv = 0.f;
				}

				vecVertices.push_back(Vertex);
				UniqueMap[nKey] = nNewIndex;
				vecIndices.push_back(nNewIndex);
			}

			++nSubmeshIndexCount;
		}
	}
	FlushSubmesh();

	if (vecSubmeshes.empty()) {
		FNTMDL_SUBMESH Submesh = {};
		Submesh.nIndexOffset = 0;
		Submesh.nIndexCount = static_cast<uint32_t>(vecIndices.size());
		vecSubmeshes.push_back(Submesh);
	}

	std::ofstream fout(strOutputPath, std::ios::binary);
	if (!fout.is_open()) {
		std::cerr << "[ERROR] Cannot create output file " << strOutputPath << std::endl;
		return false;
	}

	FNTMDL_HEADER Header = {};
	Header.nMagic = 0x4D544E46;
	Header.nVersion = 2;

	Header.nVertexCount = static_cast<uint32_t>(vecVertices.size());
	Header.nIndexCount = static_cast<uint32_t>(vecIndices.size());
	Header.nSubmeshCount = static_cast<uint32_t>(vecSubmeshes.size());

	Header.nVertexOffset = sizeof(Header);
	Header.nIndexOffset = Header.nVertexOffset + Header.nVertexCount * sizeof(FNTMDL_VERTEX);
	Header.nSubmeshOffset = Header.nIndexOffset + Header.nIndexCount * sizeof(uint32_t);

	Header.nFlags = (bHasNormals ? 1 : 0) | (bHasTexcoords ? 2 : 0);

	fout.write(reinterpret_cast<const char*>(&Header), sizeof(Header));
	fout.write(reinterpret_cast<const char*>(vecVertices.data()), vecVertices.size() * sizeof(FNTMDL_VERTEX));
	fout.write(reinterpret_cast<const char*>(vecIndices.data()), vecIndices.size() * sizeof(uint32_t));
	fout.write(reinterpret_cast<const char*>(vecSubmeshes.data()), vecSubmeshes.size() * sizeof(FNTMDL_SUBMESH));

	std::cout << "Conversion successful!" << std::endl;
	std::cout << "Vertices: " << vecVertices.size() 
		<< ", Indices: " << vecIndices.size() 
		<< ", Submeshes: " << vecSubmeshes.size() << std::endl;
	std::cout << "Output file: " << strOutputPath << std::endl;
	
	return true;
}

int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cout << "Usage: " << argv[0] << " <input.obj> <output.fntmdl> [mapping.txt] [material_prefix]\n";
		return 1;
	}

	std::unordered_map<std::string, std::string> Mapping;
	if (argc >= 4) {
		std::ifstream fin(argv[3]);
		std::string strLine;
		while (std::getline(fin, strLine)) {
			if (strLine.empty() || strLine[0] == '#')
				continue;

			std::istringstream iss(strLine);
			std::string strName, strPath;
			if (iss >> strName >> strPath)
				Mapping[strName] = strPath;
		}
	}

	std::string strMaterialPrefix = (argc >= 5) ? argv[4] : "materials/";

	if (!std::filesystem::exists(argv[1])) {
		std::cerr << "[ERROR] Input file does not exists. " << std::endl;
		return 1;
	}

	if (!ConvertObjectToFntmdl(argv[1], argv[2], strMaterialPrefix,
		Mapping.empty() ? nullptr : &Mapping)) 
	{
		std::cerr << "Conversion failed.\n";
		return 1;
	}

	return 0;
}