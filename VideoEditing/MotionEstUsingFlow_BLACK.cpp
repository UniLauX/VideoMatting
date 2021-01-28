#include "MotionEstUsingFlow_BLACK.h"
#include "../grab/BiImageProcess.h"
MotionEstUsingFlow_BLACK::MotionEstUsingFlow_BLACK(void)
{
}

MotionEstUsingFlow_BLACK::~MotionEstUsingFlow_BLACK(void)
{
}
void MotionEstUsingFlow_BLACK::PredictMotion(char* uflowfilename, char* vflowfilename, unsigned char* premask, 	
										unsigned char* predictedmask, int width, int height, VVPOINTS &newCenters, const VVPOINTS &localWins,int winSize)      //错误的改变了Predictedmask,而newCenters则和 winCenters值相同
{
	winSize = 3;
	float* u = new float[width*height];
	float* v = new float[width*height];
	memset(u, 0, sizeof(float)*width*height);
	memset(v, 0, sizeof(float)*width*height);
	//load optical flow
	FILE* pfile = NULL;
	pfile = fopen(uflowfilename, "rb");
	if (pfile)                                       //未执行
	{
		fread(u, sizeof(float), width*height, pfile);
		fclose(pfile);
		
	}
	pfile = fopen(vflowfilename, "rb");
	if (pfile)
	{
		fread(v, sizeof(float), width*height, pfile);
		fclose(pfile);
	}
	//load flow over
	memset(predictedmask, 0, sizeof(unsigned char)*width*height);
	//CxImage tempimg;
	//tempimg.Create(width, height, 24);
	//tempimg.Clear();
	//predict mask of next frame
	int i,j;
#pragma  omp parallel for private(i,j)                          //u[index]和v[index]全部为0，也就是说predictedmask和premask中的元素一样
	for ( j=0; j<height; ++j)
	{
		for ( i=0; i<width; ++i)
		{
			int index = j*width+i;
			
			if (premask[index])
			{
				int x = i + u[index];
				int y = j + v[index];
				if (x>=0&&x<width&&y>=0&&y<height)
					predictedmask[y*width+x] = 1;
				
			}
		}
	}// end of for j
	int xindex;
	int yindex;
	POINT ptemp;
	VPOINTS vptemp;
	for (i=0;i<localWins.size();++i)
	{
		vptemp.clear();
		ptemp.x=0;
		ptemp.y=0;
		for (j=0;j<(localWins[i]).size();++j)
		{
			//Xoffset=0;
			//Yoffset=0;
			//count=0;
			xindex=localWins[i][j].x;
			yindex=localWins[i][j].y;
			//for (itemp=xindex-winSize;itemp<=xindex+winSize;++itemp)
			//{
			//	for (jtemp=yindex-winSize;jtemp<=yindex+winSize;++jtemp)
			//	{
			//		if (BiImageProcess::isValid(itemp,jtemp,width,height))//)
			//		{
			//			if (premask[(jtemp)*width+itemp])
			//			{
			//				Xoffset+=u[jtemp*width+itemp];
			//				Yoffset+=v[jtemp*width+itemp];
			//				count++;
			//			}
			//		}
			//	}
			//}
			//if (count)
			//{
			//	Xoffset*=1.0f/count;
			//	Yoffset*=1.0f/count;
			//	ptemp.x=xindex+Xoffset;
			//	ptemp.y=yindex+Yoffset;
			//	vptemp.push_back(ptemp);
			//}
			ptemp.x = xindex +u[yindex*width+xindex];
			ptemp.y = yindex + v[yindex*width+xindex];

			vptemp.push_back(ptemp);
		}
		newCenters.push_back(vptemp);                                          //newCenters实际就是原来winCenters,并未有任何改变
	}
	unsigned char* templabel = new unsigned char[width*height];
	smoothLabel(templabel, predictedmask, width, height);

	//for (int j=0; j<height; ++j)
	//{
	//	for (int i=0; i<width; ++i)
	//	{
	//		BYTE index = predictedmask[j*width+i]*255;
	//		//tempimg.SetPixelColor(i,j, RGB(index, index, index));
	//	}
	//}
	//tempimg.Save("H:/test/premask.bmp", CXIMAGE_FORMAT_BMP);
	delete[] templabel;
	delete[] u;
	delete[] v;

}
void MotionEstUsingFlow_BLACK::smoothLabel(unsigned char *label,unsigned char* newlabel,int width,int height)
{
	int i,j;
	int index=0;
	int count;
	int xtemp;
	int ytemp;
	memcpy(label, newlabel, sizeof(unsigned char)*width*height);
	for (i=0;i<width;++i)                                                   
		for (j=0;j<height;++j)                                        
		{
			count=0;
			index=j*width+i;
			if (label[index])                                //找出label[index]为1周围6*6方格所有label[index]为1的元素个数,
			{
				for (xtemp=i-3;xtemp<i+3;++xtemp)
					for (ytemp=j-3;ytemp<j+3;++ytemp)
					{
						index=ytemp*width+xtemp;
						if (BiImageProcess::isValid(xtemp, ytemp, width,height))
						{
							if (label[index])
								count++;							
						}
					}
				if (count>10)                                                             //在此处改变了newlabel[index]中部分元素，不过完全是乱弄
					for (xtemp=i-3;xtemp<i+3;++xtemp)
					{
						for (ytemp=j-3;ytemp<j+3;++ytemp)
						{
							index=ytemp*width+xtemp;
							if (BiImageProcess::isValid(xtemp, ytemp, width,height))
								newlabel[index]=1;	
						}
					}

			}
		

		}
}

#include <Windows.h>

void MotionEstUsingFlow_BLACK::CheckMemory()
{
	MEMORYSTATUS myMemoryStatus;
	memset(&myMemoryStatus, 0, sizeof(MEMORYSTATUS));
	myMemoryStatus.dwLength = sizeof(MEMORYSTATUS);

	GlobalMemoryStatus(&myMemoryStatus);

	const int nOneM = 1024 * 1024;
	printf("memory of P %fM, available memory of P %fM\n",
		(double)myMemoryStatus.dwTotalVirtual / nOneM, (double)myMemoryStatus.dwAvailVirtual / nOneM
		);
}
