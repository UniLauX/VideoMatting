#pragma once
#include "ImageArray.h"
#include "SIFTLibraryPreq.h"

namespace SIFT
{

class KeyGen
{
public:

	KeyGen(void){}
	~KeyGen(void){}

	// detailed implementation required.
	virtual bool Generate(ImageArrayf* img, KeyPointVector* kps) = 0;
};
} // namespace SIFT