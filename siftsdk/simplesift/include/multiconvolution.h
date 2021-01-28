#pragma once

#include "Vision/Image/simpleimage/include/simpleimage.h"
#include "Vision/Image/simpleimage/include/simpleimageopencv.h"


class CMultiConvolution
{
public:
	CMultiConvolution(void);
	~CMultiConvolution(void);

	void operator()();

	SimpleImage::CGaussianConvolutorOpenCV* _convolution;
	CSimpleImagef* _image;
};
