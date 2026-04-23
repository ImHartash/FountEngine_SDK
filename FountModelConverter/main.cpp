#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include "fntmdl_header.hpp"
#include "fntmdl_vertex.hpp"

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

bool StartsWith(const std::string& strSource, const std::string& strPrefix) {
	return strSource.size() >= strPrefix.size() && strSource.substr(0, strPrefix.size()) == strPrefix;
}

bool ConvertObjectToFntmdl(const std::string& strInputPath, const std::string& strOutputPath) {
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
	std::vector<std::vector<FaceVertex_t>> vecFaces;

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
		else if (strCommand == "f") {
			std::vector<FaceVertex_t> vecFace;
			for (size_t i = 1; i < vecTokens.size(); i++) {
				const std::string& strVertSpec = vecTokens[i];
				FaceVertex_t fv = { 0, 0, 0 };

				size_t nSlash1 = strVertSpec.find('/');
				size_t nSlash2 = strVertSpec.rfind('/');

				if (nSlash1 == std::string::npos) {
					fv.p = std::stoi(strVertSpec);
				}
				else {
					std::string pstr = strVertSpec.substr(0, nSlash1);
					if (!pstr.empty()) fv.p = std::stoi(pstr);
					if (nSlash1 != nSlash2) {
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

				vecFace.push_back(fv);
			}

			if (vecFace.size() < 3) {
				std::cerr << "[WARN] Face with less than 3 vertices at line " << nLineNum << std::endl;
				continue;
			}

			vecFaces.push_back(vecFace);
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

	std::unordered_map<uint64_t, uint32_t> UniqueMap;
	auto PackKey = [](int p, int t, int n) -> uint64_t {
		uint64_t uKey = 0;
		uKey |= (static_cast<uint64_t>(p) & 0x1FFFFF) << 42;
		uKey |= (static_cast<uint64_t>(t) & 0x1FFFFF) << 21;
		uKey |= (static_cast<uint64_t>(n) & 0x1FFFFF);
		return uKey;
	};

	for (const auto& vecFace : vecFaces) {
		for (const auto& fv : vecFace) {
			int nPosIndex = fv.p - 1;
			int nTexcoordIndex = (bHasTexcoords && fv.t > 0) ? fv.t - 1 : -1;
			int nNormalIndex = (bHasNormals && fv.n > 0) ? fv.n - 1 : -1;

			if (nPosIndex < 0 || nPosIndex >= static_cast<int>(vecPositions.size())) {
				std::cerr << "[ERROR] Position index out of range." << std::endl;
				return false;
			}

			uint64_t nKey = PackKey(fv.p, fv.t, fv.n);
			auto pIterator = UniqueMap.find(nKey);
			if (pIterator != UniqueMap.end()) {
				vecIndices.push_back(pIterator->second);
			}
			else {
				uint32_t nNewIndex = static_cast<uint32_t>(vecVertices.size());
				 
				FNTMDL_VERTEX Vertex;
				const Vector3_t& vecPos = vecPositions[nPosIndex];
				Vertex.px = vecPos.x;
				Vertex.py = vecPos.y;
				Vertex.pz = vecPos.z;

				if (bHasNormals && nNormalIndex >= 0) {
					const Vector3_t& vecNorm = vecNormals[nNormalIndex];
					Vertex.nx = vecNorm.x;
					Vertex.ny = vecNorm.y;
					Vertex.nz = vecNorm.z;
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
		}
	}

	std::ofstream fout(strOutputPath, std::ios::binary);
	if (!fout.is_open()) {
		std::cerr << "[ERROR] Cannot create output file " << strOutputPath << std::endl;
		return false;
	}

	FNTMDL_HEADER Header = {};
	Header.nMagic = 0x464E544D;
	Header.nVersion = 1;
	Header.nVertexCount = static_cast<uint32_t>(vecVertices.size());
	Header.nIndexCount = static_cast<uint32_t>(vecIndices.size());
	Header.nVertexOffset = sizeof(Header);
	Header.nIndexOffset = Header.nVertexOffset + Header.nVertexCount * sizeof(FNTMDL_VERTEX);
	Header.nFlags = (bHasNormals ? 1 : 0) | (bHasTexcoords ? 2 : 0);

	fout.write(reinterpret_cast<const char*>(&Header), sizeof(Header));
	fout.write(reinterpret_cast<const char*>(vecVertices.data()), vecVertices.size() * sizeof(FNTMDL_VERTEX));
	fout.write(reinterpret_cast<const char*>(vecIndices.data()), vecIndices.size() * sizeof(uint32_t));

	std::cout << "Conversion successful!" << std::endl;
	std::cout << "Vertices: " << vecVertices.size() << ", Indices: " << vecIndices.size() << std::endl;
	std::cout << "Output file: " << strOutputPath << std::endl;
	
	return true;
}

int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cout << "Usage: " << argv[0] << " <input.obj> <output.fntmdl>\n";
		return 1;
	}

	std::string strInputPath = argv[1];
	std::string strOutputPath = argv[2];

	if (!std::filesystem::exists(strInputPath)) {
		std::cerr << "[ERROR] Input file does not exists. " << std::endl;
		return 1;
	}

	if (!ConvertObjectToFntmdl(strInputPath, strOutputPath)) {
		std::cerr << "Conversion failed.\n";
		return 1;
	}

	return 0;
}