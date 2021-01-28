#include "../VideoEditing/stdafx.h"
#include "Grabcut.h"
#include "cluster.h"
#include <limits>
#include <math.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>
#include <stack>
#include <list>
#include <functional>
#include "BiImageProcess.h"

using namespace std;

Grabcut::Grabcut(void)
{
	component=0;
	alpha=0;
	this->back_rgb.rgbBlue=this->back_rgb.rgbGreen=this->back_rgb.rgbRed=0;
}

Grabcut::~Grabcut(void)
{
	this->RRelease();
}

void Grabcut::RRelease()
{
	if (label)
	{
		delete[] label;
		label = NULL;
	}
	
}
void Grabcut::RReleaseHighLevel()
{
	if (alpha)
	{
		for (int i=0;i<width;++i)
		{
			delete[] alpha[i];
			alpha[i]=NULL;
		}
		delete[] alpha;
		alpha=NULL;
	}
}

void Grabcut::RReleaseLowLevel()
{
	if (component)
	{
		for (int i=0;i<width;++i)
		{
		delete[] component[i];
		component[i]=NULL;
		delete[] triBack[i];
		triBack[i]=NULL;
		delete[] triFor[i];
		triFor[i]=NULL;
		delete[] triUnknown[i];
		triUnknown[i]=NULL;
		delete[] gnode[i];
		gnode[i]=NULL;
		delete[] nweight[i];
		nweight[i]=NULL;
	}
	delete[] component;
	component=NULL;
	delete[] triBack;
	triBack=NULL;
	delete[] triFor;
	triFor=NULL;
	delete[] triUnknown;
	triUnknown=NULL;
	delete[] gnode;
	gnode=NULL;
	delete[] nweight;
	nweight=NULL;
	}
}
void Grabcut::init(CxImage &image,int x1,int y1,int x2,int y2,RECT rect)
{
	double times,timee,timed;
	times = clock();
	this->RReleaseHighLevel();
	cout<<"in init"<<endl;
	cropRect=rect;
	originalImage=image;
	originalImage.Crop(rect.left,image.GetHeight()-rect.top,rect.right,image.GetHeight()-rect.bottom,&sourceImage);

	//sourceImage.Save("E:\\crop.jpg",CXIMAGE_FORMAT_JPG);
	if (sourceImage.GetHeight()>sourceImage.GetWidth())
	{
		scalar=sourceImage.GetHeight()/MIN_WIDTH;
	}
	else
		scalar=sourceImage.GetWidth()/MIN_WIDTH;
	if(scalar==0)
		scalar=1;
	else if (scalar==1)
	{
		colimage.Copy(sourceImage);
	}
	else
	sourceImage.Resample(sourceImage.GetWidth()/scalar,sourceImage.GetHeight()/scalar,2,&colimage);
	cout<<"after crop"<<endl;
	width=colimage.GetWidth();
	height=colimage.GetHeight();

	xstart=x1/scalar;
	ystart=y1/scalar;
	xend=x2/scalar;
	yend=y2/scalar;

	graph=0;

	beta=calBeta();
	cout<<"after crop2"<<endl;


	component=new int*[width];
	alpha=new float *[width];
	
	triBack=new int*[width];
	triFor=new int*[width];
	triUnknown=new int*[width];
	nweight=new NWeight*[width];
	gnode=new Graph::node_id*[width];//node for graph cut
	cout<<"after crop3"<<endl;

	for (int i=0;i<width;i++)
	{
		component[i]=new int[height];
		alpha[i]=new float[height];
		triBack[i]=new int[height];
		triFor[i]=new int[height];
		triUnknown[i]=new int[height];
		gnode[i]=new  Graph::node_id[height];
		nweight[i]=new NWeight[height];
	}
	cout<<"after crop4"<<endl;

	for (int i=0;i<width;++i)
	{
		for (int j=0;j<height;++j)
		{
			component[i][j]=-1;
			triFor[i][j]=0;
			triBack[i][j]=1;
			triUnknown[i][j]=0;
			alpha[i][j]=0;
		}
	}
	cout<<width<<" "<<height<<endl;
	cout<<"after crop5"<<endl;
	cout<<xstart<<" "<<xend<<endl;
	cout<<ystart<<" "<<yend<<endl;
	for (int i=xstart;i<xend&&i<width;++i)//根据用户的rectangle初始化相应数组
	{
		for (int j=ystart;j<yend&&j<height;++j)
		{
			triBack[i][j]=0;
			triUnknown[i][j]=1;
			alpha[i][j]=1;
		}
	}

	cout<<"after crop6"<<endl;
	//finalAlpha=new float *[width*scalar];
	label=NULL;

	cout<<"before cal"<<endl;
	calculateNweight();
	initializeGMM();
	timee = clock();
	timed = (timee-times)/CLK_TCK;
	cout<<"after Init: "<<timed<<endl;

}
void Grabcut::initializeGMM()//initialisation 
{
	//cout<<"in initializeGMM"<<endl;
	vector<colorNode> fortempvec;
	vector<colorNode> backtempvec;
	for (int i=0;i<width;++i)
	{
		for (int j=0;j<height;++j)
		{
			colorNode temp;
			RGBQUAD rgb=colimage.GetPixelColor(i,j,true);
			temp.indexX=i;
			temp.indexY=j;
			temp.colorinfo.resize(3,1);
			temp.colorinfo(0,0)=(double)rgb.rgbRed;
			temp.colorinfo(1,0)=(double)rgb.rgbGreen;
			temp.colorinfo(2,0)=(double)rgb.rgbBlue;
			if (triBack[i][j])
			{
				backtempvec.push_back(temp);
			}
			else if (triUnknown[i][j])
			{
				fortempvec.push_back(temp);
			}
		}
	}
	vector< vector < colorNode > > forvec;
	vector< vector < colorNode > > backvec;
	forvec.push_back(fortempvec);
	backvec.push_back(backtempvec);
	cluster(forvec);
	cluster(backvec);
	initGMM(forvec,fgmm);
	initGMM(backvec,bgmm);
}
void Grabcut::assignGMM()//step 1
{
	//cout<<"assignGMM"<<endl;
	for (int i=0;i<width;++i)
	{
		for (int j=0;j<height;++j)
		{
			double poss=-1;
				int minindex=-1;
				for (int k=0;k<COMNUM;++k)
				{
					double tempposs=calposs(alpha[i][j],k,colimage.GetPixelColor(i,j));
					if (tempposs>poss)
					{
						poss=tempposs;
						minindex=k;
					}
				}
				component[i][j]=minindex;
		}
	}
}
void Grabcut::assignGMM2()//step 1
{
	for (int i=xstart-3;i<xend+3;++i)
	{
		for (int j=ystart-3;j<yend+3;++j)
		{
			if(i>=0&&i<width&&j>=0&&j<height)
			if (triUnknown[i][j])
			{
		
				double poss=-1;
				int minindex=-1;
				for (int k=0;k<COMNUM;++k)
				{
					double tempposs=calposs(alpha[i][j],k,colimage.GetPixelColor(i,j));
					if (tempposs>poss)
					{
						poss=tempposs;
						minindex=k;
					}
				}
				component[i][j]=minindex;

		
			}
	
		}
	}
}
void Grabcut::recalGMM()//step 2
{
	int forcount=0;
	int backcount=0;
	vector<pixelNode> *fpvec=new vector<pixelNode>[COMNUM];
	vector<pixelNode> *bpvec=new vector<pixelNode>[COMNUM];
	for (int i=0;i<width;++i)
	{
		for (int j=0;j<height;++j)
		{
			pixelNode temp;
			temp.x=i;
			temp.y=j;
			if (alpha[i][j])//froground
			{
				fpvec[component[i][j]].push_back(temp);
				forcount++;
			}
			else//background
			{
				bpvec[component[i][j]].push_back(temp);
				backcount++;
			}
		}
	}
	for (int k=0;k<COMNUM;++k)
	{
		calGMMPara( fgmm[k],fpvec[k],forcount,colimage);
		calGMMPara(bgmm[k],bpvec[k],backcount,colimage);
	}
	delete[] fpvec;
	delete[] bpvec;
}
void Grabcut::recalGMM2()//step 2
{
	int forcount=0;
	int backcount=0;
    vector<pixelNode> *fpvec=new vector<pixelNode>[COMNUM];
	vector<pixelNode> *bpvec=new vector<pixelNode>[COMNUM];
	for (int i=0;i<width;++i)
	{
		for (int j=0;j<height;++j)
		{
			pixelNode temp;
			temp.x=i;
			temp.y=j;
			if (alpha[i][j])//froground
			{
				fpvec[component[i][j]].push_back(temp);
				forcount++;
			}
		}
	}
	for (int k=0;k<5;++k)
	{
		calGMMPara( fgmm[k],fpvec[k],forcount,colimage);
	}

	delete[] fpvec;
	delete[] bpvec;
}
double Grabcut::calposs(double alpha, int gmmindex,RGBQUAD rgb)
{
	int z[3];
	//RGBQUAD rgb=colimage.GetPixelColor(x,y,true);
	z[0]=(double)rgb.rgbRed;
        z[1]=(double)rgb.rgbGreen;
	z[2]=(double)rgb.rgbBlue;

	double temp[3];
	
	double temp2[3];

	if (alpha)
	{
		temp[0]=z[0]-fgmm[gmmindex].u(0,0);
		temp[1]=z[1]-fgmm[gmmindex].u(1,0);
		temp[2]=z[2]-fgmm[gmmindex].u(2,0);

		

		temp2[0]=fgmm[gmmindex].inverseConviance(0,0)*temp[0]+fgmm[gmmindex].inverseConviance(0,1)*temp[1]+fgmm[gmmindex].inverseConviance(0,2)*temp[2];
		temp2[1]=fgmm[gmmindex].inverseConviance(1,0)*temp[0]+fgmm[gmmindex].inverseConviance(1,1)*temp[1]+fgmm[gmmindex].inverseConviance(1,2)*temp[2];
		temp2[2]=fgmm[gmmindex].inverseConviance(2,0)*temp[0]+fgmm[gmmindex].inverseConviance(2,1)*temp[1]+fgmm[gmmindex].inverseConviance(2,2)*temp[2];


		if (fgmm[gmmindex].pi>0)
		{
			if (fgmm[gmmindex].det>0)
			{
				double mi=0.5*(temp[0]*temp2[0]+temp[1]*temp2[1]+temp[2]*temp2[2]);
			
				double leftr=sqrt(abs(fgmm[gmmindex].det))/fgmm[gmmindex].pi;
				return (1.0/(sqrt(abs(fgmm[gmmindex].det)))*Got_Exp(-mi));
				//return Got_Exp(-0.5*mi/(sqrt(abs(fgmm[gmmindex].det))));
			}
			return 0;

		}
	}
	else
	{
		temp[0]=z[0]-bgmm[gmmindex].u(0,0);
		temp[1]=z[1]-bgmm[gmmindex].u(1,0);
		temp[2]=z[2]-bgmm[gmmindex].u(2,0);



		temp2[0]=bgmm[gmmindex].inverseConviance(0,0)*temp[0]+bgmm[gmmindex].inverseConviance(0,1)*temp[1]+bgmm[gmmindex].inverseConviance(0,2)*temp[2];
		temp2[1]=bgmm[gmmindex].inverseConviance(1,0)*temp[0]+bgmm[gmmindex].inverseConviance(1,1)*temp[1]+bgmm[gmmindex].inverseConviance(1,2)*temp[2];
		temp2[2]=bgmm[gmmindex].inverseConviance(2,0)*temp[0]+bgmm[gmmindex].inverseConviance(2,1)*temp[1]+bgmm[gmmindex].inverseConviance(2,2)*temp[2];


		if (bgmm[gmmindex].pi>0)
		{
			if (bgmm[gmmindex].det>0)
			{
				double mi=0.5*(temp[0]*temp2[0]+temp[1]*temp2[1]+temp[2]*temp2[2]);
			
				double leftr=sqrt(abs(bgmm[gmmindex].det))/bgmm[gmmindex].pi;
				return (1.0/(sqrt(abs(bgmm[gmmindex].det)))*Got_Exp(-mi));

				//return Got_Exp(-0.5*mi/(sqrt(abs(bgmm[gmmindex].det))));
			}
			return 0;
		}
	}
	return 0;
}
void Grabcut::constructgraph()//construct for graph cut
{
		graph=new Graph();
		for (int i=0;i<width;++i)
		{
			for (int j=0;j<height;++j)
			{
				gnode[i][j]=graph->add_node();
			}
		}
		double startT,endT,duration=0;
		//calculate T
		startT=clock();
		for (int i=0;i<width;++i)
		{
			for (int j=0;j<height;++j)
			{
				double forposs=0.0;
				double backposs=0.0;
				if (triBack[i][j])//background
				{
					forposs=0;
					backposs=4501;//unsure
					
				}
				else if (triFor[i][j])//forground
				{
					backposs=0;
					forposs=4501;//unsure
					
				}
				else//unknown
				{
					for (int itemp=0;itemp<COMNUM;itemp++)
					{
						if (bgmm[itemp].count)
						{
							backposs+=bgmm[itemp].pi*calposs(0,itemp,colimage.GetPixelColor(i,j));
							//cout<<"PI: "<<bgmm[itemp].pi<<endl;
						}
						else
							continue;	
					}
					//cout<<"backposs:"<<backposs<<endl;
					backposs=log(backposs);
					for (int jtemp=0;jtemp<COMNUM;jtemp++)
					{
						if (fgmm[jtemp].count)
						{
							forposs+=fgmm[jtemp].pi*calposs(1,jtemp,colimage.GetPixelColor(i,j));			
						}
						else
							continue;	
					}
					forposs=log(forposs);
				}
				//logText<<"for: "<<forposs<<" back: "<<backposs<<endl;
				graph->add_tweights(gnode[i][j],forposs,backposs);
			}
		}
		endT=clock();
		duration=(endT-startT)/CLK_TCK;
		cout<<"cons T time:"<<duration<<endl;

		//calculate Neighbour
		startT=clock();
		for (int i=0;i<width;++i)
		{
			for (int j=0;j<height;++j)
			{
				if (i>=0&&j+1<height)
				{
					graph->add_edge(gnode[i][j],gnode[i][j+1],nweight[i][j].up*0.3,nweight[i][j].up*0.3);
				}
				if (i>=0&&j-1>=0)
				{
					graph->add_edge(gnode[i][j],gnode[i][j-1],nweight[i][j].down*0.3,nweight[i][j].down*0.3);
				}
				if (j<height&&i-1>=0)
				{
					graph->add_edge(gnode[i][j],gnode[i-1][j],nweight[i][j].left*0.3,nweight[i][j].left*0.3);
				}
				if (j<height&&i+1<width)
				{
					graph->add_edge(gnode[i][j],gnode[i+1][j],nweight[i][j].right*0.3,nweight[i][j].right*0.3);
				}
			}
		}
		endT=clock();
		duration=(endT-startT)/CLK_TCK;
		cout<<"con N time: "<<duration<<endl;
}
void Grabcut::calculateNweight()
{

	for (int i=0;i<width;++i)
	{
		for (int j=0;j<height;++j)
		{
			if (i>=0&&j+1<height)
			{
				double distanceE=distance(i,j,i,j+1);
				double discol=coldistance(colimage.GetPixelColor(i,j,true),colimage.GetPixelColor(i,j+1,true));
				nweight[i][j].up=calN(distanceE,discol);
			}
			if (i>=0&&j-1>=0)
			{
				double distanceE=distance(i,j,i,j-1);
				double discol=coldistance(colimage.GetPixelColor(i,j,true),colimage.GetPixelColor(i,j-1,true));
				nweight[i][j].down=calN(distanceE,discol);
			}
			if (i-1>=0&&j<height)
			{
				double distanceE=distance(i,j,i-1,j);
				double discol=coldistance(colimage.GetPixelColor(i,j,true),colimage.GetPixelColor(i-1,j,true));
				nweight[i][j].left=calN(distanceE,discol);
			}
			if (i+1<width&&j<height)
			{
				double distanceE=distance(i,j,i+1,j);
				double discol=coldistance(colimage.GetPixelColor(i,j,true),colimage.GetPixelColor(i+1,j,true));
				nweight[i][j].right=calN(distanceE,discol);
			}
		}
	}
}
double Grabcut::calN(double distance, double coldis)
{
	return 50*Got_Exp(-beta*coldis)/distance;
}
double Grabcut::distance(int x1,int y1,int x2,int y2)
{
	double dis=(x1-x2)*(x1-x2)+(y1-y2)*(y1-y2);
	return sqrt(dis);
}
double  Grabcut::coldistance(RGBQUAD c1,RGBQUAD c2)
{
	return (c1.rgbRed-c2.rgbRed)*(c1.rgbRed-c2.rgbRed)+(c1.rgbGreen-c2.rgbGreen)*(c1.rgbGreen-c2.rgbGreen)+(c1.rgbBlue-c2.rgbBlue)*(c1.rgbBlue-c2.rgbBlue);
}
double Grabcut::calBeta()
{
	double total=0.0;
	int num=0;
	for (int i=0;i<width;++i)
	{
		for (int j=0;j<height;++j)
		{
			if (i>=0&&j+1<height)
			{
				total+=coldistance(colimage.GetPixelColor(i,j,true),colimage.GetPixelColor(i,j+1));
				num++;
			}
			if (i>=0&&j-1>=0)
			{
				total+=coldistance(colimage.GetPixelColor(i,j,true),colimage.GetPixelColor(i,j-1));
				num++;
			}
			if (i-1>=0&&j<height)
			{
				total+=coldistance(colimage.GetPixelColor(i,j,true),colimage.GetPixelColor(i-1,j));
				num++;
			}
			if (i+1<width&&j<height)
			{
				total+=coldistance(colimage.GetPixelColor(i,j,true),colimage.GetPixelColor(i+1,j));
				num++;
			}
		}
	}
	return (1.0)/(2*(total/num));
}

int Grabcut::dosegonce(int num)
{
	double starT,endT,duration;
	if (first)
	{
		starT=clock();

		assignGMM();
		recalGMM();
		first=false;

		endT=clock();
		duration=(endT-starT)/CLK_TCK;
		cout<<"GMM time:"<<duration<<endl;
	}
	else
	{
		starT=clock();
		assignGMM();
		recalGMM();
		endT=clock();
		duration=(endT-starT)/CLK_TCK;
		cout<<"GMM Time: "<<duration<<endl;
	}

	starT=clock();
	constructgraph();
	endT=clock();
	duration=(endT-starT)/CLK_TCK;
	cout<<"construction graph time:"<<duration<<endl;

	starT=clock();
	double maxflow;
	if (graph)
	{
		maxflow=graph->maxflow();
	}
	int changed=0;
	for (int i=xstart;i<xend&&i<width;i++)
	{
		for (int j=ystart;j<yend&&j<height;j++)
		{
		    double fb=alpha[i][j];
			if (graph->what_segment(gnode[i][j])==Graph::SOURCE)
			{
				alpha[i][j]=1;
				//triUnknown[i][j]=1;
			}
			else
			{
				alpha[i][j]=0;
				//triUnknown[i][j]=0;
			}
			if (fb!=alpha[i][j])
			{
				changed++;
			}
		}
	}
	if (graph)
		delete graph;
	
	endT=clock();
	duration=(endT-starT)/CLK_TCK;
	cout<<"other time: "<<duration<<endl;
	//save(num);
	return changed; 
}
void Grabcut::doseg()
{
	cout<<"in doseg"<<endl;
	double times, timee, timed;
	times = clock();
	int ch=555;
	int num=0;
	int nnn=10;
	while (ch&&nnn>0)
	{
		ch=dosegonce(num);
		num++;
		nnn--;
	}
	timee = clock();
	timed = (timee-times)/CLK_TCK;
	std::cout<<"==========cut time: "<<timed<<std::endl;
	times = clock();
	CxImage templow;
	templow.Create(width, height, 24);
	for (int i=0; i<width; ++i)
	{
		for (int j=0; j<height; ++j)
		{
			templow.SetPixelColor(i,j, RGB(alpha[i][j]*255,alpha[i][j]*255,alpha[i][j]*255));
		}
	}
	templow.Save("H:/temp_low.bmp", CXIMAGE_FORMAT_BMP);
	if(scalar!=1)
	{
		RReleaseLowLevel();
		highLevelCut();
	}
	timee = clock();
	timed = (timee-times)/CLK_TCK;
	if(scalar==1)
	{		
		finalAlpha=alpha;
		SetLabel();
		checkConnectivity(alpha,width,height);
	}
	std::cout<<"===========highlivel time: "<<timed<<std::endl;
	RReleaseHighLevel();
	//save(num);	
}

void Grabcut::setForAlpha(int x,int y)
{
	if (x>cropRect.left&&x<cropRect.right&&y>cropRect.bottom&&y<cropRect.top)
	{
		if ((x-cropRect.left)/scalar<width&&(y-cropRect.bottom)/scalar<height)
		{
			alpha[(x-cropRect.left)/scalar][(y-cropRect.bottom)/scalar]=1;
		}
		
		if(scalar!=1)
		{
			finalAlpha[x-cropRect.left][y-cropRect.bottom]=1;
		}
	}
	if (y<originalImage.GetHeight()&&x<originalImage.GetWidth())
	{
		label[y*originalImage.GetWidth()+x]=1;
	}
}

void Grabcut::setForTrip(int x, int y)
{
	if (x>cropRect.left&&x<cropRect.right&&y>cropRect.bottom&&y<cropRect.top)
		if ((x-cropRect.left)/scalar<width&&(y-cropRect.bottom)/scalar<height)
		{
			triFor[(x-cropRect.left)/scalar][(y-cropRect.bottom)/scalar]=1;
		}		
}

void Grabcut::setBackAlpha(int x,int y)
{

		if (x>cropRect.left&&x<cropRect.right&&y>cropRect.bottom&&y<cropRect.top)
		{

			if ((x-cropRect.left)/scalar<width&&(y-cropRect.bottom)/scalar<height)
			{
				alpha[(x-cropRect.left)/scalar][(y-cropRect.bottom)/scalar]=0;
			}
			
			if (scalar!=1)
			{
				finalAlpha[x-cropRect.left][y-cropRect.bottom]=0;
			}
			

		}
		if (y<originalImage.GetHeight()&&x<originalImage.GetWidth())
		{
			label[y*originalImage.GetWidth()+x]=0;
		}

}

void Grabcut::setBackTrip(int x,int y)
{
		if (x>cropRect.left&&x<cropRect.right&&y>cropRect.bottom&&y<cropRect.top)
			if ((x-cropRect.left)/scalar<width&&(y-cropRect.bottom)/scalar<height)
			{
				triBack[(x-cropRect.left)/scalar][(y-cropRect.bottom)/scalar]=1;
			}
}

void Grabcut::GetResult(CxImage &result)
{
	result.Create(originalImage.GetWidth(), originalImage.GetHeight(),24);
	RGBQUAD rgba;
	int w,h,index;
	w=result.GetWidth();
	h=result.GetHeight();
	for (int i=0;i<h;++i)
	{
		for (int j=0;j<w;++j)
		{
			index=i*w+j;
			rgba=originalImage.GetPixelColor(j,i);

			rgba.rgbBlue=label[index]*rgba.rgbBlue+(1-label[index])*back_rgb.rgbBlue;
			rgba.rgbGreen=label[index]*rgba.rgbGreen+(1-label[index])*back_rgb.rgbGreen;
			rgba.rgbRed=label[index]*rgba.rgbRed+(1-label[index])*back_rgb.rgbRed;

			result.SetPixelColor(j,i,rgba);
		}
	}
}

void Grabcut::SetColor(BYTE r,BYTE g,BYTE b)
{
	this->back_rgb.rgbBlue=b;
	this->back_rgb.rgbGreen=g;
	this->back_rgb.rgbRed=r;
}

void Grabcut::GetAlphaResult(CxImage &result)
{
	//result.Copy(originalImage);
	result.Create(originalImage.GetWidth(), originalImage.GetHeight(), 8);
	result.SetGrayPalette();
	result.Clear();
	RGBQUAD rgba;
	for (int i=0;i<originalImage.GetHeight();++i)
	{
		for (int j=0;j<originalImage.GetWidth();++j)
		{
			//rgba.rgbBlue=label[i*originalImage.GetWidth()+j]*255;
			result.SetPixelIndex(j,i,label[i*originalImage.GetWidth()+j]*255);
			//result.SetPixelColor(j,i,rgba);
		}
	}
}

CxImage* Grabcut::GetFrame()
{
	return &originalImage;
}

void Grabcut::SetImage(CxImage *image)
{
	originalImage=*image;
}

void Grabcut::UpdateLabel(CxImage &alpha, CxImage &scrib)
{
	int i,j;
	RGBQUAD rgb;

	int w=originalImage.GetWidth();
	int h=originalImage.GetHeight();


	if (this->label==NULL)
	{
		int pixelnums=h*w;

		this->label=new unsigned char[pixelnums];
		memset(this->label,0,sizeof(unsigned char)*pixelnums);
	}

	for(i=0;i<h;++i)
	{
		for(j=0;j<w;++j)
		{            
			rgb=alpha.GetPixelColor(j, i);
			if(rgb.rgbRed==0)
				label[i*w+j]=0;
			else
				if(rgb.rgbRed==255)
					label[i*w+j]=1;
		}
	}

	for(i=0;i<h;++i)
	{
		for(j=0;j<w;++j)
		{
			rgb=scrib.GetPixelColor(j, i);
			if(rgb.rgbRed==0)
				label[i*w+j]=0;
			else
				if(rgb.rgbRed==255)
					label[i*w+j]=1;
		}
	}
}

void Grabcut::UpdateLabel(CxImage &scrib)
{
	int i,j;
	int w=originalImage.GetWidth();
	int h=originalImage.GetHeight();
	RGBQUAD rgb;
	for(i=0;i<h;++i)
	{
		for(j=0;j<w;++j)
		{
			rgb=scrib.GetPixelColor(j, i);
			if(rgb.rgbRed==0)
				label[i*w+j]=0;
			else
				if(rgb.rgbRed==255)
					label[i*w+j]=1;
		}
	}
}
void Grabcut::SetLabel()
{
	if (this->label==NULL)
	{
		int pixelnums=originalImage.GetHeight()*originalImage.GetWidth();

		this->label=new unsigned char[pixelnums];
		memset(this->label,0,sizeof(unsigned char)*pixelnums);
	}

	int index=0;
	for (int i=0;i<height*scalar;++i)
	{
		for (int j=0;j<width*scalar;++j)
		{
			index=(i+cropRect.bottom)*originalImage.GetWidth()+j+cropRect.left;
			this->label[index]=finalAlpha[j][i];
		}
	}

	BiImageProcess::checkConnectivityfast(label, originalImage.GetWidth(), originalImage.GetHeight());
	CxImage tem;
	tem.Copy(originalImage);
	tem.Clear();
	for (int i=0;i<originalImage.GetWidth();++i)
	{
		for (int j=0;j<originalImage.GetHeight();++j)
		{
			RGBQUAD r;
			r.rgbBlue=label[j*originalImage.GetWidth()+i]*255;
			r.rgbGreen=r.rgbBlue;
			r.rgbRed=r.rgbBlue;
			tem.SetPixelColor(i,j,r);
		}
	}
	//tem.Save("H:\\tem.jpg",CXIMAGE_FORMAT_JPG);

}

void Grabcut::SetLabel(unsigned char *label)
{
	int index;
	for (int i=0;i<originalImage.GetHeight();++i)
	{
		for (int j=0;j<originalImage.GetWidth();++j)
		{
			index=i*originalImage.GetWidth()+j;
			this->label[index]=label[index];
		}
	}

	checkConnectivity(label,originalImage.GetWidth(),originalImage.GetHeight());
}

//void Grabcut::GetTrimap(CxImage &trimap,int dis)
//{
//	trimap.Copy(originalImage);
//	int w=originalImage.GetWidth();
//	int h=originalImage.GetHeight();
//	double *tag=new double[w*h];
//	memset(tag,0,sizeof(double)*w*h);
//
//	int index;
//	for (int j=0;j<h;++j)
//	{
//		for (int i=0;i<w;++i)
//		{
//			index=j*w+i;
//			tag[index]=this->label[index];
//
//		}
//	}
//
//	BiImageProcess::GetTrimap(this->label,tag,dis,w,h);
//
//	RGBQUAD r;
//	for (int i=0;i<w;++i)
//	{
//		for (int j=0;j<h;++j)
//		{
//			if (tag[j*w+i]==0.5)
//			{
//				r.rgbBlue=0.5*256;
//				r.rgbGreen=r.rgbRed=r.rgbBlue;
//			}
//			else if(tag[j*w+i]==0)
//			{
//				r.rgbBlue=r.rgbGreen=r.rgbRed=0;
//			}
//
//			else
//				r.rgbBlue=r.rgbGreen=r.rgbRed=255;
//
//			trimap.SetPixelColor(i,j,r);
//
//		}
//	}
//	delete[] tag;
//}
unsigned char* Grabcut::GetLabels()
{
	return label;
}


//test
void Grabcut::save(int num)
{
	CxImage ii2;
	CxImage ii3;
	CxImage ii4;
	
	ii2.Copy(colimage);
	ii3.Copy(colimage);
	ii3.Resample(width*scalar,height*scalar,2,&ii4);
	for (int i=0;i<width;++i)
	{
		for (int j=0;j<height;++j)
		{
			RGBQUAD rgb;
			rgb.rgbRed=(BYTE)alpha[i][j]*255;
			rgb.rgbGreen=(BYTE)alpha[i][j]*255;
			rgb.rgbBlue=(BYTE)alpha[i][j]*255;
			rgb.rgbReserved=(BYTE)0;
			ii2.SetPixelColor(i,j,rgb);
		}
	}

	//for (int i=0;i<width*scalar;++i)
	//{
	//	for (int j=0;j<height*scalar;++j)
	//	{
	//		RGBQUAD rgb;
	//		rgb.rgbRed=(BYTE)finalAlpha[i][j]*255;
	//		rgb.rgbGreen=(BYTE)finalAlpha[i][j]*255;
	//		rgb.rgbBlue=(BYTE)finalAlpha[i][j]*255;
	//		rgb.rgbReserved=(BYTE)0;
	//		ii4.SetPixelColor(i,j,rgb);
	//	}
	//}

	CxImage ii;
	ii.Create(width*scalar,height*scalar,24);
	ii.Clear();
	//for (int i=0;i<width*scalar;++i)
	//{
	//	for (int j=0;j<height*scalar;++j)
	//	{
	//		
	//		RGBQUAD rgb;
	//		rgb.rgbRed=(BYTE)(tag[i][j]*255);
	//		rgb.rgbGreen=(BYTE)(tag[i][j]*255);
	//		rgb.rgbBlue=(BYTE)(tag[i][j]*255);
	//		rgb.rgbReserved=(BYTE)0;
	//		ii.SetPixelColor(i,j,rgb);
	//	
	//	}
	//}

	std::ostringstream stm;
	stm<<num;
	std::string path2;
	path2=stm.str();
	std::string path="D:\\tag";
	path+=path2;
	path+=".png";
	std::string pah3="D:\\alpha";
	pah3+=("s"+path2);
	pah3+=".bmp";
	ii.Save(path.c_str(),CXIMAGE_FORMAT_PNG);
	ii3.Save("D:\\a.jpg",CXIMAGE_FORMAT_PNG);
	ii2.Save(pah3.c_str(),CXIMAGE_FORMAT_PNG);
	string path4="D:\\finalalpha";
	path4+=path2;
	path4+=".png";
	ii4.Save(path4.c_str(),CXIMAGE_FORMAT_PNG);
	colimage.Save("D:\\b.jpg",CXIMAGE_FORMAT_JPG);

}