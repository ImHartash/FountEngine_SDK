#pragma once
#include <cstdint>

/*
Shortly:

In file, first is haeader with fixed size (36 byte), in header we're reading
amount of vertices, indices and submodels. After header file contains vertex list
and index list of whole model. After this, it's contains submodels list.

Every submodel - range of indices which have a personal material, which describes 
by the path.

P.S.: Maximum amount of symbols in path is 260

FILE - [
[HEADER],
[VERTEX LIST],
[INDEX LIST],
[SUBMODELS LIST],
]

by. ImHartash
*/

#pragma pack(push, 1)
struct FNTMDL_HEADER {
	uint32_t nMagic; // 'FNTM'
	uint32_t nVersion;
	uint32_t nVertexCount;
	uint32_t nIndexCount;
	uint32_t nSubmeshCount;
	uint32_t nVertexOffset;
	uint32_t nIndexOffset;
	uint32_t nSubmeshOffset;
	uint32_t nFlags;
};

struct FNTMDL_SUBMESH {
	uint32_t nIndexOffset;
	uint32_t nIndexCount;
	char szMaterialPath[260];
};
#pragma pack(pop)