#include "Grabcut.h"
#include <stack>
#include <list>
#include <vector>
#include "BiImageProcess.h"
using namespace std;

bool Grabcut::BSInterpolation(float **alphaSource,float **alphaDes,int oldX,int oldY,int newX,int newY)
{
	if (newX<=0||newY<=0)
	{
		return false;
	}

	if (newX==oldX&&newY==oldY)
	{
		memcpy(alphaDes,alphaSource,sizeof(int)*oldX*oldY);
		return true;
	}
	float xScale,yScale;

	xScale=(float)oldX/(float)newX;
	yScale=(float)oldY/(float)newY;

	float f_x,f_y,a,b,value,r1,r2;
	int i_x,i_y,xx,yy;

	float value2;
	for (int y=0;y<newY;++y)
	{
		f_y=(float)y*yScale-0.5;
		i_y=(int)floor(f_y);
		a=f_y-(float)floor(f_y);

		for (int x=0;x<newX;++x)
		{
			f_x=(float)x*xScale-0.5;
			i_x=(int)floor(f_x);
			b=f_x-(float)floor(f_x);

			value=0;
			for (int m=-1;m<3;m++)
			{
				r1=KernelBSpline((float)-a);
				yy=i_y+m;
				if(yy<0) yy=0;
				if(yy>=oldY) yy=oldY-1;

				for (int n=-1;n<3;n++)
				{
					r2=r1*KernelBSpline(b-(float)n);
					xx=i_x+n;
					if(xx<0) xx=0;
					if(xx>=oldX) xx=oldX-1;

					value2=alphaSource[xx][yy];

					value+=value2*r2;
				}
			}
			alphaDes[x][y]=value;
		}
	}
    return true;
}
//
//void Grabcut::JointBilateralUpsample(float **alphaSource,float **alphaDes,int oldX,int oldY,int newX,int newY)//(QImage *GrayImage,QImage *Original,QString resultName)
//{
//	double sigmad =0.5,sigmar =0.25,rscale= 1.0;
//	int widthGray = newX;
//	int heightGray =newY;
//	int widthOri = oldX;
//	int heightOri = oldY;
//	double scalenow = (double)widthOri/widthGray;
//	int domain = 2;
//	for (int i=0;i<widthOri;i++)
//	{
//		for (int j=0;j<heightOri;j++)
//		{
//			int px = (int)(i/scalenow+0.5),py = (int)(j/scalenow+0.5);
//			double pxt = i/scalenow,pyt=j/scalenow; 
//			float pColor =  alphaSource[i][j];
//			int istart = i-domain,jstart = j-domain;
//			int iend = i+domain,jend = j+domain;
//			int wstart = istart>0?istart:0,wend = iend>=widthOri?widthOri-1:iend;
//			int vstart = jstart >0?jstart:0,vend = jend>=heightOri?heightOri-1:jend;
//			double fgSum = 0.0,weightSum =0.0;
//			for (int w=wstart;w<=wend;w++)
//			{
//				for (int v=vstart;v<=vend;v++)
//				{
//					float qColor = alphaSource[w][v];
//					float Distanceqp = ((double)(w/scalenow-pxt)*(w/scalenow-pxt)+(v/scalenow-pyt)*(v/scalenow-pyt));
//					float domianFilter = (1/sigmad)*exp(-0.5*(Distanceqp/(sigmad*sigmad)));
//					domianFilter = domianFilter/0.5;
//					float tempDouble =(pColor-qColor) *(pColor-qColor);
//					float rangeFilter = exp(-0.5*tempDouble/(sigmar*sigmar));
//					float fg =  domianFilter*rangeFilter;
//					fgSum = fgSum +fg;
//					float qColorInt = QRgbToGray(GrayImage->pixel((int)((w/scalenow+0.5>=widthGray?widthGray-1:(w/scalenow+0.5))),(int)(v/scalenow+0.5>=heightGray?heightGray-1:(v/scalenow+0.5)))
//						)/255.0;
//					weightSum = weightSum + qColorInt*fg;
//				}
//			}
//			double pColorInt = weightSum/fgSum*255.0;
//
//			QColor color;
//			color.setBlue((int)pColorInt);
//			color.setRed(color.blue());
//			color.setGreen(color.blue());
//			result->setPixel(i,j,color.rgb());
//		}
//	}
//	result->save(resultName);
//	delete result;
//}
//

double Grabcut::KernelBSpline(const float x)
{
	if (x>2.0f) return 0.0f;

	float a, b, c, d;
	float xm1 = x - 1.0f;
	float xp1 = x + 1.0f;
	float xp2 = x + 2.0f;

	if ((xp2) <= 0.0f) a = 0.0f; else a = xp2*xp2*xp2;
	if ((xp1) <= 0.0f) b = 0.0f; else b = xp1*xp1*xp1;
	if (x <= 0) c = 0.0f; else c = x*x*x;  
	if ((xm1) <= 0.0f) d = 0.0f; else d = xm1*xm1*xm1;

	return (0.16666666666666666667f * (a - (4.0f * b) + (6.0f * c) - (4.0f * d)));

}

void Grabcut::checkConnectivity(float **llabel, int w, int h)
{
	int N=0;
	unsigned char **P;
	unsigned char **coexist;
	unsigned char *num;

	P=new unsigned char *[w];
	coexist=new unsigned char*[NMAX];
	num=new unsigned char[NMAX];

	memset(num,0,sizeof(unsigned char)*NMAX);

	for (int i=0;i<w;++i)
	{
		P[i]=new unsigned char[h];

		memset(P[i],0,sizeof(unsigned char)*h);
	}

	for (int i=0;i<NMAX;++i)
	{
		coexist[i]=new unsigned char [NMAX];
		memset(coexist[i],0,sizeof(unsigned char)*NMAX);
	}

	for (int i=2;i<w-1;++i)
	{
		for (int j=2;j<h-1;++j)
		{
			if (llabel[i][j])
			{
				if (!llabel[i-1][j]&&!llabel[i][j-1]&&!llabel[i+1][j]&&!llabel[i][j+1])
					llabel[i][j]=0;
			}
		}
	}

	for (int i=0;i<w;++i)//label object
	{
		for (int j=0;j<h;++j)
		{
			if (llabel[i][j])
			{
				if (!neighbourNCaled(i,j,w,h,P))//邻居均为0
				{

					++N;
					P[i][j]=N;


				}
				else 
				{
					P[i][j]=NeighbourMax(i,j,w,h,P);
					num[P[i][j]]++;
					UpdateCoexists(i,j,w,h,P,coexist);
				}
			}
		}
	}//label over
	int included[NMAX];
	for (int i=1;i<=N;++i)
	{
		included[i]=0;
	}

	stack<int> stack1;
	list<int>     list1[NMAX];
	for (int i=1;i<=N;++i)
	{
		list1[i].clear();
	}

	//DFS
	for (int i=1;i<=N;++i)
	{
		while(!included[i])//unhandle
		{
			included[i]=2;

			stack1.push(i);
			list1[i].push_back(i);

			while (!stack1.empty())
			{
				int k=stack1.top();
				stack1.pop();

				if (k!=i&&included[k]!=1)
				{
					num[i]+=num[k];
					included[k]=1;
				}

				for (int j=i;j<=N;++j)
				{
					if (coexist[k][j]&&(!included[j]))
					{
						list1[i].push_back(j);
						stack1.push(j);
					}

				}
			}
		}
	}//DFS

	int maxNum=-1;
	int maxObjectIndex=-1;
	for (int i=1;i<=N;++i)
	{

		if (included[i]==2)
		{
			if (maxNum<num[i])
			{
				maxNum=num[i];
				maxObjectIndex=i;
			}
		}
	}

	int numm=0;

	for (int i=0;i<w;++i)
	{
		for (int j=0;j<h;++j)
		{
			if(llabel[i][j])
				if (P[i][j]!=maxObjectIndex)
				{

					if (find(list1[maxObjectIndex].begin(),list1[maxObjectIndex].end(),P[i][j])==list1[maxObjectIndex].end())
					{
						llabel[i][j]=0;
						numm++;
					}

				}
		}
	}

	for (int i=0;i<w;++i)
	{
		delete[] P[i];
		P[i]=NULL;
	}
	delete[] P;
	P=NULL;

	for (int i=0;i<NMAX;++i)
	{
		delete[] coexist[i];
		coexist[i]=NULL;
	}

	delete[] coexist;
	coexist=NULL;

	delete[] num;  
	num=NULL;
}

void Grabcut::checkConnectivity(unsigned char *label,int w,int h)
{
	int N=0;
	unsigned char **P;
	unsigned char **coexist;
	unsigned char *num;

	P=new unsigned char*[w];
	coexist=new unsigned char*[NMAX];
	num=new unsigned char[NMAX];

	memset(num,0,sizeof(unsigned char)*NMAX);

	for (int i=0;i<w;++i)
	{
		P[i]=new unsigned char[h];
		memset(P[i],0,sizeof(unsigned char)*h);
	}

	for (int i=0;i<NMAX;++i)
	{
		coexist[i]=new unsigned char [NMAX];
		memset(coexist[i],0,sizeof(unsigned char)*NMAX);
	}

	int index=0;
	for (int i=2;i<w-1;++i)
	{
		for (int j=2;j<h-1;++j)
		{
			index=j*w+i;
			if (label[index])
			{
				if (!label[j*w+i-1]&&!label[(j-1)*w+i]&&!label[j*w+i+1]&&!label[(j+1)*w+i])
					label[index]=0;
			}
		}
	}

	for (int i=0;i<w;++i)//label object
	{
		for (int j=0;j<h;++j)
		{
			index=j*w+i;
			if (label[index])
			{
				if (!neighbourNCaled(i,j,w,h,P))//邻居均为0
				{

					++N;
					P[i][j]=N;


				}
				else 
				{
					P[i][j]=NeighbourMax(i,j,w,h,P);
					num[P[i][j]]++;
					UpdateCoexists(i,j,w,h,P,coexist);
				}
			}
		}
	}//label over
	int included[NMAX];
	for (int i=1;i<=N;++i)
	{
		included[i]=0;
	}

	stack<int> stack1;
	list<int>     list1[NMAX];
	for (int i=1;i<=N;++i)
	{
		list1[i].clear();
	}

	//DFS
	for (int i=1;i<=N;++i)
	{
		while(!included[i])//unhandle
		{
			included[i]=2;

			stack1.push(i);
			list1[i].push_back(i);

			while (!stack1.empty())
			{
				int k=stack1.top();
				stack1.pop();

				if (k!=i&&included[k]!=1)
				{
					num[i]+=num[k];
					included[k]=1;
				}

				for (int j=i;j<=N;++j)
				{
					if (coexist[k][j]&&(!included[j]))
					{
						list1[i].push_back(j);
						stack1.push(j);
					}

				}
			}
		}
	}//DFS

	int maxNum=-1;
	int maxObjectIndex=-1;
	for (int i=1;i<=N;++i)
	{

		if (included[i]==2)
		{
			if (maxNum<num[i])
			{
				maxNum=num[i];
				maxObjectIndex=i;
			}
		}
	}

	int numm=0;

	for (int i=0;i<w;++i)
	{
		for (int j=0;j<h;++j)
		{
			index=j*w+i;
			if(label[index])
				if (P[i][j]!=maxObjectIndex)
				{

					if (find(list1[maxObjectIndex].begin(),list1[maxObjectIndex].end(),P[i][j])==list1[maxObjectIndex].end())
					{
						label[index]=0;
						numm++;
					}

				}
		}
	}

	for (int i=0;i<w;++i)
	{
		delete[] P[i];
		P[i]=NULL;
	}
	delete[] P;
	P=NULL;

	for (int i=0;i<NMAX;++i)
	{
		delete[] coexist[i];
		coexist[i]=NULL;
	}

	delete[] coexist;
	coexist=NULL;

	delete[] num;
	num=NULL;
}

void Grabcut::manhattan(float **label,int w,int h)
{
	for (int i=0;i<w;++i)
	{
		for (int j=0;j<h;++j)
		{
			if (label[i][j])
			{
				label[i][j]=0;
			}
			else
			{
				label[i][j]=w+h;

				if (i>0)
					label[i][j]=__min(label[i][j],label[i-1][j]+1);

				if (j>0)
					label[i][j]=__min(label[i][j],label[i][j-1]+1);


			}
		}
	}

	for (int i=w-1;i>=0;i--)
	{
		for (int j=h-1;j>=0;j--)
		{
			if (i+1<w) 
				label[i][j]=__min(label[i][j],label[i+1][j]+1);

			if (j+1<h)
				label[i][j]=__min(label[i][j],label[i][j+1]+1);


		}
	}
}

void Grabcut::dilate(float** label,int w,int h,int k)
{
	manhattan(label,w,h);

	for (int i=0;i<w;++i)
	{
		for (int j=0;j<h;++j)
		{
			if (i<xstart*scalar||i>xend*scalar||j<ystart*scalar||j>yend*scalar)
			{
				label[i][j]=0;
			}
			else
				label[i][j]=((label[i][j]<k)?1:0);
		}
	}
}

void Grabcut::erode(float**label,int w,int h,int k)
{

	for (int i=0;i<w;++i)
	{
		for (int j=0;j<h;++j)
		{
			if (!label[i][j])
			{
				if (i>0)
					label[i-1][j]=2;

				if (j+1<h)
					label[i][j+1]=2;
				if (i+1<w)
					label[i+1][j]=2;
				if(j>0)
					label[i][j-1]=2;

			}
		}
	}

	for (int i=0;i<w;++i)
	{
		for (int j=0;j<h;++j)
		{
			if (label[i][j]==2)
			{
				label[i][j]=0;
			}
		}
	}

}

void Grabcut::highLevelCut()
{
	std::cout<<" in high level cut"<<std::endl;
	checkConnectivity(alpha,width,height);
	finalAlpha=new float *[width*scalar];
	for (int i=0;i<width*scalar;++i)
	{
		finalAlpha[i]=new float[height*scalar];
	}
	BSInterpolation(alpha,finalAlpha,width,height,width*scalar,height*scalar);
	dilate(finalAlpha,width*scalar,height*scalar,2);

	graphNode=new Graph::node_id *[width*scalar];
	std::map<std::pair<int, int>, int> colrcompIndex;
	for (int i=0;i<width*scalar;++i)
		graphNode[i]=new Graph::node_id [height*scalar];
	
	CxImage tagimage;
	tagimage.Create(width*scalar, height*scalar, 8);
	tagimage.SetGrayPalette();
	tagimage.Clear();
	getBound(bound,finalAlpha,tagimage,width*scalar,height*scalar);

	int changed=555;
	int count=100;
	while(changed)
	{	
		changed=highLevelCutOnce(bound,graphNode,tagimage);
		assignGMMHighlevel(colrcompIndex,tagimage);//,bound);
		recalGMMHighlevel(colrcompIndex,tagimage);//,bound);
		changed=0;
		count++;
	}
	printf("after while changed\n");
	//checkConnectivity(finalAlpha,width*scalar,height*scalar);
	//BiImageProcess::checkConnectivityfast(finalAlpha, width*scalar, height*scalar);
	SetLabel();
	getBound(bound,finalAlpha,tagimage,width*scalar,height*scalar);

	for (int i=0;i<width*scalar;++i)
	{
		delete[] finalAlpha[i];
		finalAlpha[i]=NULL;
		delete[] graphNode[i];
		graphNode[i]=NULL;
	}
	delete[] finalAlpha;
	finalAlpha=NULL;
	delete[] graphNode;
	graphNode=NULL;
}

void Grabcut::interactiveOnce()
{/*
	if(scalar==1)
	{
		int changed=1;
		while(changed)
			changed=dosegonce(0);
	}
	else
	{
		int changed=1;
		reInitializeGMM();
		changed=highLevelCutOnce(bound,graphNode,tag);
		checkConnectivity(finalAlpha,width*scalar,height*scalar);
		SetLabel();
		getBound(bound,finalAlpha,tag,width*scalar,height*scalar);
		
	}*/
}

int Grabcut::highLevelCutOnce(const vector<POINT> &bound,Graph::node_id **graphNode,CxImage& tag)
{
	graph=new Graph();
	int imax,jmax;
	imax=width*scalar;
	jmax=height*scalar;
	for (int i=0;i<imax;++i)
	{
		for (int j=0;j<jmax;++j)
		{
			if(tag.GetPixelGray(i,j)==128)
				graphNode[i][j]=graph->add_node();
		}
	}

	double forposs;
	double backposs;
	RGBQUAD rgb;
	//caculate T
	for (int i=xstart*scalar;i<xend*scalar;++i)
	{
		for (int j=ystart*scalar;j<yend*scalar;++j)
		{
			if (tag.GetPixelGray(i,j)==128)
			{
				rgb=sourceImage.GetPixelColor(i,j);
				backposs=0;
				forposs=0;
				for (int itemp=0;itemp<5;++itemp)
				{

					if (bgmm[itemp].count)
					{
						backposs+=bgmm[itemp].pi*calposs(0,itemp,rgb);
					}
				}
				backposs=log(backposs);
	

				for (int jtemp=0;jtemp<5;++jtemp)
				{
					if (fgmm[jtemp].count)
					{
						forposs+=fgmm[jtemp].pi*calposs(1,jtemp,rgb);
					}
				}
				forposs=log(forposs);
				graph->add_tweights(graphNode[i][j],forposs,backposs);
			}
		}
	}

	//caculate N

	double nweight = 0;
	for (int i=0;i<imax;++i)
	{
		for (int j=0;j<jmax;++j)
		{
			if (tag.GetPixelGray(i,j)==128)
			{
				if (j+1<jmax)
				{
					if (tag.GetPixelGray(i,j+1)==128)
					{
						nweight = calHighLevelNWeight(i,j,i,j+1);
						graph->add_edge(graphNode[i][j],graphNode[i][j+1],nweight*0.3,nweight*0.3);
					}
				}

				if (j-1>=0)
				{
					if (tag.GetPixelGray(i, j-1)==128)
					{
						nweight = calHighLevelNWeight(i,j,i,j-1);
						graph->add_edge(graphNode[i][j],graphNode[i][j-1],nweight*0.3,nweight*0.3);
					}
				}

				if (i-1>=0)
				{
					if(tag.GetPixelGray(i-1,j)==128)
					{
						nweight = calHighLevelNWeight(i,j,i-1,j);
						graph->add_edge(graphNode[i][j],graphNode[i-1][j],nweight*0.3,nweight*0.3);
					}
				}

				if (i+1<imax)
				{
					if(tag.GetPixelGray(i+1,j)==128)
					{
						nweight = calHighLevelNWeight(i,j, i+1, j);
						graph->add_edge(graphNode[i][j],graphNode[i+1][j],nweight*0.3,nweight*0.3);
					}
				}
			}
		}

	}
	if (graph)
	{
		graph->maxflow();
	}
	float fb;
	int changed=0;
	for (int i=0;i<imax;++i)
	{
		for (int j=0;j<jmax;++j)
		{
			if (tag.GetPixelGray(i,j)==128)
			{
				fb=finalAlpha[i][j];

				if (graph->what_segment(graphNode[i][j])==Graph::SOURCE)
				{
					finalAlpha[i][j]=1;
				}

				else
				{
					finalAlpha[i][j]=0;
				}

				if (fb!=finalAlpha[i][j])
				{
					changed++;
				}
			}

		}

	}
	delete graph;
	return changed;	
}


void Grabcut::getBound(std::vector<POINT> &bound, float **label,CxImage& tag,int w,int h)
{
	cout<<"in bound"<<endl;
	POINT p;
	vector<POINT> btemp;
	for (int i=xstart*scalar;i<xend*scalar&&i<w;++i)
	{
		for (int j=ystart*scalar;j<yend*scalar&&j<h;++j)
		{

			//if (tag[i][j]!=2)
			//{
			//	tag[i][j]=0;
			//}
			if (label[i][j]==1)
			{
				if (i-1>=0)
					if (!label[i-1][j])
					{
						p.x=i;
						p.y=j;
						btemp.push_back(p);
						continue;
					}

					if (i+1<w)
						if (!label[i+1][j])
						{
							p.x=i;
							p.y=j;
							btemp.push_back(p);
							continue;							
						}

						if(j-1>=0)
							if (!label[i][j-1])
							{
								p.x=i;
								p.y=j;
								btemp.push_back(p);
								continue;						
							}

							if(j+1<h)
								if (!label[i][j+1])
								{
									p.x=i;
									p.y=j;
									btemp.push_back(p);
									continue;
								}


			}
		}
	}

	POINT ptemp;
	for (int i=0;i<btemp.size();++i)
	{
		int x=btemp[i].x;
		int y=btemp[i].y;

		for (int j=x+15;j>=x-15;--j)
		{
			for (int k=y+15;k>=y-15;--k)
			{
				if (j>=xstart*scalar&&j<xend*scalar&&j<w&&k>=ystart*scalar&&k<yend*scalar&&k<h)
				{
					if(label[j][k]==0.0f)//&&tag[j][k]!=2)
					{
						if (distance(x,y,j,k)<10)
						{
							ptemp.x=j;
							ptemp.y=k;
							//tag[j][k]=0.5;
							tag.SetPixelIndex(j,k, 128);

						}
					}

					else if (label[j][k]==1.0f)//&&tag[j][k]!=2)
					{
						if (distance(x,y,j,k)<10)
						{
							ptemp.x=j;
							ptemp.y=k;
							//tag[j][k]=0.5;
							tag.SetPixelIndex(j,k,128);
						}
					}
				}
			}
		}
	}
	cout<<"after bound"<<endl;
}

void Grabcut::calHighLevelNWeight(NWeight **weight)//,vector<POINT> &bound)
{

	int imax=width*scalar;
	int jmax=height*scalar;
	double distanceE,discol;
	for (int i=0;i<imax;++i)
	{
		for (int j=0;j<jmax;++j)
		{
			if (j+1<jmax)
			{
				distanceE=distance(i,j,i,j+1);
				discol=coldistance(sourceImage.GetPixelColor(i,j),sourceImage.GetPixelColor(i,j+1));
				weight[i][j].up=calN(distanceE,discol);
			}
			if (j-1>=0)
			{
				distanceE=distance(i,j,i,j-1);
				discol=coldistance(sourceImage.GetPixelColor(i,j),sourceImage.GetPixelColor(i,j-1));
				weight[i][j].down=calN(distanceE,discol);
			}
			if (i-1>=0)
			{
				distanceE=distance(i,j,i-1,j);
				discol=coldistance(sourceImage.GetPixelColor(i,j),sourceImage.GetPixelColor(i-1,j));
				weight[i][j].left=calN(distanceE,discol);
			}
			if (i+1<imax)
			{
				distanceE=distance(i,j,i+1,j);
				discol=coldistance(sourceImage.GetPixelColor(i,j),sourceImage.GetPixelColor(i+1,j));
				weight[i][j].right=calN(distanceE,discol);
			}
		}
	}
}
double Grabcut::calHighLevelNWeight(int x1, int y1, int x2, int y2)
{
	double distanceE=distance(x1,y1,x2,y2);
	double discol=coldistance(sourceImage.GetPixelColor(x1,y1),sourceImage.GetPixelColor(x2,y2));
	return calN(distanceE,discol);
}
void Grabcut::assignGMMHighlevel(std::map<std::pair<int, int>, int>& comp,CxImage& tag)//,vector<POINT> &bound)
{
	double poss;
	int minidex;
	int imax,jmax;
	imax=width*scalar;
	jmax=height*scalar;

	for (int i=0;i<imax;++i)
	{
		for (int j=0;j<jmax;++j)
		{
			if (tag.GetPixelGray(i,j)==128)
			{
				poss=-1;
				minidex=-1;
				for (int k=0;k<COMNUM;++k)
				{
					double tempposs=calposs(finalAlpha[i][j],k,sourceImage.GetPixelColor(i,j));
					if (tempposs>poss)
					{
						poss=tempposs;
						minidex=k;
					}
				}
				//comp[i][j]=minidex;
				comp.insert(make_pair(make_pair(i,j),minidex));
				
			}


		}


	}
}

void Grabcut::recalGMMHighlevel(std::map<std::pair<int, int>, int>& comp,CxImage& tag)//vector<POINT> &bound)
{
	int forcount=0;
	int backcount=0;
	vector<pixelNode> *fpvec=new vector<pixelNode>[COMNUM];
	vector<pixelNode> *bpvec=new vector<pixelNode>[COMNUM];

	int imax,jmax;
	imax=width*scalar;
	jmax=height*scalar;
	for (int i=0;i<imax;++i)
	{
		for (int j=0;j<jmax;++j)
		{
			pixelNode temp;

			if (finalAlpha[i][j]&&tag.GetPixelGray(i,j)==128)
			{
				temp.x=i;
				temp.y=j;
				fpvec[comp[make_pair(i,j)]].push_back(temp);			
				forcount++;
			}
			else if(tag.GetPixelGray(i,j)==128)
			{
				temp.x=i;
				temp.y=j;
				bpvec[comp[make_pair(i,j)]].push_back(temp);
				backcount++;
			}

		}

	}

	for (int k=0;k<COMNUM;++k)
	{
		calGMMPara(fgmm[k],fpvec[k],forcount,sourceImage);
		calGMMPara(bgmm[k],bpvec[k],backcount,sourceImage);
	}
	delete[] fpvec;
	delete[] bpvec;
}

void Grabcut::reInitializeGMM()
{
	vector<colorNode> fortempvec;
	vector<colorNode> backtempvec;
	for (int i=xstart*scalar;i<xend*scalar;++i)
	{
		for (int j=ystart*scalar;j<yend*scalar;++j)
		{
			colorNode temp;
			RGBQUAD rgb=sourceImage.GetPixelColor(i,j);
			temp.indexX=i;
			temp.indexY=j;
			temp.colorinfo.resize(3,1);
			temp.colorinfo(0,0)=(double)rgb.rgbRed;
			temp.colorinfo(1,0)=(double)rgb.rgbGreen;
			temp.colorinfo(2,0)=(double)rgb.rgbBlue;
			if (finalAlpha[i][j])
			{
				fortempvec.push_back(temp);
			}
			else if (!finalAlpha[i][j])
			{
				backtempvec.push_back(temp);
			}
		}
	}

	vector< vector<colorNode> > forvec;
	vector< vector<colorNode> > backvec;

	forvec.push_back(fortempvec);
	backvec.push_back(backtempvec);

	cluster(forvec);
	cluster(backvec);
	initGMM(forvec,fgmm);
	initGMM(backvec,bgmm);

}