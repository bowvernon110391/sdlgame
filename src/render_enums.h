#pragma once

enum GeomType {
	STATIC = 0,
	SKINNED,
	BILLBOARD
};

enum OpacityType {
	OPAQUE = 0,
	ALPHA_CLIP,
	ALPHA_BLEND
};

enum BlendMode {
	NORMAL = 0,
	ADD,
	DST_MULTIPLY
};

enum LightType {
	UNLIT = 0,
	AMBIENT,
	DIRECTIONAL,
	POINT,
	SPOT,
	EMISSION
};
