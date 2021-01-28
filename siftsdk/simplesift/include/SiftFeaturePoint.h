#pragma once

#include "simpleimage.h"

#include <vector>

namespace SIMPLESIFT
{
class ScaleSpace
{
public:
	CSimpleImagef m_base;
	std::vector<CSimpleImagef> m_scales;
	std::vector<CSimpleImagef> m_diffs;
};

//////////////////////////////////////////////////////////////////////////
extern int minwidth;
extern int maxoctaves;
extern int levels;
extern float startsigma;
extern int firstoctave;
//////////////////////////////////////////////////////////////////////////

extern bool filtereachoctave;

extern bool bUseGaussDeriate;

//////////////////////////////////////////////////////////////////////////
extern float dogThreshold;// 0.00700;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
extern int relocationMaximum;
extern float dValueLowThresh;
extern float maximumEdgeRatio;
extern float scaleAdjustThresh;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
extern bool enableadvanced;
extern int mindistance;
extern int maxcount;
//////////////////////////////////////////////////////////////////////////

extern float dMinDog;
extern float dMaxDog;

extern void print_params();

void setSensitivity(int iFeatureSens);

class ExtremePoint
{
public:
	ExtremePoint(int nx, int ny, int nlevel, float fdvalue):x(nx),y(ny),level(nlevel),dvalue(fdvalue)
	{
		adjustx = 0.0f;
		adjusty = 0.0;
		adjustlevel = 0.0f;
	}
	// peak position.
	int x;
	int y;
	int level;

	// local adjustment.
	float adjustx;
	float adjusty;
	float adjustlevel;
	float dvalue;

	float r;
	float g;
	float b;
};

class FeaturePoint
{
public:
	float foctavex;
	float foctavey;
	int octavex;
	int octavey;
	float octavescale;

	int x;
	int y;
	float fx;
	float fy;

	float scale;
	float dvalue;
	int octave;
};

#define OPENCV 1
#define NORMAL 0

class CSiftFeaturePoint
{
public:
	CSiftFeaturePoint(void);
	~CSiftFeaturePoint(void);

	std::vector<FeaturePoint> Generate(CSimpleImagef & image, int method = NORMAL);

	std::vector<ScaleSpace> m_octaves;
};
}