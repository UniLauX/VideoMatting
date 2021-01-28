
#include "BiImageProcess.h"
#include <cmath>
bool BiImageProcess::isValid(int x,int y,int w,int h)
{
	return (x>=0&&x<w&&y>=0&&y<h);
}

bool BiImageProcess::neighbourNCaled(int i,int j,int w,int h,int **P)
{
	int x=i;
	int y=j;
	int temp1=0,temp2=0,temp3=0,temp4=0;

	if (x>1&&x<w-1&&y>1&&y<h-1)
	{
		if(P[x-1][y])
			temp1=1;
		if(P[x][y-1])
			temp2=1;
		if(P[x-1][y-1])
			temp3=1;
		if (P[x-1][y+1])
			temp4=1;
	}
	else
	{
		if (isValid(x-1,y,w,h))
		{
			if(P[x-1][y])
				temp1=1;
		}
		if (isValid(x,y-1,w,h))
		{
			if(P[x][y-1])
				temp2=1;
		}

		if (isValid(x-1,y-1,w,h))
		{
			if(P[x-1][y-1])
				temp3=1;
		}

		if (isValid(x-1,y+1,w,h))
		{
			if (P[x-1][y+1])
				temp4=1;
		}

	}
	
	return (temp1||temp2||temp3||temp4);
}

int BiImageProcess::NeighbourMax(int i,int j,int w,int h,int **P)
{
	int temp1=0,temp2=0,temp3=0,temp4=0;

	if (i>1&&i<w-1&&j>1&&j<h-1)
	{
				temp1=P[i-1][j];
				temp2=P[i-1][j-1];
				temp3=P[i-1][j+1];
				temp4=P[i][j-1];
	}
	else
	{
		if(isValid(i-1,j,w,h))
			temp1=P[i-1][j];

		if (isValid(i-1,j-1,w,h))
			temp2=P[i-1][j-1];

		if (isValid(i-1,j+1,w,h))
			temp3=P[i-1][j+1];

		if(isValid(i,j-1,w,h))
			temp4=P[i][j-1];
	}
	return _MAX(temp4,_MAX(temp3,_MAX(temp2,temp1)));
}

void BiImageProcess::UpdateCoexists(int i,int j,int w,int h,int **P,int **coexist)
{

	if (i>1&&i<w-1&&j>1&&j<h-1)
	{
		coexist[P[i][j]][P[i-1][j]]=1;
		coexist[P[i-1][j]][P[i][j]]=1;
		coexist[P[i][j]][P[i-1][j-1]]=1;
		coexist[P[i-1][j-1]][P[i][j]]=1;
		coexist[P[i][j]][P[i-1][j+1]]=1;
		coexist[P[i-1][j+1]][P[i][j]]=1;
		coexist[P[i][j]][P[i][j-1]]=1;
		coexist[P[i][j-1]][P[i][j]]=1;
	}

	else
	{

		if(isValid(i-1,j,w,h))
		{
			coexist[P[i][j]][P[i-1][j]]=1;
			coexist[P[i-1][j]][P[i][j]]=1;
		}
		if (isValid(i-1,j-1,w,h))
		{
			coexist[P[i][j]][P[i-1][j-1]]=1;
			coexist[P[i-1][j-1]][P[i][j]]=1;
		}
		if (isValid(i-1,j+1,w,h))
		{
			coexist[P[i][j]][P[i-1][j+1]]=1;
			coexist[P[i-1][j+1]][P[i][j]]=1;

		}
		if(isValid(i,j-1,w,h))
		{
			coexist[P[i][j]][P[i][j-1]]=1;
			coexist[P[i][j-1]][P[i][j]]=1;

		}

	}
}

double BiImageProcess::KernelBSpline(const float x)
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

bool BiImageProcess::BSInterpolation(float **alphaSource, float **alphaDes, int oldX, int oldY, int newX, int newY)
{

	if (newX<=0||newY<=0)
	{
		return false;
	}

	if (newX==oldX&&newY==oldY)
	{
		memcpy(alphaDes,alphaSource,sizeof(float)*oldX*oldY);
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



void BiImageProcess::checkConnectivity(float **llabel, int w, int h)
{
	int N=0;
	int **P;
	int **coexist;
	int *num;

	P=new int *[w];
	coexist=new int*[NMAX];
	num=new int[NMAX];

	memset(num,0,sizeof(int)*NMAX);

	for (int i=0;i<w;++i)
	{
		P[i]=new int[h];

		memset(P[i],0,sizeof(int)*h);
	}

	for (int i=0;i<NMAX;++i)
	{
		coexist[i]=new int [NMAX];
		memset(coexist[i],0,sizeof(int)*NMAX);
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


void BiImageProcess::checkConnectivityfast(unsigned char* l, int w, int h)
{
	KConnectedComponentLabeler* ccl=new KConnectedComponentLabeler;
	ccl->SetMask( l,w,h); // mask to be processed
	ccl->CheckConnectivity(l);
	ccl->Clear();
	delete ccl;
}
void BiImageProcess::fillHoles(unsigned char* label , int w, int h, int minarea)
{
	int N=0;
	int **P;
	int **coexist;
	int *num;
	P=new int *[w];
	coexist=new int*[NMAX];
	num=new int[NMAX];
	memset(num,0,sizeof(int)*NMAX);
	for (int i=0;i<w;++i)
	{
		P[i]=new int[h];
		memset(P[i],0,sizeof(int)*h);
	}
	for (int i=0;i<NMAX;++i)
	{
		coexist[i]=new int [NMAX];
		memset(coexist[i],0,sizeof(int)*NMAX);
	}
	int index=0;
	//for (int i=2;i<w-1;++i)
	//{
	//	for (int j=2;j<h-1;++j)
	//	{
	//		index=j*w+i;
	//		if (!label[index])
	//		{
	//			if (label[j*w+i-1]&&label[(j-1)*w+i]&&label[j*w+i+1]&&label[(j+1)*w+i])
	//				label[index]=1;
	//		}
	//	}
	//}

	for (int i=0;i<w;++i)//label object
	{
		for (int j=0;j<h;++j)
		{
			index=j*w+i;
			if (!label[index])
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
	for (int i=1;i<N;++i)
	{
		if (included[i]==2)
			if (num[i]<minarea)
				for (int j=0;j<w;++j)
					for (int k=0;k<h;++k)
					{
						index=k*w+j;
						if (!label[index])
							if (find(list1[i].begin(),list1[i].end(),P[j][k])!=list1[i].end())
								label[index]=1;
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

void BiImageProcess::checkConnectivity(unsigned char *label, int w, int h, int minarea)
{
	int N=0;
	int **P;
	int **coexist;
	int *num;
	P=new int *[w];
	coexist=new int*[NMAX];
	num=new int[NMAX];
	memset(num,0,sizeof(int)*NMAX);
	for (int i=0;i<w;++i)
	{
		P[i]=new int[h];
		memset(P[i],0,sizeof(int)*h);
	}

	for (int i=0;i<NMAX;++i)
	{
		coexist[i]=new int [NMAX];
		memset(coexist[i],0,sizeof(int)*NMAX);
	}
	int index=0;
	//for (int i=2;i<w-1;++i)
	//{
	//	for (int j=2;j<h-1;++j)
	//	{
	//		index=j*w+i;
	//		if (label[index])
	//		{
	//			if (!label[j*w+i-1]&&!label[(j-1)*w+i]&&!label[j*w+i+1]&&!label[(j+1)*w+i])
	//				label[index]=0;
	//		}
	//	}
	//}

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

	for (int i=1;i<N;++i)
	{
		if (included[i]==2)
		{
			if (num[i]<minarea)
			{
				for (int j=0;j<w;++j)
				{
					for (int k=0;k<h;++k)
					{
						index=k*w+j;
						if (label[index])
						{
							if (find(list1[i].begin(),list1[i].end(),P[j][k])!=list1[i].end())
							{
								label[index]=0;
							}
						}
					}
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

void BiImageProcess::checkConnectivity(CxImage &label)
{
	int w,h;
	w=label.GetWidth();
	h=label.GetHeight();

	unsigned char *l=new unsigned char[w*h];
	for (int j=0;j<h;++j)
	{
		for (int i=0;i<w;++i)
		{
			RGBQUAD rgb=label.GetPixelColor(i,j);
			if (rgb.rgbBlue==255)
				l[j*w+i]=1;
			else
			l[j*w+i]=0;
		}
	}

	checkConnectivity(l,w,h);

	label.Clear(0);

	RGBQUAD white;
	white.rgbBlue=white.rgbGreen=white.rgbRed=255;
	for (int j=0;j<h;++j)
	{
		for (int i=0;i<w;++i)
		{
			if (l[j*w+i])
			{
				label.SetPixelColor(i,j,white);
			}
		}
	}
}


void BiImageProcess::manhattan(float **label, int w, int h)
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

void BiImageProcess::dilate(float** label,int w,int h,int k)
{
	manhattan(label,w,h);

	for (int i=0;i<w;++i)
	{
		for (int j=0;j<h;++j)
		{
			label[i][j]=((label[i][j]<k)?1:0);
		}
	}
}

void BiImageProcess::erode(float**label,int w,int h,int k)
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


void BiImageProcess::getBound(std::vector<POINT> &bound, int **label,  int xStart, int yStart,int xEnd,int yEnd)
{
	cout<<"in bound"<<endl;
	POINT p;

	bound.clear();
	for (int i=xStart;i<xEnd;++i)
	{
		for (int j=yStart;j<yEnd;++j)
		{
			if (label[i][j]==1)
			{
				if (i-1>=0)
					if (!label[i-1][j])
					{
						p.x=i;
						p.y=j;
						bound.push_back(p);
						continue;
					}

					if (i+1<xEnd)
						if (!label[i+1][j])
						{
							p.x=i;
							p.y=j;
							bound.push_back(p);
							continue;							
						}

						if(j-1>=0)
							if (!label[i][j-1])
							{
								p.x=i;
								p.y=j;
								bound.push_back(p);
								continue;						
							}

							if(j+1<yEnd)
								if (!label[i][j+1])
								{
									p.x=i;
									p.y=j;
									bound.push_back(p);
									continue;
								}

			}
		}
	}
	std::cout<<"out of bound"<<std::endl;
}

void BiImageProcess::getBound(std::vector<POINT> &bound,unsigned char *label,int xStart,int yStart,int xEnd,int yEnd,int w,int h)
{
	POINT p;
	bound.clear();

	for (int i=xStart;i<xEnd;++i)
	{
		for (int j=yStart;j<yEnd;++j)
		{
			if (label[j*w+i]==1)
			{
				if (i-1>=0)
					if (!label[j*w+i-1])
					{
						p.x=i;
						p.y=j;
						bound.push_back(p);
						continue;
					}

					if (i+1<w)
						if (!label[j*w+i+1])
						{
							p.x=i;
							p.y=j;
							bound.push_back(p);
							continue;							
						}

						if(j-1>=0)
							if (!label[(j-1)*w+i])
							{
								p.x=i;
								p.y=j;
								bound.push_back(p);
								continue;						
							}

							if(j+1<h)
								if (!label[(j+1)*w+i])
								{
									p.x=i;
									p.y=j;
									bound.push_back(p);
									continue;
								}

			}
		}
	}

}

void BiImageProcess::getBound(std::vector<POINT> &bound,CxImage* image)
{
	POINT p;
	bound.clear();

	int w=image->GetWidth();
	int h=image->GetHeight();

	RGBQUAD r1,r2;
	for (int j=0;j<h;++j)
	{
		for (int i=0;i<w;++i)
		{
			r1=image->GetPixelColor(i,j);
			if (r1.rgbBlue==255)
			{
				if (i-1>=0)
				{
					r2=image->GetPixelColor(i-1,j);
					if (r2.rgbBlue!=255)
					{
						p.x=i;
						p.y=j;
						bound.push_back(p);
						continue;
					}
				}

				if (i+1<w)
				{
					r2=image->GetPixelColor(i+1,j);
					if (r2.rgbBlue!=255)
					{
						p.x=i;
						p.y=j;
						bound.push_back(p);
						continue;
					}
				}

				if (j-1>=0)
				{
					r2=image->GetPixelColor(i,j-1);
					if (r2.rgbBlue!=255)
					{
						p.x=i;
						p.y=j;
						bound.push_back(p);
						continue;
					}
				}
				if (j+1<h)
				{
					r2=image->GetPixelColor(i,j+1);
					if (r2.rgbBlue!=255)
					{
						p.x=i;
						p.y=j;
						bound.push_back(p);
						continue;
					}
				}

			}
		}
	}
}

void BiImageProcess::getBound_ex(std::vector<POINT> &bound, CxImage* image)
{
	getBound(bound, image);
	int width = image->GetWidth(); 
	int height = image->GetHeight();

	for (int i=0; i<width; ++i)
	{
		RGBQUAD rgb1 = image->GetPixelColor(i,0);
		RGBQUAD rgb2 = image->GetPixelColor(i, height-1);
		if (rgb1.rgbBlue==255)
		{
			POINT p;
			p.x = i;
			p.y = 0;
			bound.push_back(p);
		}
		if (rgb2.rgbBlue==255)
		{
			POINT p;
			p.x = i;
			p.y = height-1;
			bound.push_back(p);
		}
	}

	for (int j=0; j<height; ++j)
	{
		RGBQUAD rgb1 = image->GetPixelColor(0,j);
		RGBQUAD rgb2 = image->GetPixelColor(width-1, j);
		if (rgb1.rgbBlue==255)
		{
			POINT p;
			p.x = 0;
			p.y = j;
			bound.push_back(p);
		}
		if (rgb2.rgbBlue==255)
		{
			POINT p;
			p.x = width-1; 
			p.y = j;
			bound.push_back(p);
		}
	}
}

void BiImageProcess::GetTrimap(unsigned char *soreceLabel,CxImage &trimap,int dis,int w,int h)
{
	vector<POINT> bound;
	getBound(bound,soreceLabel,0,0,w,h,w,h);
	int size=(int)bound.size();
	int x, y;
	for (int i=0;i<size;++i)
	{
		x=bound[i].x;
		y=bound[i].y;

		for (int j=x-dis;j<=x+dis;++j)
		{
			for (int k=y-dis;k<=y+dis;++k)
			{
				if(k>=0&&k<h&&j>=0&&j<w)
				if(trimap.GetPixelGray(j,k)==0)
				{
					if (distance(x,y,j,k)<=dis)
						trimap.SetPixelIndex(j,k,128);
				}
			}
		}
	}
}

void BiImageProcess::GetTrimap(CxImage* label,CxImage& trimap,int dis)
{
	vector<POINT> bound;

	getBound(bound,label);

	int w=label->GetWidth();
	int h=label->GetHeight();

	int size=(int)bound.size();

	int x,y;
	RGBQUAD rgb;
	rgb.rgbBlue=rgb.rgbGreen=rgb.rgbRed=128;
	for (int i=0;i<size;++i)
	{
		x=bound[i].x;
		y=bound[i].y;

		for (int j=x-dis;j<=x+dis;++j)
		{
			for (int k=y-dis;k<=y+dis;++k)
			{
				if (k>=0&&k<h&&j>=0&&j<w)
				{
					if (distance(x,y,j,k)<dis)
					{
						trimap.SetPixelColor(j,k,rgb);
					}
				}
			}
		}
		
	}


}
float BiImageProcess::distance(int x1,int y1,int x2,int y2)
{
	return sqrtf((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}

void BiImageProcess::FindBoundBox(CxImage* label,int& left,int &top,int &right,int &bottom)
{
	int w,h;
	w=label->GetWidth();
	h=label->GetHeight();

	int xmin=w;
	int xmax=-1;
	int ymin=h;
	int ymax=-1;

	RGBQUAD rgb;
	for (int i=0;i<w;++i)
	{
		for (int j=0;j<h;++j)
		{
			rgb=label->GetPixelColor(i,j);
			if (rgb.rgbBlue)
			{
				xmin=__min(xmin,i);
				xmax=__max(xmax,i);
				ymin=__min(ymin,j);
				ymax=__max(ymax,j);

			}
		}
	}


	left=((xmin-10)>0)?(xmin-10):0;
	right=(xmax+10<w)?(xmax+10):w;
	top=(ymax+10<h)?(ymax+10):h;
	bottom=(ymin-10>0)?(ymin-10):0;
}

void BiImageProcess::GetAllWindowsCenters(VVPOINTS &vec,int *label,int width,int height,int localWinSize)// label 0 or 1
{
	int i,j;
	int index;
	int *flag;
	unsigned char **alpha;
	vector<POINT>::iterator ibp;
	vector<POINT> boundPoints;
	vector<POINT> vOrder;
	vector<POINT> vCenter;
	vector<vector<POINT>> cb;         //每一个center对应的window内包含的边界点 index与vcenter对应

	int pixelNum=width*height;

	flag=new int[pixelNum];
	alpha=new unsigned char *[width];
	for (i=0;i<width;++i)
	{
		alpha[i]=new unsigned char[height];
		for (j=0;j<height;++j)
		{
			alpha[i][j]=label[j*width+i];
		}
	}

	memset(flag, 0, sizeof(int)*pixelNum);
	GetBound(label,flag, boundPoints,width,height);
	cout<<"after GetBound"<<endl;
	for (ibp=boundPoints.begin();ibp!=boundPoints.end();++ibp)
	{
		index=(*ibp).y*width+(*ibp).x;
		if (flag[index])
		{
			vOrder.clear();
			vCenter.clear();
			cb.clear();
			//GetOrderedBound(*ibp, alpha,  vOrder,width,height, flag, width, height);
			GetOrderedBound(*ibp, alpha, flag, vOrder, boundPoints.size(), width, height);
			if (vOrder.size())
			{
				GetWindowCenter(vOrder, vCenter, cb,localWinSize);
				if (vCenter.size()>=2)
				{
					vec.push_back(vCenter);
				}
			}
		}
	}
	for (i=0;i<width;++i)
	{
		delete [] alpha[i];
	}
	delete [] alpha;
	delete [] flag;
}

void BiImageProcess::GetBound(int *label,int *flag, VPOINTS &points,int width,int height)
{     
	int i,j,k;
	int x,y;
	int index1,index2;
	POINT point;
	int direct[4][2]={{1,0},{0,1},{-1,0},{0,-1}};
	for(i=0;i<height;++i)
	{	
		for(j=0;j<width;++j)
		{
			index1=i*width+j;
			if (label[index1])
			{			
				for(k=0;k<4;++k)
				{	
					y=i+direct[k][0];
					x=j+direct[k][1];
					index2=y*width+x;
					if(x>=0 && x<width && y>=0 && y<height)
					{
						if(label[index1]!=label[index2])
						{
							point.x=j;
							point.y=i;
							points.push_back(point);
							flag[index1]=1;
							break;
						}
					}
				}
			}
		}
	}
}

void BiImageProcess::GetOrderedBound(const POINT &sPoint, unsigned char **alpha, int *flag, VPOINTS &points, int num, int width, int height)
{	
	POINT point;
	int x0,y0;//first point
	int x1(-2),y1(-2);//second point
	int xc(-1),yc(-1);//current border point 
	int xp(-1),yp(-1);//previous border point
	int dir(7);
	int count(0);
	int mm = num;
	int cc(0);
	int xtemp,ytemp;
	RGBQUAD r={255,0,0,0};
	//find the start point
	x0=sPoint.x;
	y0=sPoint.y;
	xc=sPoint.x;
	yc=sPoint.y;
	do
	{
		--mm;
		count++;
		point.x=xc;
		point.y=yc;
		if (flag[yc*width+xc])
		{
			flag[yc*width+xc]=0;
			cc++;
			points.push_back(point);
		}
		if (dir & 1)
		{
			dir=(dir+6) & 7;
		}
		else
			dir=(dir+7) & 7;
		for (int p=0;p<8;++p)
		{
			if (GetNeighbourValue(xc,yc,dir,xtemp,ytemp,alpha, width, height))
			{
				xp=xc;
				yp=yc;
				xc=xtemp;
				yc=ytemp;
				if (count==1)
				{
					x1=xc;
					y1=yc;
				}
				break;
			}
			else
			{
				dir++;
				dir &=7;
			}
		}
	} while (mm);

}

int BiImageProcess::GetNeighbourValue(int x, int y, int dir, 
			       int &xtemp, int &ytemp, unsigned char **alpha,int width,int height)
{
	static int direction[8][2]={{1,0},{1,1},{0,1},{-1,1},
	{-1,0},{-1,-1},{0,-1},{1,-1}};

	if (!(x>=0 && x<width && y>=0 && y<height))
		return 0;

	xtemp=x+direction[dir][0];
	ytemp=y+direction[dir][1];
	if (xtemp>=0&&xtemp<width&&ytemp>=0&&ytemp<height)
	{
		if (alpha[xtemp][ytemp]==1)
			return 1;
		else
			return 0;
	}
	else
		return 0;
}
void BiImageProcess::GetWindowCenter(const VPOINTS &bound, VPOINTS &centers, vector<VPOINTS> &cb,int localWinSize)
{
	if (!bound.size())
	{
		centers.clear();
		return;
	}
	vector<POINT>::const_iterator ib;
	POINT firstPoint(bound[0]);
	POINT currentPoint(firstPoint);
	double dis=(2.0/3)*localWinSize;               //1/3 overlap

	for(ib=bound.begin()+1;ib!=bound.end();++ib)
	{
		if((dis-GetDistance(currentPoint, *ib))<0.5)
		{
			currentPoint=*ib;
			centers.push_back(currentPoint);
		}
	}
	if (centers.size()<4)
		return;

	int count(0);
	vector<POINT> c1;
	vector<POINT> c2;
	vector<POINT>::iterator ic;
	vector<POINT>::iterator ic2;

	ib=bound.begin();
	ic=centers.begin();
	ic2=ic+1;
	while (ic!=centers.end() && ic2!=centers.end())
	{
		while ((*ib).x!=(*ic2).x || (*ib).y!=(*ic2).y)
		{
			if (IsInWindow(*ib,*ic,localWinSize))
			{
				c1.push_back(*ib);
			}
			if (IsInWindow(*ib,*ic2,localWinSize))
			{
				c2.push_back(*ib);
			}
			++ib;
		}
		++count;
		if (count & 1)
		{
			ic=ic2+1;
			cb.push_back(c1);
			c1.clear();
		}
		else
		{
			ic2=ic+1;
			cb.push_back(c2);
			c2.clear();
		}
	}

	if (ic==centers.end())
	{
		while (ib!=bound.end())
		{
			if (IsInWindow(*ib,*ic2,localWinSize))
			{
				c2.push_back(*ib);
			}
			if (IsInWindow(*ib,*(centers.begin()),localWinSize))
			{
				cb[0].push_back(*ib);
			}
			++ib;
		}
		cb.push_back(c2);
	}
	else
	{
		while (ib!=bound.end())
		{
			if (IsInWindow(*ib,*ic,localWinSize))
			{
				c1.push_back(*ib);
			}
			if (IsInWindow(*ib,*(centers.begin()),localWinSize))
			{
				cb[0].push_back(*ib);
			}
			++ib;
		}
		cb.push_back(c1);
	}
}


double BiImageProcess::GetDistance(const POINT &p1, const POINT &p2)
{
	return sqrt((double)(p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y));
}
bool BiImageProcess::IsInWindow(const POINT &point, const POINT &center, int size)
{
	return ((point.x<center.x+size/2) && (point.x>center.x-size/2) &&
		(point.y<center.y+size/2) && (point.y>center.y-size/2));	
}

bool BiImageProcess::checkNeighbour(int x, int y, unsigned char** mask)
{
	int sum = 0;
	sum+=mask[x-1][y+1]+mask[x][y+1]+mask[x+1][y+1]+mask[x][y-1]+mask[x][y+1]+mask[x-1][y-1]+mask[x-1][y]+mask[x-1][y+1];
	return (sum==8);
}