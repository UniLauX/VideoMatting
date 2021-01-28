#include "Refiner.h"
#include "OpticalProcessor.h"
#include "Global.h"
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

Refiner::Refiner()
{
    outDir="F:/data/";

    gmmWin=15;
    boundWin=10;
}

Refiner::Refiner(string _outDir, string _moveDir, string _optDir, string _trimapDir)
{
	outDir = _outDir;
	moveDir = _moveDir;
	optDir = _optDir;
	trimapDir = _trimapDir;
}

Refiner::~Refiner()
{

}

void Refiner::SetOutputDir(string dir)
{
    outDir=dir;
}

void Refiner::SetVideo(string name, int start, int end, int optStart, int optEnd)
{
    video.SetInfo(name, start, end, optStart, optEnd);
}

static int *LoadLabel(const char *dir, int frame, int &w, int &h)
{
    int *label;
    CxImage img;
    char name[1000];

    sprintf_s(name, "%s/%d.png", dir, frame);
    img.Load(name);
    w=img.GetWidth();
    h=img.GetHeight();
    label=new int[w*h];
    for(int i=0;i<h;++i)
    {
        for(int j=0;j<w;++j)
        {
            if(img.GetPixelGray(j, i)>128)
                label[j+i*w]=1;
            else
                label[j+i*w]=0;
        }
    }
    return label;
}

static int FindMin(int w, int h, int x, int y, int win, 
                   const vector<OptData> &frames, const int *gray, float *p)
{
    int   i,j,d;
    int   a,b,c,u,r(0);
    int   half(win/2);
    float M(0),s,t,v;
    float T1(2.0f),T2(64.0f);
    float T3(2.0f),T4(50.0f);
    vector<OptData>::const_iterator it;

    for(d=0;d<=1;++d)
    {
        s=0;
        for(it=frames.begin();it!=frames.end();++it)
        {
            v=0.0f;
            c=u=0;
            for(i=-half;i<=half;++i)
            {
                for(j=-half;j<=half;++j)
                {
                    a=x+j;
                    b=y+i;
                    if(a>=0 && a<w && b>=0 && b<h)
                    {
                        if((t=it->data[a+b*w])>=0)
                        {
                            v+=(float)exp(-(d-t)*(d-t)/T1)*               // Label...
                                exp(-(i*i+j*j)/T2)*                       // Distance...
                                exp(-it->r[a+b*w]/T3)*                    // Optical flow reliable...
                                (T4/(T4+abs(gray[a+b*w]-gray[x+y*w])));   // Color...

                            if((int)t==d)
                                ++u;
                        }
                        ++c;
                    }
                }
            }
            s+=v*u/(float)c;
        }
        p[d]=s;
    }
    v=p[0]+p[1];
    p[0]=p[0]/(v+1e-10f);
    p[1]=p[1]/(v+1e-10f);
    return r;
}

void Refiner::OptRefineCuttingProb(int frame, float *r, int win)
{
    int              i,j;
    int              w,h;
    int *            gray;
    OpticalProcessor detector(video.width, video.height);
    vector<OptData>  t;
    CxImage          image;

    w=video.width;
    h=video.height;
    gray=new int[w*h];
    detector.SetVideo(video);
    detector.SetDir(optDir, "", moveDir);

    video.LoadImage(frame, image);
    ImageToGray(image, gray);
   // detector.LoadOptLabel(frame, win, t);
	detector.LoadOptLabel_jin(frame, win, t);

    #pragma omp parallel for private(j)
    for(i=0;i<h;++i)
    {
        for(j=0;j<w;++j)
        {
            float p[2];
            FindMin(w, h, j, i, 8, t, gray, p);
            r[j+i*w]=p[1];
        }
    }
    for(vector<OptData>::iterator it=t.begin();it!=t.end();++it)
        it->Destroy();
    delete [] gray;
	printf("After OptRefineCuttingProb\n");
}

void Refiner::Trimap_Common(CxImage& trimap, CxImage& newTrimap)
{
	CxImage temp;
	temp.Copy(trimap);
	temp.Clear(0);
	for (int j=0; j<trimap.GetHeight();++j)
	{
		for (int i=0; i<trimap.GetWidth();++i)
		{
			if (trimap.GetPixelGray(i,j)==newTrimap.GetPixelGray(i,j))
			{
				temp.SetPixelColor(i,j,newTrimap.GetPixelColor(i,j));
			}
		}
	}
	trimap.Copy(temp);
}

#include "../VideoCut/Contour.h"
#include "../VideoCut/Cutting.h"
void Refiner::GMMRefineCutting()
{
    if(video.end-video.start<5)
        return;

    int     frame,n;
    int *   label;
    float * P;
    float * r;
    CxImage image,tmp, newTrimap;
    char    name[1000];

    n=video.width*video.height;
    label=new int[n];
    P=new float[n];
    r=new float[n];
    for(frame=video.start;frame<video.end;++frame)
    {
        video.LoadImage(frame, image);
		LoadImage((moveDir+video.fileTitle).c_str(), "png", tmp);
		LoadImage((trimapDir+video.fileTitle).c_str(), "png", newTrimap);

        ImageToLabel(tmp, label);
        OptRefineCuttingProb(frame, r, 5);
        // Notice that the GMM results for every call are different due to the different k-means results.
        // There may be some errors!!!
        //for(int i=0;i<1;++i)
        //{
			int BGPtNm=0;
			int FGPtNm=0;
			for(int k=0;k<n;k++)
			{
				BGPtNm+=!label[k];
				FGPtNm+=label[k];
			}
            cout << "BGPtNm: " << BGPtNm << " FGPtNm: " << FGPtNm << endl;

			if(BGPtNm && FGPtNm)
			{
				Contour_ywz contour;
				GrabCut_ywz cut;
				contour.SetFrameInfo(&image, label);
				contour.SetLocalWinSize(20);
				CxImage tempTrimap;
				tempTrimap.Create(video.width, video.height, 8);
				tempTrimap.SetGrayPalette();
				tempTrimap.Clear();
				int pointNum;
				contour.GetFGProb(P, tempTrimap, pointNum);
				
				//if (i==0)
				//{
				//	char nametemp[1000];
				//	sprintf_s(nametemp, "H:/test/temp_trimap_%d.bmp", frame);
				//	tempTrimap.Save(nametemp, CXIMAGE_FORMAT_BMP);
				//}
				cut.Init(image, label, P);
				printf("Before cut: ");
				CheckMemory();
				//cut.Cut(label, r);
				std::cout<<" Point Num: "<<pointNum<<std::endl;
				this->Trimap_Common(tempTrimap, newTrimap);
				cut.Cut_graphcut(label, tempTrimap, pointNum,r);
				printf("After cut: ");
				CheckMemory();
			}
       // }
        //sprintf_s(name, "%s/%s/%d.bmp", outDir.c_str(), moveDir.c_str(), frame);
		sprintf_s(name, "%s%s.png", moveDir.c_str(),video.fileTitle.c_str());
        GetResult(video.width, video.height, label, tmp);
        tmp.Save(name, CXIMAGE_FORMAT_PNG);
    }
    delete [] label;
    delete [] P;
    delete [] r;
}
