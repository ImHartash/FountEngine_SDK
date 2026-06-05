#pragma once
#include <string>
#include "enums.hpp"

struct MaterialDesc_t {
	std::string strDiffuseTexture;

	float flAmbient[3];
	float flDiffuse[3];
	float flSpecular[3];

	float flShininess;
	float flOpacity;

	EBlendMode BlendMode;
	ECullMode CullMode;
	EDepthMode DepthMode;
};