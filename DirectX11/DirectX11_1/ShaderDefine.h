#pragma once

extern const int TEXTURE_MAIN;
extern const int TEXTURE_SPH;
extern const int TEXTURE_SPA;

extern const int TEXTURE_SUB;
extern const int TEXTURE_SUB2;
extern const int TEXTURE_NORMAL;
extern const int TEXTURE_HEIGHT;
extern const int TEXTURE_DISPLACEMENT;
extern const int TEXTURE_MASK;
extern const int TEXTURE_DECAL;
extern const int TEXTURE_CAMERA_DEPTH;
extern const int TEXTURE_LIGHT_DEPTH;
extern const int TEXTURE_FLOW;

struct BlurWeight
{
	float weight0;
	float weight1;
	float weight2;
	float weight3;
	float weight4;
	float weight5;
	float weight6;
	float weight7;
	float ofsX;
	float ofsY;
};