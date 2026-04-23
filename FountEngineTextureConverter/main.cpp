#include <iostream>
#include <vector>
#include <fstream>
#include "fnttex_header.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "ext/stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "ext/stb_image_resize2.h"

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <input_image> [output.fnttex] [max_mip_maps]\n";
		std::cerr << "Ex: " << argv[0] << " wood.png wood.fnttex\n";
		std::cerr << "If output is ommited, it will be replaced by input with .fnttex extension.\n";
		std::cerr << "max_mip_maps (optional) limits num of mipmaps, 0 = all possible.\n";
		return 1;
	}

	std::string strInputPath = argv[1];
	std::string strOutputPath;
	int nMaxMipOverride = 0;

	if (argc >= 3) {
		char* pEnd;
		long lValue = strtol(argv[2], &pEnd, 10);
		if (*pEnd == '\0') {
			nMaxMipOverride = static_cast<int>(lValue);
			if (argc >= 4) strOutputPath = argv[3];
		}
		else {
			strOutputPath = argv[2];
			if (argc >= 4) nMaxMipOverride = static_cast<int>(strtol(argv[3], nullptr, 10));
		}
	}

	if (strOutputPath.empty()) {
		size_t nDotPos = strInputPath.find_last_of('.');
		if (nDotPos != std::string::npos)
			strOutputPath = strInputPath.substr(0, nDotPos) + ".fnttex";
		else
			strOutputPath = strInputPath + ".fnttex";
	}

	int nWidth, nHeight, nChannelsCount;
	unsigned char* pImage = stbi_load(strInputPath.c_str(), &nWidth, &nHeight, &nChannelsCount, 4);
	if (!pImage) {
		std::cerr << "Failed to load image: " << strInputPath << "\n";
		return 1;
	}

	std::cout << "Loaded image " << nWidth << "x" << nHeight << " (" << nChannelsCount << " channels -> RGBA)\n";

	int nMaxSize = std::max(nWidth, nHeight);
	int nMaxPossibleMips = 0;
	while (nMaxSize > 0) {
		nMaxPossibleMips++;
		nMaxSize >>= 1;
	}

	int nMipLevels = nMaxPossibleMips;
	if (nMaxMipOverride > 0 && nMaxMipOverride < nMipLevels)
		nMipLevels = nMaxMipOverride;

	std::cout << "Generating " << nMipLevels << " mip levels\n";

	std::vector<std::vector<uint8_t>> vecMipData(nMipLevels);
	std::vector<int> vecMipWidths(nMipLevels);
	std::vector<int> vecMipHeights(nMipLevels);

	vecMipWidths[0] = nWidth;
	vecMipHeights[0] = nHeight;
	size_t nLevel0Size = static_cast<size_t>(nWidth) * nHeight * 4;
	vecMipData[0].assign(pImage, pImage + nLevel0Size);

	for (int i = 1; i < nMipLevels; i++) {
		int nPrevWidth = vecMipWidths[i - 1];
		int nPrevHeight = vecMipHeights[i - 1];
		int nNewWidth = std::max(1, nPrevWidth / 2);
		int nNewHeight = std::max(1, nPrevHeight / 2);

		vecMipWidths[i] = nNewWidth;
		vecMipHeights[i] = nNewHeight;
		vecMipData[i].resize(nNewWidth * nNewHeight * 4);

		if (!stbir_resize_uint8_srgb(
				vecMipData[i - 1].data(), nPrevWidth, nPrevHeight, 0,
				vecMipData[i].data(), nNewWidth, nNewHeight, 0,
				STBIR_RGBA)) {
			std::cerr << "Failed to generate mip level " << i << "\n";
			stbi_image_free(pImage);
			return 1;
		}
	}

	// Clear image pls..
	stbi_image_free(pImage);

	FNTTEX_HEADER Header;
	Header.nMagic = 0x54544E46;
	Header.nVersion = 1;
	Header.nWidth = nWidth;
	Header.nHeight = nHeight;
	Header.nMipLevels = nMipLevels;
	Header.nDXFormat = 28; // DXGI_FORMAT_R8G8B8A8_UNORM

	std::ofstream fout(strOutputPath, std::ios::binary);
	if (!fout) {
		std::cerr << "Cannot create output file: " << strOutputPath << "\n";
		return 1;
	}

	fout.write(reinterpret_cast<const char*>(&Header), sizeof(Header));
	for (int i = 0; i < nMipLevels; i++)
		fout.write(reinterpret_cast<const char*>(vecMipData[i].data()), vecMipData[i].size());

	fout.close();
	std::cout << "Successfully baked to " << strOutputPath << "\n";
	return 0;
}