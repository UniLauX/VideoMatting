#include "../VideoEditing/stdafx.h"
#include "../GrabCut/Cutting.h"
#include "../GrabCut/gmm.h"
#include "../GrabCut/Global.h"
#include <iostream>
#include <algorithm>

using namespace std;

Cutting::Cutting()
{
	Init();
}

Cutting::~Cutting()
{
	Clear();
}

void Cutting::Init()
{
	width=0;
	height=0;
	pixelNum=0;

	image=NULL;
	//bgPixels=NULL;
	bgPixels.clear();
	bgPixelsNum=0;
	bgCluster=NULL;
	
	//fgPixels=NULL;
	fgPixels.clear();
	fgPixelsNum=0;
	fgCluster=NULL;
	
	label=NULL;
	graph=NULL;
	graphPoint=NULL;
}

void Cutting::Init(CxImage *image)
{
	int i;
	Clear();
	this->image=image;
	width=image->GetWidth();
	height=image->GetHeight();
	pixelNum=width*height;
	left=0;
	right=image->GetWidth();
	top=image->GetHeight();
	bottom=0;
	bgPixelsNum=0;
	fgPixelsNum=0;
	bgCluster=new Cluster(5);	
	fgCluster=new Cluster(5);

	label=new unsigned char[pixelNum];
	memset(label, 0, sizeof(unsigned char)*pixelNum);
	graph=new Graph;
	//graphPoint=new void*[pixelNum];
	graphPoint = NULL;
}

void Cutting::Clear()
{
	delete bgCluster;
	delete fgCluster;
	delete [] label;
	label = NULL;
	delete graph;
	if (graphPoint!=NULL)
	{
		delete [] graphPoint;
		graphPoint = NULL;
	}
	Init();
}

void Cutting::SetImage(CxImage *image)
{
	this->image = image;
	Init(this->image);
}

inline bool Cutting::IsValid(int x, int y)
{
	if(x>=0 && x<width && y>=0 && y<height)
		return true;
	else
		return false;
}

void Cutting::Adjust(int &x1, int &y1, int &x2, int &y2)
{
	int tmp;
	if(x1>x2)
	{
		tmp=x1;
		x1=x2;
		x2=tmp;
	}
	if(y1>y2)
	{
		tmp=y1;
		y1=y2;
		y2=tmp;
	}
}

//void Cutting::GetCuttingRegion(int x1, int y1, int x2, int y2)
//{
//	int w,h;
//	int i,j,index;
//	RGBQUAD rgb;
//	double s(0.5);
//
//	Adjust(x1, y1, x2, y2);
//	X[0]=x1;X[1]=x2;
//	Y[0]=y1;Y[1]=y2;
//	w=(int)((x2-x1)*s); 
//	h=(int)((y2-y1)*s);
//	left=x1-w; right=x2+w;
//	top=y2+h; bottom=y1-h;
//	left=left<0 ? 0:left;
//	right=right>width ? width-1:right;
//	top=top>height ? height-1:top;
//	bottom=bottom<0 ? 0:bottom;
//
//	for(i=bottom;i<=top;++i)
//	{
//		for(j=left;j<=right;++j)
//		{
//			rgb=image->GetPixelColor(j, i);
//			if(i>=y1 && i<=y2 && j>=x1 && j<=x2)
//			{
//				index=3*fgPixelsNum;
//				fgPixels[index]=rgb.rgbRed;
//				fgPixels[index+1]=rgb.rgbGreen;
//				fgPixels[index+2]=rgb.rgbBlue;
//				++fgPixelsNum;
//			}
//			else
//			{
//				index=3*bgPixelsNum;
//				bgPixels[index]=rgb.rgbRed;
//				bgPixels[index+1]=rgb.rgbGreen;
//				bgPixels[index+2]=rgb.rgbBlue;
//				++bgPixelsNum;	
//			}
//		}
//	}
//
//	if(logText)
//		logText<<"In Cutting:GetCuttingRegion"<<endl;
//
//	GMM::InitGMM(bgPixels, bgPixelsNum, bgCluster->clusterNum, bgCluster->mean, 
//		bgCluster->weight, bgCluster->det, bgCluster->cov);
//
//	GMM::InitGMM(fgPixels, fgPixelsNum, fgCluster->clusterNum, fgCluster->mean, 
//		fgCluster->weight, fgCluster->det, fgCluster->cov);
//
//	GMM::times=1;
//	GMM::GetGMM(bgPixels, bgPixelsNum, bgCluster->clusterNum, bgCluster->mean, 
//		bgCluster->weight, bgCluster->det, bgCluster->cov);
//
//	GMM::GetGMM(fgPixels, fgPixelsNum, fgCluster->clusterNum, fgCluster->mean, 
//		fgCluster->weight, fgCluster->det, fgCluster->cov);
//}

double Cutting::GetTLinkWeight(int x, int y, Cluster *cluster)
{
	int k;
	double prob;
	double data[3];

    GetData(*image, x, y, data);
	for(prob=0,k=0;k<cluster->clusterNum;++k)
	{
        prob+=cluster->weight[k]*
              Prob(data, cluster->mean[k], cluster->det[k], cluster->cov[k]);
	}
	
	return (double)(-log(prob));
}

double Cutting::CalNLinkWeight(double Beta, int x1, int y1, int x2, int y2)
{
	double dist,Z;
	double tmp1,tmp2,tmp3;
	RGBQUAD rgb1,rgb2;
	const double C(50);	

	if(IsValid(x2, y2))
	{
		tmp1=x1-x2;
		tmp2=y1-y2;
		dist=(double)sqrt(tmp1*tmp1+tmp2*tmp2);
		
		rgb1=image->GetPixelColor(x1, y1);
		rgb2=image->GetPixelColor(x2, y2);
		tmp1=rgb1.rgbRed-rgb2.rgbRed;
		tmp2=rgb1.rgbGreen-rgb2.rgbGreen;
		tmp3=rgb1.rgbBlue-rgb2.rgbBlue;
		Z=tmp1*tmp1+tmp2*tmp2+tmp3*tmp3;
		Z=-Beta*Z;
		
		return (double)(C/dist*Got_Exp(Z));
	}
	else
		return -1;
}

double Cutting::GetBeta()
{
	int i,j;
	int k,count;
	int x,y;
	double Z;
	double tmp1,tmp2,tmp3;
	RGBQUAD rgb1,rgb2;
	int direct[4][2]={{1,0},{0,1},{1,1},{1,-1}};
	Z=0;
	count=0;
	for(i=bottom;i<=top;++i)
	{
		for(j=left;j<=right;++j)
		{
			rgb1=image->GetPixelColor(j, i);
			for(k=0;k<4;++k)
			{
				y=i+direct[k][1];
				x=j+direct[k][0];
				if(IsValid(x, y))
				{
					rgb2=image->GetPixelColor(x, y);
					tmp1=rgb1.rgbRed-rgb2.rgbRed;
					tmp2=rgb1.rgbGreen-rgb2.rgbGreen;
					tmp3=rgb1.rgbBlue-rgb2.rgbBlue;
					Z+=tmp1*tmp1+tmp2*tmp2+tmp3*tmp3;
					count++;
				}
			}
		}
	}
	Z/=count;	
	return 0.5/Z;
}
void Cutting::ImageBeta()
{
	beta = GetBeta();
}
void Cutting::InitGraph()
{
	int i,j,k;
	int x1,y1;
	int index1,index2;
	double prob;//,beta;
	double back,fore;
	static const int direct[4][2]={{1,0},{0,1},{1,1},{1,-1}};

	//beta=GetBeta();
	memset(graphPoint, 0, sizeof(void *)*pixelNum);
	for(i=Y[0];i<Y[1];++i)
	{
		for(j=X[0];j<X[1];++j)
		{
			index1=i*width+j;
			for(k=0;k<4;++k)
			{
				x1=j+direct[k][0];
				y1=i+direct[k][1];
				prob=CalNLinkWeight(beta, j, i, x1, y1);
				if(prob!=-1)
				{
					if(graphPoint[index1]==NULL)
					{
						graphPoint[index1]=graph->add_node();
						back=GetTLinkWeight(j, i, bgCluster);
						fore=GetTLinkWeight(j, i, fgCluster);
						graph->set_tweights(graphPoint[index1], (float)fore, (float)back);
					}
					index2=y1*width+x1;
					if(graphPoint[index2]==NULL)
					{
						graphPoint[index2]=graph->add_node();
						back=GetTLinkWeight(x1, y1, bgCluster);
						fore=GetTLinkWeight(x1, y1, fgCluster);
						graph->set_tweights(graphPoint[index2], (float)fore, (float)back);
					}
					graph->add_edge(graphPoint[index1], graphPoint[index2], (float)prob, (float)prob);
				}
			}
		}
	}
}

void Cutting::Update()
{
	int i,j,k;
	int index;
	double back,fore;
	double K=100000000;
	RGBQUAD rgb;

	bgPixelsNum=0;
	fgPixelsNum=0;
	
	//Update GMM
	for(i=bottom;i<=top;++i)
	{
		for(j=left;j<right;++j)
		{
			k=i*width+j;
			rgb=image->GetPixelColor(j, i);
			
			if(graphPoint[k]==NULL)
			{
				//index=3*bgPixelsNum;
				//bgPixels[index]=rgb.rgbRed;
				//bgPixels[index+1]=rgb.rgbGreen;
				//bgPixels[index+2]=rgb.rgbBlue;
				//++bgPixelsNum;
				bgPixels.push_back(rgb.rgbRed);
				bgPixels.push_back(rgb.rgbGreen);
				bgPixels.push_back(rgb.rgbBlue);
			}
			else
			{
				if (graph->what_segment(graphPoint[k])==Graph::SOURCE)
				{
					//index=3*bgPixelsNum;
					//bgPixels[index]=rgb.rgbRed;
					//bgPixels[index+1]=rgb.rgbGreen;
					//bgPixels[index+2]=rgb.rgbBlue;
					//++bgPixelsNum;
					bgPixels.push_back(rgb.rgbRed);
					bgPixels.push_back(rgb.rgbGreen);
					bgPixels.push_back(rgb.rgbBlue);
					label[k]=0;
				}
				else
				{
					//index=3*fgPixelsNum;
					//fgPixels[index]=rgb.rgbRed;
					//fgPixels[index+1]=rgb.rgbGreen;
					//fgPixels[index+2]=rgb.rgbBlue;
					//++fgPixelsNum;
					fgPixels.push_back(rgb.rgbRed);
					fgPixels.push_back(rgb.rgbGreen);
					fgPixels.push_back(rgb.rgbBlue);
					label[k]=1;
				}
			}
		}
	}
	
	GMM::times=1;
	bgPixelsNum = bgPixels.size()/3;
	fgPixelsNum = fgPixels.size()/3;
	GMM::GetGMM(&bgPixels[0], bgPixelsNum, bgCluster->clusterNum, bgCluster->mean, 
		        bgCluster->weight, bgCluster->det, bgCluster->cov);
	
	GMM::GetGMM(&fgPixels[0], fgPixelsNum, fgCluster->clusterNum, fgCluster->mean, 
		        fgCluster->weight, fgCluster->det, fgCluster->cov);
	
	//Update Graph
	for(i=Y[0];i<Y[1];++i)
	{
		for(j=X[0];j<X[1];++j)
		{
			index=i*width+j;
			back=GetTLinkWeight(j, i, bgCluster);
			fore=GetTLinkWeight(j, i, fgCluster);			
			graph->set_tweights(graphPoint[index], (float)fore, (float)back);
		}
	}
	fgPixels.clear();
	bgPixels.clear();
}

void Cutting::Update(CxImage &alpha)
{
     int i,j;
     RGBQUAD rgb;
     double back,fore;
     double K=100000000;

     for(i=Y[0];i<Y[1];++i)
     {
          for(j=X[0];j<X[1];++j)
          {
               rgb=alpha.GetPixelColor(j, i);
               if(rgb.rgbRed==255 && rgb.rgbGreen==255)
                    graph->set_tweights(graphPoint[i*width+j], 0, (float)K);
               else
               if(rgb.rgbRed==255 && rgb.rgbGreen==0)
                    graph->set_tweights(graphPoint[i*width+j], (float)K, 0);
               else
               {
                    back=GetTLinkWeight(j, i, bgCluster);
                    fore=GetTLinkWeight(j, i, fgCluster);			
                    graph->set_tweights(graphPoint[i*width+j], (float)fore, (float)back);
               }
          }
     }

     graph->maxflow();
     Update();
}

void Cutting::Calculate()
{
	int Times=6;
	int i;

	beta=GetBeta();
	InitGraph();
	for(i=0;i<Times;++i)
	{
		graph->maxflow();
		Update();
	}
}

void Cutting::GetResult(CxImage &result)
{
     int i,j,index;
     RGBQUAD rgb1,rgb2;
     result.Copy(*image);
     for(i=0;i<height;++i)
     {
          for(j=0;j<width;++j)
          {
               index=i*width+j;
               if (label[index])
               {
                    rgb2=image->GetPixelColor(j, i);
                    rgb1.rgbRed=rgb2.rgbRed;
                    rgb1.rgbGreen=rgb2.rgbGreen;
                    rgb1.rgbBlue=rgb2.rgbBlue;
                    result.SetPixelColor(j, i, rgb1);
               }
               else
                    result.SetPixelColor(j,i,this->rgb);
          }
     }
}

void Cutting::GetAlphaResult(CxImage &result)
{
     int i,j,index;
     RGBQUAD rgb;
     result.Copy(*image);
     for(i=0;i<height;++i)
     {
          for(j=0;j<width;++j)
          {
               index=i*width+j;
               rgb.rgbRed=label[index]*255;
               rgb.rgbGreen=rgb.rgbRed;
               rgb.rgbBlue=rgb.rgbRed;
               result.SetPixelColor(j, i, rgb);
          }
     }
}

unsigned char *Cutting::GetLabels()
{
     return label;
}

CxImage *Cutting::GetFrame()
{
     return image;
}

void Cutting::SetColor(BYTE r, BYTE g, BYTE b)
{
	this->rgb.rgbRed=r;
	this->rgb.rgbGreen=g;
	this->rgb.rgbBlue=b;
}
