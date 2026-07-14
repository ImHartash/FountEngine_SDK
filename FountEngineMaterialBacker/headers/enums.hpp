#pragma once
#include <cstdint>

enum class EBlendMode : uint32_t {
	Opaque = 0,
	AlphaBlend,
	Additive
};

enum class ECullMode : uint32_t {
	Back = 0,
	Front,
	None
};

enum class EDepthMode : uint32_t {
	Enabled = 0,
	Disabled,
	ReadOnly
};