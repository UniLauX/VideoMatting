#pragma once

#include <vector>
#include "simpleimage.h"
#include "SiftFeaturePoint.h"

namespace SIMPLESIFT
{class CMultiGradient
{
public:
	CMultiGradient(CSimpleImagef *grad, 
		CSimpleImagef *mags,
		CSimpleImagef *dir,
		int start,
		int end);
	CMultiGradient(const CMultiGradient& object);

	~CMultiGradient(void);

	void operator()();

	//////////////////////////////////////////////////////////////////////////
	CSimpleImagef *_grad;
	CSimpleImagef *_mag;
	CSimpleImagef *_dir;


	//////////////////////////////////////////////////////////////////////////
	int _start;
	int _end;
};
}
