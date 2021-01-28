#include "stdafx.h"
#include "mattingutil.h"
#include "grabcut/FastMarchingMethod.h"

#include "CameraFrame.h"

#include "globalpublic.h"
#include "MattingManager.h"

bool MattingAlgorithm::MattingUtil::GenerateTrimap(int index, int expand)
{
	CameraFrame *pFrame = lpGlobalMotion->GetCameraFrame(index);

	if (pFrame == NULL)
	{
		return false;
	}

	ZByteImage& trimap = pFrame->m_triMap;
	ZByteImage& alpha = pFrame->m_AlphaMatte;

	int width = alpha.GetWidth();
	int height = alpha.GetHeight();

	// not a cut result.
	for (int x = 0; x < width; ++ x)
	{
		for (int y = 0; y < height; ++ y)
		{
			unsigned char a = alpha.at(x, y);
			if (a > 0 && a < 255)
			{
				CMattingManager::GetInstance()->ImageMatting(index);
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	float *forevalues = new float[width*height];
	FLAG_TYPE *flags = new FLAG_TYPE[width*height];

	//////////////////////////////////////////////////////////////////////////
	// expand to the foreground.
	for (int y = 0; y < height; ++ y)
	{
		for ( int x = 0; x < width; ++ x)
		{
			forevalues[y * width + x] = INFINITY;
			flags [y * width + x] = FAR_AWAY;
		}
	}

	for (int y = 1; y < height - 1; ++ y)
	{
		for ( int x = 1; x < width - 1; ++ x)
		{
			if (alpha.at(x, y) == 0)
			{
				forevalues[y * width + x] = 0;
				flags[y * width + x] = ALIVE;
				
				if (alpha.at(x, y + 1) == 255
					|| alpha.at(x + 1, y) == 255
					|| alpha.at(x - 1, y) == 255
					|| alpha.at(x, y - 1) == 255)
				{
					forevalues[y * width + x] = 1;
					flags [y * width + x] = NARROW_BAND;
				}
			}
			else
			{
				forevalues[y * width + x] = INFINITY;
				flags [y * width + x] = FAR_AWAY;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	FastMarchingMethod fmm0(forevalues, flags, width, height);
	fmm0.execute(expand);

	trimap.Create(width, height);
	for(int y = 0; y < height; ++ y)
	{
		for(int x = 0; x < width; ++ x)
		{
			if (alpha.at(x, y) == 255)
			{
				if (flags[y*width + x] == ALIVE)
				{
					trimap.at(x, y) = REGION_UNKNOWN;
				}
				else
				{
					trimap.at(x, y) = REGION_FG;
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// expand to the background..
	for (int y = 0; y < height; ++ y)
	{
		for ( int x = 0; x < width; ++ x)
		{
			forevalues[y * width + x] = INFINITY;
			flags [y * width + x] = FAR_AWAY;
		}
	}
	for (int y = 1; y < height - 1; ++ y)
	{
		for ( int x = 1; x < width - 1; ++ x)
		{
			if (alpha.at(x, y) == 255)
			{
				forevalues[y * width + x] = 0;
				flags[y * width + x] = ALIVE;

				if (alpha.at(x, y + 1) == 0
					|| alpha.at(x + 1, y) == 0
					|| alpha.at(x - 1, y) == 0
					|| alpha.at(x, y - 1) == 0)
				{
					forevalues[y * width + x] = 1;
					flags [y * width + x] = NARROW_BAND;
				}
			}
			else
			{
				forevalues[y * width + x] = INFINITY;
				flags [y * width + x] = FAR_AWAY;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	FastMarchingMethod fmm1(forevalues, flags, width, height);
	fmm1.execute(expand);

	for(int y = 0; y < height; ++ y)
	{
		for(int x = 0; x < width; ++ x)
		{
			if (alpha.at(x, y) == 0)
			{
				if (flags[y*width + x] == ALIVE)
				{
					trimap.at(x, y) = 128;
				}
				else
				{
					trimap.at(x, y) = 0;
				}
			}
		}
	}

	delete []forevalues;
	delete []flags;
	pFrame->SetDataModified(true);

	return true;
}

bool MattingAlgorithm::MattingUtil::DeflateTrimap(int index, int step)
{
	CameraFrame *pFrame = lpGlobalMotion->GetCameraFrame(index);

	if (pFrame == NULL)
	{
		return false;
	}

	ZByteImage& trimap = pFrame->m_triMap;
	int width = trimap.GetWidth();
	int height = trimap.GetHeight();

	ZByteImage newtrimap = trimap;

	for (int x = 1; x < width - 1; ++ x)
	{
		for (int y = 1; y < height - 1; ++ y)
		{
			unsigned char &a = newtrimap.at(x, y);
			if (a == REGION_UNKNOWN)
			{
				if (trimap.at(x + 1, y) == REGION_FG
					||trimap.at(x - 1, y) == REGION_FG
					||trimap.at(x, y + 1) == REGION_FG
					||trimap.at(x, y - 1) == REGION_FG)
				{
					a = REGION_FG;
				}
				if (trimap.at(x + 1, y) == REGION_BG
					||trimap.at(x - 1, y) == REGION_BG
					||trimap.at(x, y + 1) == REGION_BG
					||trimap.at(x, y - 1) == REGION_BG)
				{
					a = REGION_BG;
				}
			}
		}
	}

	trimap = newtrimap;

	return true;
}

bool MattingAlgorithm::MattingUtil::InflateTrimap(int index ,int step)
{
	CameraFrame *pFrame = lpGlobalMotion->GetCameraFrame(index);

	if (pFrame == NULL)
	{
		return false;
	}

	ZByteImage& trimap = pFrame->m_triMap;
	int width = trimap.GetWidth();
	int height = trimap.GetHeight();

	ZByteImage newtrimap = trimap;

	for (int x = 1; x < width - 1; ++ x)
	{
		for (int y = 1; y < height - 1; ++ y)
		{
			unsigned char &a = newtrimap.at(x, y);
			if (a == REGION_FG)
			{
				if(trimap.at(x + 1, y) == REGION_UNKNOWN
					||trimap.at(x - 1, y) == REGION_UNKNOWN
					||trimap.at(x, y + 1) == REGION_UNKNOWN
					||trimap.at(x, y - 1) == REGION_UNKNOWN)
				{
					a = REGION_UNKNOWN;
				}
			}
			else if(a == REGION_BG)
			{
				if (trimap.at(x + 1, y) == REGION_UNKNOWN
					||trimap.at(x - 1, y) == REGION_UNKNOWN
					||trimap.at(x, y + 1) == REGION_UNKNOWN
					||trimap.at(x, y - 1) == REGION_UNKNOWN)
				{
					a = REGION_UNKNOWN;
				}
			}
		}
	}

	trimap = newtrimap;

	return true;
}