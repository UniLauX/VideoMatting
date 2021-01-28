#include "../RotoScoping/stdafx.h"
#include "../RotoScoping/VisualizeMat.h"
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"
#include "cvaux.h"
#include "../RotoScoping/BlockMatProxy.h"

void VisMat(std::vector<int> *NonZeroRowIndex,int NumOfRow,int NumOfCol,char * FileName)
{
	IplImage * Image = cvCreateImage(cvSize(NumOfCol,NumOfRow),IPL_DEPTH_8U,1);
	cvZero(Image);

	for (int c=0;c<NumOfCol;c++)
	{
		for (int r=0;r<NonZeroRowIndex[c].size();r++)
		{
			cvSetReal2D(Image,NonZeroRowIndex[c][r],c,255);
		}
	}

	cvSaveImage(FileName,Image);
	cvReleaseImage(&Image);

}

void VisMat(stdext::hash_map<int,Block_Offset> *NonZeroRowIndex,int NumOfRow,int NumOfCol,char * FileName)
{
	IplImage * Image = cvCreateImage(cvSize(NumOfCol,NumOfRow),IPL_DEPTH_8U,1);
	cvZero(Image);

	for (int c=0;c<NumOfCol;c++)
	{
		
		stdext::hash_map<int,Block_Offset>::iterator it = NonZeroRowIndex[c].begin();
		for (;it!=NonZeroRowIndex[c].end();it++)
		{
			cvSetReal2D(Image,(*it).first,c,255);
		}
	}

	cvSaveImage(FileName,Image);
	cvReleaseImage(&Image);


}