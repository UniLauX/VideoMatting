#pragma once

#include "SimpleKeypoint.h"

#include "z:/vision/image/simpleimage/include/simpleimage.h"
#include "os_mapping.h"

namespace SURF
{
	// Initial sampling step (default 2)
	extern int samplingStep;
	// Number of analysed octaves (default 4)
	extern int octaves;
	// Blob response treshold
	extern double thres;
	// Set this flag "true" to double the image size
	extern bool doubleImageSize;
	// Initial lobe size, default 3 and 5 (with double image size)
	extern int initLobe;
	// Upright SURF or rotation invaraiant
	extern bool upright;
	// If the extended flag is turned on, SURF 128 is used
	extern bool extended;
	// Spatial size of the descriptor window (default 4)
	extern int indexSize;
	// Variables for the timing measure
	extern osmapping::os_TIME tim1, tim2; //STS
	// verbose output
	extern bool bVerbose;
	// skip sign of laplacian
	extern bool bLaplacian;


	// max number of surf features
	extern int maxnumber;

	class CSurfKeypoint
	{
	public:
		CSurfKeypoint(void);
		~CSurfKeypoint(void);

		static std::vector<CSimpleKeypoint> Generate(CSimpleImagef & image);
	};
}
