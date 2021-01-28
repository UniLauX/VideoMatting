#include "MultiFrameRefine.h"
extern "C"
#include "../Opticalflow/calflow.h"

#define MULTI_GMM

CMultiFrameRefine::CMultiFrameRefine(void)
:width(0)
,height(0)
,rrange(0)
,gmmnum(3)
{
}

CMultiFrameRefine::~CMultiFrameRefine(void)
{
}

void CMultiFrameRefine::Refine(ImageBuffer* alphaBuffer,ImageBuffer* imageBuffer,ImageBuffer* trimapBuffer,int refineRange,int winSize)
{
	//首先建立窗口，针对每一个小窗口和序列中对应的窗口建立树即索引

	int pos=alphaBuffer->GetFramePos();//prepare to restore buffer

	//get all window centers
	CxImage label;
	VVPOINTS vec;//window centers
	label.Copy(*(alphaBuffer->GetImage()));

	rrange=refineRange;
	width=label.GetWidth();
	height=label.GetHeight();

	int *ll=new int[width*height];

	for (int j=0;j<height;++j)
	{
		for (int i=0;i<width;++i)
		{
			ll[j*width+i]=label.GetPixelGray(i,j)/255;
		}
	}
	BiImageProcess::GetAllWindowsCenters(vec,ll,width,height,2*winSize+1);
	delete[] ll;
	//calculate Optical flow
	float **u;
	float **v;
      //remember free
	u=new float *[refineRange-1];
	v=new float *[refineRange-1];

	float **u2;
	float **v2;

	u2=new float *[refineRange-1];
	v2=new float *[refineRange-1];


	for (int i=0;i<refineRange-1;++i)
	{
		u[i]=new float[width*height];
		v[i]=new float[width*height];

		u2[i]=new float[width*height];
		v2[i]=new float[width*height];

		memset(u[i],0,sizeof(float)*width*height);
		memset(v[i],0,sizeof(float)*width*height);

		memset(u2[i],0,sizeof(float)*width*height);
		memset(v2[i],0,sizeof(float)*width*height);
	}

	CxImage frame1,frame2;
	for (int i=0;i<refineRange-1;++i)
	{
		frame1.Copy(*(imageBuffer->GetImage()));
		bool next=imageBuffer->Forward();

		if (next)
		{
			frame2.Copy(*(imageBuffer->GetImage()));
			CalOpticalFlow(frame1,frame2,u[i],v[i]);
			CalOpticalFlow(frame2,frame1,u2[i],v2[i]);
		}
		
		else
			break;
	}
	imageBuffer->Goto(pos);//restore buffer
	
	cout<<"cal optical flow over"<<endl;

	//test window position

	//BiImageProcess::VPOINTS pointtemp;
	//frame1.Copy(*(alphaBuffer->GetImage()));
	//RGBQUAD rgbredtemp;
	//rgbredtemp.rgbBlue=rgbredtemp.rgbGreen=0;
	//rgbredtemp.rgbRed=255;
	//RGBQUAD rgbbuletemp;
	//rgbbuletemp.rgbBlue=255;
	//rgbbuletemp.rgbGreen=rgbbuletemp.rgbRed=0;
	//for (int ii=0;ii<vec.size();++ii)
	//{
	//	for (int j=0;j<vec[ii].size();++j)
	//	{
	//		pointtemp.push_back(vec[ii][j]);
	//		frame1.SetPixelColor(vec[ii][j].x,vec[ii][j].y,rgbredtemp);
	//	}
	//}
	//frame1.Save(alphaBuffer->GetFrameName(),CXIMAGE_FORMAT_BMP);

	//for (int i=0;i<refineRange-1;++i)
	//{
	//	alphaBuffer->Forward();
	//	frame1.Copy(*(alphaBuffer->GetImage()));

	//	for (int j=0;j<pointtemp.size();++j)
	//	{
	//		int x,y;
	//		x=pointtemp[j].x+u[i][(pointtemp[j].y*width)+pointtemp[j].x];
	//		y=pointtemp[j].y+v[i][(pointtemp[j].y*width)+pointtemp[j].x];
	//		pointtemp[j].x=x;
	//		pointtemp[j].y=y;
	//		
	//	
	//		frame1.SetPixelColor(x,y,rgbredtemp);

	//		for (int xt=x-winSize;xt<=x+winSize;++xt)
	//		{
	//			frame1.SetPixelColor(xt,y+winSize,rgbbuletemp);
	//			frame1.SetPixelColor(xt,y-winSize,rgbbuletemp);
	//		}
	//		for (int yt=y-winSize;yt<=y+winSize;++yt)
	//		{
	//			frame1.SetPixelColor(x+winSize,yt,rgbbuletemp);
	//			frame1.SetPixelColor(x-winSize,yt,rgbbuletemp);
	//		}
	//		
	//	}

	//	frame1.Save(alphaBuffer->GetFrameName(),CXIMAGE_FORMAT_BMP);
	//}
	//test window position

	for (int i=0;i<vec.size();++i)
	{
		for (int j=0;j<vec[i].size();++j)
		{
			//initialize index
			int **index;
			//remember free
			index=new int *[refineRange];
			for (int itemp=0;itemp<refineRange;++itemp)
			{
				index[itemp]=new int[(2*winSize+1)*(2*winSize+1)];
				for (int jtemp=0;jtemp<(2*winSize+1)*(2*winSize+1);++jtemp)
				{
					index[itemp][jtemp]=-1;
				}
			}// end of initilalize index

			cout<<"index over"<<endl;
			int x=vec[i][j].x;
			int y=vec[i][j].y;

			Graph *graph;
			graph=new Graph;

			Graph::node_id *gnode;
			gnode=new Graph::node_id[2*width*height];

		   for(int itemp=0;itemp<2*width*height;++itemp)
			{
				gnode[itemp]=graph->add_node();
			}

			//localRefine
			cout<<"before local refine"<<endl;
			LocalRefine(x,y,graph,gnode,u,v,index,winSize,alphaBuffer,imageBuffer);
			cout<<"local refine over"<<endl;

			alphaBuffer->Goto(pos);
			graph->maxflow();

			RGBQUAD w;
			w.rgbBlue=w.rgbRed=255;
			w.rgbGreen=255;
			RGBQUAD b;
			b.rgbBlue=0;
			b.rgbGreen=b.rgbRed=0;

			cout<<"before for rr"<<endl;
			for (int il=0;il<rrange;++il)
			{
				CxImage labell;
				labell.Copy(*(alphaBuffer->GetImage()));

				int xmin=x-winSize;
				int xmax=x+winSize;
				int ymin=y-winSize;
				int ymax=y+winSize; 

				for (int xx=x-winSize;xx<=x+winSize;++xx)
				{
					for(int yy=y-winSize;yy<=y+winSize;++yy)
					{
						int ind=(yy-ymin)*(2*winSize+1)+xx-xmin;
						if (index[il][ind]!=-1)
						{
							if (graph->what_segment(gnode[index[il][ind]])==Graph::SOURCE)
								labell.SetPixelColor(xx,yy,w);
							else
								labell.SetPixelColor(xx,yy,b);
						}
					}
				}

				labell.Save(alphaBuffer->GetFrameName(),CXIMAGE_FORMAT_BMP);

				if (il!=rrange-1)
				{
					x=x+u[il][y*width+x];
					y=y+v[il][y*width+x];
				}
				
				alphaBuffer->Forward();

			}
			cout<<"after for rr"<<endl;
			alphaBuffer->Goto(pos);
			imageBuffer->Goto(pos);


			delete graph;
			delete[] gnode;

			for (int it=0;it<refineRange;++it)
			{
				delete[] index[it];
				index[it]=NULL;
			}
			delete[] index;
			index=NULL;
		}
	}
}

void CMultiFrameRefine::CalOpticalFlow(const CxImage &curr,const CxImage &next,float* u,float* v)
{
	CxImage image1,image2;
	image1.Copy(curr);
	image2.Copy(next);

	float* bf1;
	float* bf2;
	int w,h;
	w=image1.GetWidth();
	h=image1.GetHeight();

	bf1=(float*)malloc(sizeof(float)*w*h);
	bf2=(float*)malloc(sizeof(float)*w*h);

	int count=0;
	for (int j=0;j<h;++j)
	{
		for (int i=0;i<w;++i)
		{
			BYTE temp1=image1.GetPixelGray(i,j);
			BYTE temp2=image2.GetPixelGray(i,j);

			bf1[count]=temp1;
			bf2[count]=temp2;
			count++;

		}
	}
	calFlow(bf1,bf2,u,v,0,1,4,1,w,h);
	free(bf1);
	free(bf2);
}

void CMultiFrameRefine::LocalRefine(int x,int y,Graph *graph, Graph::node_id *gnode, float **u, float **v, int **index, int winSize, ImageBuffer *alphaBuffer, ImageBuffer *imageBuffer)
{
	//prepare to construct graph
	int count=0;
	for (int f=0;f<rrange;++f)
	{
		//current gmm 
		CxImage frame1;
		frame1.Copy(*(imageBuffer->GetImage()));
		CxImage label;
		label.Copy(*(alphaBuffer->GetImage()));
		
		GMMNode fgmm[3];
		GMMNode bgmm[3];
		SampleGMM(fgmm,bgmm,x,y,winSize,frame1,label);	
		//current gmm
		GMMNode nfgmm[3];//gmm of next frame
		GMMNode nbgmm[3];

		cout<<"for time:"<<f<<endl;
		CxImage curr;
		curr.Copy(*(imageBuffer->GetImage()));
		CxImage next;
		bool nex=imageBuffer->Forward();
		if (nex)
			next.Copy(*(imageBuffer->GetImage()));
		CxImage currLabel;
		currLabel.Copy(*(alphaBuffer->GetImage()));
		CxImage nextLabel;
		bool nextl=alphaBuffer->Forward();
		if (nextl)
			nextLabel.Copy(*(alphaBuffer->GetImage()));
		int xn,yn;
		if (f!=rrange-1)
		{
			xn=x+u[f][y*width+x];
			yn=y+v[f][y*width+x];
			//next frame gmm
			SampleGMM(nfgmm,nbgmm,xn,yn,winSize,next,nextLabel);
			//next frame gmm
		}

		int xmin=x-winSize;
		int xmax=x+winSize;
		int ymin=y-winSize;
		int ymax=y+winSize;

		this->beta=GetBeta(&curr,x,y,winSize);
		cout<<"before widow graph"<<endl;
		for (int i=x-winSize;i<=x+winSize;++i)
		{
			for (int j=y-winSize;j<=y+winSize;++j)
			{
				RGBQUAD rgb=curr.GetPixelColor(i,j);

				if (index[f][(j-ymin)*(2*winSize+1)+i-xmin]==-1)//data cost
				{
					float forposs,backposs;
					RGBQUAD lrgb=currLabel.GetPixelColor(i,j);
					if (i==xmax||i==xmin||j==ymin||j==ymax)
					{
						if (lrgb.rgbBlue==255)
						{
							forposs=4500;
							backposs=0;
						}
						else
						{
							forposs=0;
							backposs=4500;
						}
					}
					else
					{
						forposs=CalForTWeight(fgmm,rgb);
						backposs=CalBackTWeight(bgmm,rgb);
					}

					graph->set_tweights(gnode[count],forposs,backposs);

					index[f][(j-ymin)*(2*winSize+1)+i-xmin]=count;
					count++;
				}
				//neighbourhood
				if (i-1>=xmin)
				{
					RGBQUAD rgb2=curr.GetPixelColor(i-1,j);
					RGBQUAD lrgb=currLabel.GetPixelColor(i-1,j);
					if (index[f][(j-ymin)*(2*winSize+1)+i-1-xmin]==-1)
					{			
						float forposs,backposs;
						if (i-1==xmin||j==ymin||j==ymax)
						{
							if (lrgb.rgbBlue==255)
							{
								forposs=4500;
								backposs=0;
							}
							else
							{
								forposs=0;
								backposs=4500;
							}
						}
						else
						{
							forposs=CalForTWeight(fgmm,rgb2);
							backposs=CalBackTWeight(bgmm,rgb2);
						}
						graph->set_tweights(gnode[count],forposs,backposs);
						index[f][(j-ymin)*(2*winSize+1)+i-1-xmin]=count;
						count++;
					}
					double nweight=CalNWeight(rgb,rgb2);
					graph->add_edge(gnode[index[f][(j-ymin)*(2*winSize+1)+i-xmin]],gnode[index[f][(j-ymin)*(2*winSize+1)+i-1-xmin]],nweight,nweight);
				}//end if

				if (i+1<=xmax)
				{
					RGBQUAD rgb2=curr.GetPixelColor(i+1,j);
					RGBQUAD lrgb=currLabel.GetPixelColor(i+1,j);

					if (index[f][(j-ymin)*(2*winSize+1)+i+1-xmin]==-1)
					{
						float forposs,backposs;
						if (i+1==xmax||j==ymin||j==ymax)
						{
							if (lrgb.rgbBlue==255)
							{
								forposs=4500;
								backposs=0;
							}
							else
							{
								forposs=0;
								backposs=4500;
							}
						}
						else
						{
							forposs=CalForTWeight(fgmm,rgb2);
							backposs=CalBackTWeight(bgmm,rgb2);
						}						
						graph->set_tweights(gnode[count],forposs,backposs);
						index[f][(j-ymin)*(2*winSize+1)+i+1-xmin]=count;
						count++;
					}
					double nweight=CalNWeight(rgb,rgb2);
					graph->add_edge(gnode[index[f][(j-ymin)*(2*winSize+1)+i-xmin]],gnode[index[f][(j-ymin)*(2*winSize+1)+i+1-xmin]],nweight,nweight);
				}//end if
				if (j-1>=ymin)
				{
					
					RGBQUAD rgb2=curr.GetPixelColor(i,j-1);
					RGBQUAD lrgb=currLabel.GetPixelColor(i,j-1);

					if (index[f][(j-1-ymin)*(2*winSize+1)+i-xmin]==-1)
					{
						float forposs,backposs;

						if (i==xmin||i==xmax||j-1==ymin)
						{
							if (lrgb.rgbBlue==255)
							{
								forposs=4500;
								backposs=0;
							}
							else
							{
								 forposs=0;
								 backposs=4500;
							}
						}
						else
						{
							forposs=CalForTWeight(fgmm,rgb2);
							backposs=CalBackTWeight(bgmm,rgb2);
						}

						graph->set_tweights(gnode[count],forposs,backposs);
						index[f][(j-1-ymin)*(2*winSize+1)+i-xmin]=count;
						count++;
					}

					double nweight=CalNWeight(rgb,rgb2);
					graph->add_edge(gnode[index[f][(j-ymin)*(2*winSize+1)+i-xmin]],gnode[index[f][(j-1-ymin)*(2*winSize+1)+i-xmin]],nweight,nweight);
				}//end if

				if (j+1<=ymax)
				{
					RGBQUAD rgb2=curr.GetPixelColor(i,j+1);
					RGBQUAD lrgb=currLabel.GetPixelColor(i,j+1);
					if (index[f][(j+1-ymin)*(2*winSize+1)+i-xmin]==-1)
					{
						float forposs,backposs;

						if (j+1==ymax||i==xmin||i==xmax)
						{
							if (lrgb.rgbBlue==255)
							{
								forposs=4500;
								backposs=0;
							}
							else
							{
								forposs=0;
								backposs=4500;
							}
						}
						else
						{
							forposs=CalForTWeight(fgmm,rgb2);
							backposs=CalBackTWeight(bgmm,rgb2);
						}
						graph->set_tweights(gnode[count],forposs,backposs);
						index[f][(j+1-ymin)*(2*winSize+1)+i-xmin]=count;
						count++;
					}
					double nweight=CalNWeight(rgb,rgb2);
					graph->add_edge(gnode[index[f][(j-ymin)*(2*winSize+1)+i-xmin]],gnode[index[f][(j+1-ymin)*(2*winSize+1)+i-xmin]],nweight,nweight);
				}//end if
				//end neighbour
				//temporal weight
				if (f!=rrange-1)
				{
					int ni,nj;
					ni=i+u[f][j*width+i];
					nj=j+v[f][j*width+i];

					if (ni>=xn-winSize&&ni<=xn+winSize&&nj>=yn-winSize&&nj<=yn+winSize)//位于下一帧窗口内
					{
						RGBQUAD rgb3=next.GetPixelColor(ni,nj);
						RGBQUAD nrgb=nextLabel.GetPixelColor(ni,nj);
						float forposs,backposs;

						if (ni==xn-winSize||ni==xn+winSize||nj==yn-winSize||nj==yn+winSize)
						{
							if (nrgb.rgbBlue==255)
							{
								forposs=4500;
								backposs=0;
							}
							else
							{
								forposs=0;
								backposs=4500;
							}
						}
						else
						{
							forposs=CalForTWeight(nfgmm,rgb3);
							backposs=CalBackTWeight(nbgmm,rgb3);
						}
						graph->set_tweights(gnode[count],forposs,backposs);
						index[f+1][(nj-(yn-winSize))*(2*winSize+1)+ni-(xn-winSize)]=count;
						count++;
						double temporalWeight=CalTemporalWeight(rgb.rgbRed,rgb.rgbGreen,rgb.rgbBlue,rgb3.rgbRed,rgb3.rgbGreen,rgb3.rgbBlue);
						graph->add_edge(gnode[index[f][(j-ymin)*(2*winSize+1)+i-xmin]],gnode[index[f+1][(nj-(yn-winSize))*(2*winSize+1)+ni-(xn-winSize)]],20*temporalWeight,20*temporalWeight);
					}
				}
			}
		}
		x=xn;
		y=yn;
	}
}
float CMultiFrameRefine::CalForPoss(GMMNode fgmm[COMNUM],int gmmindex,RGBQUAD rgb)
{
	int z[3];
	z[0]=rgb.rgbRed;
	z[1]=rgb.rgbGreen;
	z[2]=rgb.rgbBlue;
	double temp[3];
	double temp2[3];

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
			return (1.0/(sqrt(abs(fgmm[gmmindex].det)))*Got_Exp(-0.5*mi));
		}
		return 0;
	}
	return 0;
}

float CMultiFrameRefine::CalBackPoss(GMMNode bgmm[COMNUM],int gmmindex,RGBQUAD rgb)
{

	int z[3];
	//RGBQUAD rgb=colimage.GetPixelColor(x,y,true);
	z[0]=rgb.rgbRed;
	z[1]=rgb.rgbGreen;
	z[2]=rgb.rgbBlue;

	double temp[3];

	double temp2[3];

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
			return (1.0/(sqrt(abs(bgmm[gmmindex].det)))*Got_Exp(-0.5*mi));
			//return Got_Exp(-0.5*mi/(sqrt(abs(bgmm[gmmindex].det))));
		}
		return 0;
	}
	return 0;
}

float CMultiFrameRefine::CalForTWeight(GMMNode fgmm[COMNUM],RGBQUAD rgb)
{
	float  forposs=0.0f;
	for (int jtemp=0;jtemp<gmmnum;jtemp++)
	{
		//cout<<"count:"<<fgmm[jtemp].count<<"pi:"<<fgmm[jtemp].pi<<endl;
		if (fgmm[jtemp].count>20)
		{
			forposs+=fgmm[jtemp].pi*CalForPoss(fgmm,jtemp,rgb);			
		}
		else
			continue;	
	}
	if (forposs)
	     return log(forposs);
	else 
		return -100;
	
}

float CMultiFrameRefine::CalBackTWeight(GMMNode bgmm[COMNUM],RGBQUAD rgb)
{
	float backposs=0.0f;

	for(int jtemp=0;jtemp<gmmnum;++jtemp)
	{
		if (bgmm[jtemp].count>20)
		{
			backposs+=bgmm[jtemp].pi*CalBackPoss(bgmm,jtemp,rgb);
		}
		else
			continue;
	}
	if (backposs)
	    return log(backposs);
	else 
		return -100;
}

float CMultiFrameRefine::CalNWeight(RGBQUAD rgb1,RGBQUAD rgb2)
{
	double distanceE=1;
	double discol=coldistance(rgb1,rgb2);

	return calN(distanceE,discol)*0.1;
}

double CMultiFrameRefine::coldistance(RGBQUAD c1,RGBQUAD c2)
{
	return (c1.rgbRed-c2.rgbRed)*(c1.rgbRed-c2.rgbRed)+(c1.rgbGreen-c2.rgbGreen)*(c1.rgbGreen-c2.rgbGreen)+(c1.rgbBlue-c2.rgbBlue)*(c1.rgbBlue-c2.rgbBlue);
}

double CMultiFrameRefine::calN(double distance, double coldis)
{
	return 50*Got_Exp(-beta*coldis)/distance;
}

double CMultiFrameRefine::GetBeta(CxImage* colimage,int x,int y,int winSize)
{

	double total=0.0;
	int num=0;
	for (int i=x-winSize;i<=x+winSize;++i)
	{
		for (int j=y-winSize;j<=y+winSize;++j)
		{
			if (i>=x-winSize&&j+1<=y+winSize)
			{
				total+=coldistance(colimage->GetPixelColor(i,j,true),colimage->GetPixelColor(i,j+1));
				num++;
			}
			if (i>=x-winSize&&j-1>y-winSize)
			{
				total+=coldistance(colimage->GetPixelColor(i,j,true),colimage->GetPixelColor(i,j-1));
				num++;
			}
			if (i-1>=x-winSize&&j<=y+winSize)
			{
				total+=coldistance(colimage->GetPixelColor(i,j,true),colimage->GetPixelColor(i-1,j));
				num++;
			}
			if (i+1<=x+winSize&&j<=y+winSize)
			{
				total+=coldistance(colimage->GetPixelColor(i,j,true),colimage->GetPixelColor(i+1,j));
				num++;
			}
		}
	}
	return (1.0)/(2*(total/num));
}


double CMultiFrameRefine::CalTemporalWeight(int r1,int g1,int b1,int r2,int g2,int b2)
{
	double i=(r1-r2)*(r1-r2)+(g1-g2)*(g1-g2)+(b1-b2)*(b1-b2);
	return Got_Exp(-i/(2*25));
}

void CMultiFrameRefine::SampleGMM(GMMNode fgmm[COMNUM],GMMNode bgmm[COMNUM],int x,int y,int winSize,CxImage &image,CxImage &label,CxImage &trimap)
{
	vector<colorNode> fortempvec;//all forground pixel
	vector<colorNode> backtempvec;//all background pixel
	fortempvec.clear();
	backtempvec.clear();
	for (int i=x-winSize;i<=x+winSize;++i)
	{
		for(int j=y-winSize;j<=y+winSize;++j)
		{
			RGBQUAD rgb=image.GetPixelColor(i,j);
			RGBQUAD rgb2=label.GetPixelColor(i,j);
			RGBQUAD rgb3=trimap.GetPixelColor(i,j);

			colorNode co;
			co.indexX=i;
			co.indexY=j;
			co.colorinfo.resize(3,1);
			co.colorinfo(0,0)=(double)rgb.rgbRed;
			co.colorinfo(1,0)=(double)rgb.rgbGreen;
			co.colorinfo(2,0)=(double)rgb.rgbBlue;
			if (rgb2.rgbBlue==255&&rgb3.rgbBlue==128)
				fortempvec.push_back(co);
			else if (rgb2.rgbBlue==0&&rgb3.rgbBlue==128)
				backtempvec.push_back(co);				

		}
	}
	vector< vector < colorNode > > forvec;
	vector< vector < colorNode > > backvec;
	forvec.push_back(fortempvec);
	backvec.push_back(backtempvec);

	cluster(forvec,gmmnum-1);
	cluster(backvec,gmmnum-1);

	initGMM(forvec,fgmm);
	initGMM(backvec,bgmm);
}

void CMultiFrameRefine::SampleGMM(GMMNode fgmm[COMNUM],GMMNode bgmm[COMNUM],int x,int y,int winSize,CxImage &image,CxImage &label)
{
	vector<colorNode> fortempvec;//all forground pixel
	vector<colorNode> backtempvec;//all background pixel
	fortempvec.clear();
	backtempvec.clear();
	for (int i=x-winSize;i<=x+winSize;++i)
	{
		for(int j=y-winSize;j<=y+winSize;++j)
		{
			RGBQUAD rgb=image.GetPixelColor(i,j);
			RGBQUAD rgb2=label.GetPixelColor(i,j);
			
			colorNode co;
			co.indexX=i;
			co.indexY=j;
			co.colorinfo.resize(3,1);
			co.colorinfo(0,0)=(double)rgb.rgbRed;
			co.colorinfo(1,0)=(double)rgb.rgbGreen;
			co.colorinfo(2,0)=(double)rgb.rgbBlue;
			if (rgb2.rgbBlue==255)
				fortempvec.push_back(co);
			else if (rgb2.rgbBlue==0)
				backtempvec.push_back(co);				

		}
	}
	vector< vector < colorNode > > forvec;
	vector< vector < colorNode > > backvec;
	forvec.push_back(fortempvec);
	backvec.push_back(backtempvec);

	cluster(forvec,gmmnum-1);
	cluster(backvec,gmmnum-1);

	initGMM(forvec,fgmm);
	initGMM(backvec,bgmm);
}

void CMultiFrameRefine::TrimapRefine(ImageBuffer* alphaBuffer,ImageBuffer* imageBuffer,ImageBuffer* trimapBuffer,int refineRange,int trimapWidth)
{
	this->GenerateTrimap(alphaBuffer,imageBuffer,trimapBuffer,refineRange,trimapWidth);
	cout<<"after generate trimap"<<endl;

	int pos=alphaBuffer->GetFramePos();//prepare to restore buffer

	//get all window centers
	CxImage label;
	VVPOINTS vec;//window centers
	label.Copy(*(alphaBuffer->GetImage()));

	width=label.GetWidth();
	height=label.GetHeight();

	int *ll=new int[width*height];
	for (int j=0;j<height;++j)
	{
		for (int i=0;i<width;++i)
		{
			ll[j*width+i]=label.GetPixelGray(i,j)/255;
		}
	}
	BiImageProcess::GetAllWindowsCenters(vec,ll,width,height,2*15+1);//winsize=15
	delete[] ll;
	//calculate Optical flow
	float **u;
	float **v;
	//remember free
	u=new float *[rrange-1];
	v=new float *[rrange-1];

	float **u2;
	float **v2;

	u2=new float *[rrange-1];
	v2=new float *[rrange-1];

	for (int i=0;i<rrange-1;++i)
	{
		u[i]=new float[width*height];
		v[i]=new float[width*height];

		u2[i]=new float[width*height];
		v2[i]=new float[width*height];

		memset(u[i],0,sizeof(float)*width*height);
		memset(v[i],0,sizeof(float)*width*height);

		memset(u2[i],0,sizeof(float)*width*height);
		memset(v2[i],0,sizeof(float)*width*height);
	}

	CxImage frame1,frame2;
	for (int i=0;i<rrange-1;++i)
	{
		frame1.Copy(*(imageBuffer->GetImage()));
		bool next=imageBuffer->Forward();
		if (next)
		{
			frame2.Copy(*(imageBuffer->GetImage()));
			CalOpticalFlow(frame1,frame2,u[i],v[i]);
			CalOpticalFlow(frame2,frame1,u2[i],v2[i]);
		}
		else
			break;
	}// cal optical flow over
	imageBuffer->Goto(pos);//restore buffer

	Graph *graph;
	graph=new Graph;

	Graph::node_id *gnode;
	gnode=new Graph::node_id[rrange*width*height];

	for(int itemp=0;itemp<rrange*width*height;++itemp)
	{
		gnode[itemp]=graph->add_node();
	}
	//initialize index
	int **index;
	//remember free
	index=new int *[refineRange];
	for (int itemp=0;itemp<refineRange;++itemp)
	{
		index[itemp]=new int[width*height];
		for (int jtemp=0;jtemp<width*height;++jtemp)
		{
			index[itemp][jtemp]=-1;
		}
	}// end of initilalize index

	int count=0;
	for (int i=0;i<vec.size();++i)
	{
		for (int j=0;j<vec[i].size();++j)
		{
			int x=vec[i][j].x;
			int y=vec[i][j].y;

			//localRefine
			cout<<"before construct graph"<<endl;
			if (x>=0&&y>=0&&x<width&&y<height)
				ConstructGraph(x,y,graph,gnode,u,v,u2,v2,index,15,alphaBuffer,imageBuffer,trimapBuffer,count);
			cout<<"after construct graph"<<endl;

			alphaBuffer->Goto(pos);
			imageBuffer->Goto(pos);
			trimapBuffer->Goto(pos);
		}
	}
	graph->maxflow();

	RGBQUAD w,b;
	w.rgbBlue=w.rgbGreen=w.rgbRed=255;
	b.rgbBlue=b.rgbGreen=b.rgbRed=0;
	for (int i=0;i<rrange;++i)
	{
		CxImage label;
		label.Copy(*(alphaBuffer->GetImage()));
		for (int n=0;n<label.GetHeight();++n)
		{
			for (int m=0;m<label.GetWidth();++m)
			{
				if (index[i][n*width+m]!=-1)
				{
					if (graph->what_segment(gnode[index[i][n*width+m]])==Graph::SOURCE)
					{
						label.SetPixelColor(m,n,w);
					}
					else
						label.SetPixelColor(m,n,b);
				}
			}
		}
		BiImageProcess::checkConnectivity(label);
		label.Save(alphaBuffer->GetFrameName(),CXIMAGE_FORMAT_BMP);
		alphaBuffer->Forward();
	}
	alphaBuffer->Goto(pos);

	delete graph;
	delete[] gnode;
	
	for (int it=0;it<refineRange;++it)
	{
		delete[] index[it];
		index[it]=NULL;
	}
	delete[] index;
	index=NULL;

	for (int it=0;it<rrange-1;++it)
	{
		delete[] u[it];
		delete[] v[it];
		delete[] u2[it];
		delete[] v2[it];

		u[it]=NULL;
		v[it]=NULL;
		u2[it]=NULL;
		v2[it]=NULL;
	}
	delete[] u;
	u=NULL;
	delete[] v;
	v=NULL;
	delete[] u2;
	u2=NULL;
	delete[] v2;
	v2=NULL;
}

void CMultiFrameRefine::GenerateTrimap(ImageBuffer* alphaBuffer,ImageBuffer* imageBuffer,ImageBuffer* trimapBuffer,int refineRange,int trimapWidth)
{
	int pos=alphaBuffer->GetFramePos();

	CxImage* label;
	CxImage trimap;

	vector<POINT> bound;
	RGBQUAD rgb;
	rgb.rgbBlue=rgb.rgbGreen=rgb.rgbRed=128;
	RGBQUAD rgb2;
	

	label=alphaBuffer->GetImage();
	int w=label->GetWidth();
	int h=label->GetHeight();

	int count=refineRange;
	do 
	{
		label=alphaBuffer->GetImage();//label
		trimap.Copy(*label);//trimap image

		BiImageProcess::getBound(bound,label);
		int size=(int )bound.size();
		int x,y;
		for (int i=0;i<size;++i)
		{
			x=bound[i].x;
			y=bound[i].y;

			for (int k=y-trimapWidth;k<=y+trimapWidth;++k)
			{
				for(int p=x-trimapWidth;p<=x+trimapWidth;++p)
				{
					if (k>=0&&k<h&&p>=0&&p<w)
					{
						
						if (BiImageProcess::distance(p,k,x,y)<=trimapWidth)
						{
						
							trimap.SetPixelColor(p,k,rgb);//128
						}
											
					}
				}
			}

		}
		trimap.Save(trimapBuffer->GetFrameName(),CXIMAGE_FORMAT_BMP);
		count--;
	} while (alphaBuffer->Forward()&&trimapBuffer->Forward()&&imageBuffer->Forward()&&count);//color samples and trimap generation

	rrange=refineRange-count;


	
	//reset imagebuffer
	alphaBuffer->Goto(pos);
	trimapBuffer->Goto(pos);
	imageBuffer->Goto(pos);

	cout<<rrange<<endl;
}

void CMultiFrameRefine::ConstructGraph(int x,int y,Graph* graph,Graph::node_id *gnode,
				       float **u,float **v,float **u2,float **v2,int **index,int winSize,ImageBuffer* alphaBuffer,ImageBuffer* imageBuffer,ImageBuffer* trimapBuffer,int &count)
{

	CxImage curr;
	CxImage next;
	CxImage previous;
	CxImage currLabel;
	CxImage nextLabel;
	CxImage preLabel;
	CxImage currTrimap;
	CxImage nextTrimap;
	CxImage preTrimap;

	GMMNode fgmm[3];//gmm of current frame
	GMMNode bgmm[3];
	GMMNode nfgmm[3];//gmm of next frame
	GMMNode nbgmm[3];

	GMMNode pfgmm[3];
	GMMNode pbgmm[3];

	for (int f=0;f<rrange;++f)
	{
		cout<<"for time:"<<f<<endl;
		//color image
		previous.Copy(curr);
		curr.Copy(*(imageBuffer->GetImage()));
		bool nex=imageBuffer->Forward();
		if (nex)
			next.Copy(*(imageBuffer->GetImage()));

		//mask image
		preLabel.Copy(currLabel);
		currLabel.Copy(*(alphaBuffer->GetImage()));
		
		bool nextl=alphaBuffer->Forward();
		if (nextl)
			nextLabel.Copy(*(alphaBuffer->GetImage()));

		//trimap image
		preTrimap.Copy(currTrimap);
		currTrimap.Copy(*(trimapBuffer->GetImage()));
		nex=trimapBuffer->Forward();
		if (nex)
			nextTrimap.Copy(*(trimapBuffer->GetImage()));

		cout<<"before sampleGMM"<<endl;
#ifdef MULTI_GMM
		SampleGMM(fgmm,bgmm,x,y,winSize,curr,currLabel,currTrimap);
#else
		if (f==0)
			SampleGMM(fgmm,bgmm,x,y,winSize,curr,currLabel,currTrimap);
		
#endif
		
		int xn,yn;
		cout<<"before f!=rrange-1"<<endl;
		if (f!=rrange-1)
		{
			xn=x+u[f][y*width+x];
			yn=y+v[f][y*width+x];
			//next frame gmm

#ifdef MULTI_GMM

			SampleGMM(nfgmm,nbgmm,xn,yn,winSize,next,nextLabel);
#endif
			//next frame gmm
		}

		cout<<"before f!=0"<<endl;
		int xp,yp;
		if (f!=0)
		{
			xp=x+u2[f-1][y*width+x];
			yp=y+v2[f-1][y*width+x];
			cout<<"before sampleGMM"<<endl;
#ifdef MULTI_GMM
			SampleGMM(pfgmm,pbgmm,xp,yp,winSize,previous,preLabel);
#endif
			
		}
		cout<<"gmm calculation over"<<endl;
		//gmm over
		int xmin=(x-winSize>=0)?(x-winSize):0;
		int ymin=(y-winSize>=0)?(y-winSize):0;
		int xmax=(x+winSize<width)?(x+winSize):(width-1);
		int ymax=(y+winSize<height)?(y+winSize):(height-1);
		//construct graph
		//static int count=0;
		for (int i=xmin;i<=xmax;++i)
		{
			for (int j=ymin;j<=ymax;++j)
			{
				RGBQUAD trimapCol,trimapCol2;
				trimapCol=currTrimap.GetPixelColor(i,j);

				RGBQUAD imageCol,imageCol2;
				if (trimapCol.rgbBlue==128)
				{
					imageCol=curr.GetPixelColor(i,j);
					if (index[f][j*width+i]==-1)
					{			
						float forposs,backposs;
						forposs=CalForTWeight(fgmm,imageCol);
						backposs=CalForTWeight(bgmm,imageCol);

						graph->set_tweights(gnode[count],forposs,backposs);
						index[f][j*width+i]=count;
						count++;
					}
					if (i-1>=xmin)
					{
						trimapCol2=currTrimap.GetPixelColor(i-1,j);
						if (trimapCol.rgbBlue==128)
						{
							if (index[f][j*width+i-1]==-1)
							{
								imageCol2=curr.GetPixelColor(i-1,j);
								float forposs,backposs;
								forposs=CalForTWeight(fgmm,imageCol2);
								backposs=CalBackTWeight(bgmm,imageCol2);
								graph->set_tweights(gnode[count],forposs,backposs);

								index[f][j*width+i-1]=count;
								count++;
							}
							float nWeight=CalNWeight(imageCol,imageCol2);
							graph->add_edge(gnode[index[f][j*width+i]],gnode[index[f][j*width+i-1]],nWeight,nWeight);
						}
					}//end of if (i-1>=xmin)
					if (i+1<=xmax)
					{
						trimapCol2=currTrimap.GetPixelColor(i+1,j);
						if (trimapCol2.rgbBlue==128)
						{
							if (index[f][j*width+i+1]==-1)
							{
								imageCol2=curr.GetPixelColor(i+1,j);
								float forposs,backposs;
								forposs=CalForTWeight(fgmm,imageCol2);
								backposs=CalBackTWeight(bgmm,imageCol2);

								graph->set_tweights(gnode[count],forposs,backposs);
								index[f][j*width+i+1]=count;
								count++;
							}
							float nWeight=CalNWeight(imageCol,imageCol2);
							graph->add_edge(gnode[index[f][j*width+i]],gnode[index[f][j*width+i+1]],nWeight,nWeight);
						}
					}// end of if (i+1<xmax)
					if (j-1>=ymin)
					{
						trimapCol2=currTrimap.GetPixelColor(i,j-1);
						if (trimapCol2.rgbBlue==128)
						{
							if (index[f][(j-1)*width+i]==-1)
							{
								imageCol2=curr.GetPixelColor(i,j-1);
								float forposs,backposs;
								forposs=CalForTWeight(fgmm,imageCol2);
								backposs=CalBackTWeight(bgmm,imageCol2);

								graph->set_tweights(gnode[count],forposs,backposs);
								index[f][(j-1)*width+i]=count;
								count++;
							}
							float nWeight=CalNWeight(imageCol,imageCol2);
							graph->add_edge(gnode[index[f][j*width+i]],gnode[index[f][(j-1)*width+i]],nWeight,nWeight);
						}
					}//end of 	if (j-1>=ymin)
					if (j+1<=ymax)
					{
						trimapCol2=currTrimap.GetPixelColor(i,j+1);
						if (trimapCol2.rgbBlue==128)
						{
							if (index[f][(j+1)*width+i]==-1)
							{
								imageCol2=curr.GetPixelColor(i,j+1);
								float forposs,backposs;
								forposs=CalForTWeight(fgmm,imageCol2);
								backposs=CalBackTWeight(bgmm,imageCol2);
								graph->set_tweights(gnode[count],forposs,backposs);
								index[f][(j+1)*width+i]=count;
								count++;
							}
							float nWeight=CalNWeight(imageCol,imageCol2);
							graph->add_edge(gnode[index[f][j*width+i]],gnode[index[f][(j+1)*width+i]],nWeight,nWeight);
						}
					}//end of if (j+1<ymax)
					//temporal 
					if (f!=rrange-1)
					{
						int ni=i+u[f][j*width+i];
						int nj=j+v[f][j*width+i];

						int xnmin=(xn-winSize>=0)?(xn-winSize):0;
						int ynmin=(yn-winSize>=0)?(yn-winSize):0;
						int xnmax=(xn+winSize<width)?(xn+winSize):(width-1);
						int ynmax=(yn+winSize<height)?(yn+winSize):(height-1);

						if (ni>=xnmin&&ni<=xnmax&&nj>=ynmin&&nj<=ynmax)//位于下一窗口内
						{
							trimapCol2=nextTrimap.GetPixelColor(ni,nj);
							if (trimapCol2.rgbBlue==128)//位于未知区域内
							{
								imageCol2=next.GetPixelColor(ni,nj);
								if (index[f+1][nj*width+ni]==-1)
								{
									
									float forposs,backposs;
#ifdef MULTI_GMM
									forposs=CalForTWeight(nfgmm,imageCol2);
									backposs=CalBackTWeight(nbgmm,imageCol2);
#else
									forposs=CalForTWeight(fgmm,imageCol2);
									backposs=CalBackTWeight(bgmm,imageCol2);
#endif
									

									graph->set_tweights(gnode[count],forposs,backposs);
									index[f+1][nj*width+ni]=count;
									count++;
								}
								float temporalWeight;
								temporalWeight=CalTemporalWeight(imageCol.rgbRed,imageCol.rgbGreen,imageCol.rgbBlue,imageCol2.rgbRed,imageCol2.rgbGreen,imageCol2.rgbBlue);

								graph->add_edge(gnode[index[f][j*width+i]],gnode[index[f+1][nj*width+ni]],20*temporalWeight,20*temporalWeight);
							}
						}
					}//end of if (f!=rrange-1) 
					if (f!=0)
					{
						int ni=i+u2[f-1][j*width+i];
						int nj=j+v2[f-1][j*width+i];

						int xnmin=(xp-winSize>=0)?(xp-winSize):0;
						int ynmin=(yp-winSize>=0)?(yp-winSize):0;
						int xnmax=(xp+winSize<width)?(xp+winSize):(width-1);
						int ynmax=(yp+winSize<height)?(yp+winSize):(height-1);

						if (ni>=xnmin&&ni<=xnmax&&nj>=ynmin&&nj<=ynmax)//位于上一窗口内
						{
							trimapCol2=preTrimap.GetPixelColor(ni,nj);

							if (trimapCol2.rgbBlue==128)//位于未知区域内
							{
								imageCol2=previous.GetPixelColor(ni,nj);
								if (index[f-1][nj*width+ni]==-1)
								{
									float forposs,backposs;
#ifdef MULTI_GMM
									forposs=CalForTWeight(pfgmm,imageCol2);
									backposs=CalBackTWeight(pbgmm,imageCol2);
#else
									forposs=CalForTWeight(fgmm,imageCol2);
									backposs=CalBackTWeight(bgmm,imageCol2);
#endif
									graph->set_tweights(gnode[count],forposs,backposs);
									index[f-1][nj*width+ni]=count;
									count++;
								}
								float temporalWeight;
								temporalWeight=CalTemporalWeight(imageCol.rgbRed,imageCol.rgbGreen,imageCol.rgbBlue,imageCol2.rgbRed,imageCol2.rgbGreen,imageCol2.rgbBlue);
								graph->add_edge(gnode[index[f][j*width+i]],gnode[index[f-1][nj*width+ni]],20*temporalWeight,20*temporalWeight);
							}
						}
					}			
				}// end of if (trimapCol.rgbBlue==128)
			}
		}
		x=xn;
		y=yn;

		if (x<0)
			x=0;
		if (x>=width)
			x=width-1;
		if (y<0)
			y=0;
		if (y>=height)
			y=height-1;
		
		
	}
}
