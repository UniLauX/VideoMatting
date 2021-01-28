#pragma once

#include "MattingElem.h"
#include <vector>
class MattingElemGenerator
{
public:
	MattingElemGenerator(void);
	~MattingElemGenerator(void);
	void GenerateMattingElem(std::vector<MattingElem*>& vec_matElem,  CxImage* trimap,  CxImage* colImage );
};
