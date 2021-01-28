#pragma once
#include "ImageBuffer.h"
#include "OpticalFlowFileManager.h"
#include "ZImage.h"
class TemporalMattingManager
{
public:
	TemporalMattingManager(ImageBuffer* _srcbuf, ImageBuffer* _trimbuf, ImageBuffer* _alphabuf, ImageBuffer* _resbuf,ImageBuffer* _fgbuffer, ImageBuffer* _bgbuffer,std::string _optpath, float fTmpWeight);
	void PathMatting(int _startframe, int _endframe, int _step, bool bAnchorFirst, bool bAnchorEnd);
	void DoTemporalMatting(int _startframe, int _endframe);
	void DoTemporalMatting_Fix(int _startframe, int _endframem, bool bAnchorFirst, bool bAnchorEnd);
	void PrepareTemporalInfo(int _istart, int _iend, FloatImgList& srcimg, IntImgList& csMapList, FloatImgList& wMapList,ByteImgList& ferrList);

	void ExtractForground(CxImage* src, CxImage* alpha, CxImage* fg);
	~TemporalMattingManager(void);
	ZByteImage* CheckConfidence(int index);

private:
	ImageBuffer* srcimgbuffer;
	ImageBuffer* trimapbuffer;
	ImageBuffer* alphabuffer;
	ImageBuffer* FGBuffer;
	ImageBuffer* BGBuffer;
	ImageBuffer* resbuf;
	OpticalFlowFileManager pfm;//manage optical flow files
	std::string optfilepath;
	int m_iWidth;
	int m_iHeight;
	float m_fTmpWeight;
};
