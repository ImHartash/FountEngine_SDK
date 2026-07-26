#pragma once
#include <cstdint>

#pragma pack(push, 1)
struct FNTMDL_HEADER {
	uint32_t nMagic;
	uint32_t nVersion;
	uint32_t nVertexCount;
	uint32_t nIndexCount;
	uint32_t nVertexOffset;
	uint32_t nIndexOffset;
	uint32_t nFlags;
};

struct FNTTEX_HEADER {
	uint32_t nMagic;
	uint32_t nVersion;
	uint32_t nWidth;
	uint32_t nHeight;
	uint32_t nMipLevels;
	uint32_t nDXFormat;
};

struct FNTPK_HEADER {
	uint32_t nMagic;
	uint32_t nVersion;
	uint32_t nFlags;
	uint32_t nCRC;
	uint32_t nBlocksCount;
};

struct FNTPK_BLOCK {
	char szResourceName[256];
	uint32_t nBlockDataOffset;
	uint32_t nBlockDataSize;
};
#pragma pack(pop)