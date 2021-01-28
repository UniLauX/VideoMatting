#include "RefineWithTrimap.h"
#include "../grab/BiImageProcess.h"
#include <vector>
#include "../VideoCut/LocalClassifier.h"
using namespace std;

RefineWithTrimap::RefineWithTrimap(void)
:m_winsize(10)
{
}

RefineWithTrimap::~RefineWithTrimap(void)
{

}

void RefineWithTrimap::Refine(CxImage& srcimg, CxImage& labelimg, CxImage& trimapimg)
{
	tempimg.Copy(srcimg);
	VVPOINTS center_vec;
	int width = srcimg.GetWidth();
	int height = srcimg.GetHeight();
	int* label = new int[width*height];
	memset(label, 0, sizeof(int)*srcimg.GetWidth()*srcimg.GetHeight());
	CxImage labelfromtrimap;
	labelfromtrimap.Copy(labelimg);
	for (int i=0;i<width;++i)
	{
		for (int j=0; j<height;++j)
		{
			if (trimapimg.GetPixelGray(i,j)==0)
			{
				labelfromtrimap.SetPixelColor(i,j,0);
			}
			label[j*width+i] = labelfromtrimap.GetPixelGray(i,j)/255;
		}
	}
	BiImageProcess::GetAllWindowsCenters(center_vec, label, width, height,this->m_winsize);

	vector<Local_Win> window_vec;
	for (int i=0; i<center_vec.size();++i)
	{
		for (int j=0; j<center_vec[i].size();++j)
		{
			Local_Win tempWin;
			tempWin.centerX = center_vec[i][j].x;
			tempWin.centerY = center_vec[i][j].y;
			tempWin.win_size = this->m_winsize;
			window_vec.push_back(tempWin);
		}
	}

	for (int i=0; i<window_vec.size(); ++i)
	{
		Refinewindow(srcimg, labelimg, trimapimg, window_vec[i]);
	}
	printf("%d\n", window_vec.size());
	tempimg.Save("H:/test.pro.png", CXIMAGE_FORMAT_PNG);

}
void RefineWithTrimap::Refinewindow(CxImage& srcimg, CxImage& labelimg, CxImage& trimapimg, Local_Win win)
{
		printf("In Refinewindow\n");
		int width = srcimg.GetWidth();
		int height=srcimg.GetHeight();
		vector<colorNode> fortempvec;
		vector<colorNode> backtempvec;
		GMMNode fgmm[3];
		GMMNode bgmm[3];
		int x1 = (win.centerX-win.win_size/2>=0)?(win.centerX-win.win_size/2):0;
		int x2 = (win.centerX+win.win_size/2<width)?(win.centerX+win.win_size/2):width-1;

		int y1 = (win.centerY-win.win_size/2>=0)?(win.centerY-win.win_size/2):0;
		int y2 = (win.centerY+win.win_size/2<height)?(win.centerY+win.win_size/2):height-1;
		int winWidth = x2-x1+1;
		int winHeight = y2-y1+1;
		
		for (int i=x1;i<=x2;++i)
		{
			for (int j=y1;j<=y2;++j)
			{
				colorNode temp;
				RGBQUAD rgb=srcimg.GetPixelColor(i,j,true);
				temp.indexX=i;
				temp.indexY=j;
				temp.colorinfo.resize(3,1);
				temp.colorinfo(0,0)=(double)rgb.rgbRed;
				temp.colorinfo(1,0)=(double)rgb.rgbGreen;
				temp.colorinfo(2,0)=(double)rgb.rgbBlue;
				if (/*labelimg.GetPixelGray(i,j)==0&&*/trimapimg.GetPixelGray(i,j)==0)
				{
					backtempvec.push_back(temp);
				}
				else if (/*labelimg.GetPixelGray(i,j)==255&&*/trimapimg.GetPixelGray(i,j)==255)
				{
					fortempvec.push_back(temp);
				}
			}
		}
		vector< vector < colorNode > > forvec;
		vector< vector < colorNode > > backvec;
		forvec.push_back(fortempvec);
		backvec.push_back(backtempvec);
		cluster(forvec,2);
		cluster(backvec,2);
		initGMM(forvec,fgmm);
		initGMM(backvec,bgmm);
		printf("After Init GMM\n");

		Graph* graph=new Graph();
		Graph::node_id** gnode;
		gnode = new Graph::node_id *[winWidth];
		for (int i=0; i<winWidth; ++i)
			gnode[i] = new Graph::node_id[winHeight];

		for (int i=x1;i<=x2;++i)
		{
			for (int j=y1;j<=y2;++j)
			{
				gnode[i-x1][j-y1]=graph->add_node();
			}
		}		

		for (int i=x1; i<=x2;++i)
		{
			for(int j=y1;j<=y2; ++j)
			{
				float forposs=0, backposs=0;
				if(trimapimg.GetPixelGray(i,j)==0)//back
				{
					forposs=0;
					backposs=4501;//unsure
				}
				else if (trimapimg.GetPixelGray(i,j)==255)
				{
					forposs = 4501;
					backposs=0;
				}
				else
				{
					for (int itemp=0;itemp<3;itemp++)
					{
						if (bgmm[itemp].count)
						{
							backposs+=bgmm[itemp].pi*calposs(bgmm[itemp],srcimg.GetPixelColor(i,j));
							//cout<<"PI: "<<bgmm[itemp].pi<<endl;
						}
						else
							continue;	
					}
					//cout<<"backposs:"<<backposs<<endl;
					backposs=log(backposs);
					for (int jtemp=0;jtemp<3;jtemp++)
					{
						if (fgmm[jtemp].count)
						{
							forposs+=fgmm[jtemp].pi*calposs(fgmm[jtemp],srcimg.GetPixelColor(i,j));			
						}
						else
							continue;	
					}
					forposs=log(forposs);
				}
					graph->add_tweights(gnode[i-x1][j-y1],forposs,backposs);
					//tempimg.SetPixelColor(i,j, RGB((int)(forposs/(backposs+forposs)*255),(int)(forposs/(backposs+forposs)*255),(int)(forposs/(backposs+forposs)*255)));
			}
		}// end of construct graph
		NWeight** nweight = new NWeight*[winWidth];
		for (int i=0; i<winWidth; ++i)
		{
			nweight[i] = new NWeight[winHeight];
		}
		double beta = this->calBeta(srcimg, x1, x2, y1, y2);
		this->calculateNweight(nweight, x1, x2, y1, y2, srcimg, beta);

		for (int i=x1;i<=x2;++i)
		{
			for (int j=y1;j<=y2;++j)
			{
				if (i>=0&&j+1<y2)
				{
					graph->add_edge(gnode[i-x1][j-y1],gnode[i-x1][j+1-y1],nweight[i-x1][j-y1].up*0.3,nweight[i-x1][j-y1].up*0.3);
				}
				if (i>=0&&j-1>y1)
				{
					graph->add_edge(gnode[i-x1][j-y1],gnode[i-x1][j-1-y1],nweight[i-x1][j-y1].down*0.3,nweight[i-x1][j-y1].down*0.3);
				}
				if (j<=y2&&i-1>=x1)
				{
					graph->add_edge(gnode[i-x1][j-y1],gnode[i-1-x1][j-y1],nweight[i-x1][j-y1].left*0.3,nweight[i-x1][j-y1].left*0.3);
				}
				if (j<=y2&&i+1<=x2)
				{
					graph->add_edge(gnode[i-x1][j-y1],gnode[i+1-x1][j-y1],nweight[i-x1][j-y1].right*0.3,nweight[i-x1][j-y1].right*0.3);
				}
			}
		}// end of N-Link
		graph->maxflow();
		printf("After construct graph\n");
		for(int i=x1; i<=x2;++i)
			for (int j=y1;j<=y2;++j)
			{
				if (graph->what_segment(gnode[i-x1][j-y1])==Graph::SOURCE)
				{
					labelimg.SetPixelColor(i,j, RGB(255,255,255));
				}
				else
				{
					labelimg.SetPixelColor(i,j, RGB(0,0,0));
				}
			}
			delete graph;
			for (int i=0; i<winWidth; ++i)
			{
				delete[] gnode[i];
				delete[] nweight[i];
			}
			delete[] gnode;
			delete[] nweight;
		printf("out of Refinewindow\n");
}
double RefineWithTrimap::calposs( GMMNode& gmm,RGBQUAD rgb)
{
	int z[3];
	//RGBQUAD rgb=colimage.GetPixelColor(x,y,true);
	z[0]=(double)rgb.rgbRed;
	z[1]=(double)rgb.rgbGreen;
	z[2]=(double)rgb.rgbBlue;

	double temp[3];

	double temp2[3];

	temp[0]=z[0]-gmm.u(0,0);
	temp[1]=z[1]-gmm.u(1,0);
	temp[2]=z[2]-gmm.u(2,0);

	temp2[0]=gmm.inverseConviance(0,0)*temp[0]+gmm.inverseConviance(0,1)*temp[1]+gmm.inverseConviance(0,2)*temp[2];
	temp2[1]=gmm.inverseConviance(1,0)*temp[0]+gmm.inverseConviance(1,1)*temp[1]+gmm.inverseConviance(1,2)*temp[2];
	temp2[2]=gmm.inverseConviance(2,0)*temp[0]+gmm.inverseConviance(2,1)*temp[1]+gmm.inverseConviance(2,2)*temp[2];


	if (gmm.pi>0)
	{
		if (gmm.det>0)
		{
			double mi=0.5*(temp[0]*temp2[0]+temp[1]*temp2[1]+temp[2]*temp2[2]);

			double leftr=sqrt(abs(gmm.det))/gmm.pi;
			return (1.0/(sqrt(abs(gmm.det)))*Got_Exp(-mi));
			//return Got_Exp(-0.5*mi/(sqrt(abs(fgmm[gmmindex].det))));
		}
		return 0;

	}

	return 0;
}

void RefineWithTrimap::calculateNweight(NWeight** nweight, int x1, int x2, int y1, int y2, CxImage& srcimg, double beta)
{
	int width = srcimg.GetWidth();
	int height = srcimg.GetHeight();

	for (int i=x1;i<=x2;++i)
	{
		for (int j=y1;j<=y2;++j)
		{
			if (i>=0&&j+1<height)
			{
				double distanceE=distance(i,j,i,j+1);
				double discol=coldistance(srcimg.GetPixelColor(i,j,true),srcimg.GetPixelColor(i,j+1,true));
				nweight[i-x1][j-y1].up=calN(distanceE,discol, beta);
			}
			if (i>=0&&j-1>=0)
			{
				double distanceE=distance(i,j,i,j-1);
				double discol=coldistance(srcimg.GetPixelColor(i,j,true),srcimg.GetPixelColor(i,j-1,true));
				nweight[i-x1][j-y1].down=calN(distanceE,discol, beta);
			}
			if (i-1>=0&&j<height)
			{
				double distanceE=distance(i,j,i-1,j);
				double discol=coldistance(srcimg.GetPixelColor(i,j,true),srcimg.GetPixelColor(i-1,j,true));
				nweight[i-x1][j-y1].left=calN(distanceE,discol, beta);
			}
			if (i+1<width&&j<height)
			{
				double distanceE=distance(i,j,i+1,j);
				double discol=coldistance(srcimg.GetPixelColor(i,j,true),srcimg.GetPixelColor(i+1,j,true));
				nweight[i-x1][j-y1].right=calN(distanceE,discol, beta);
			}
		}
	}
}
double RefineWithTrimap::distance(int x1,int y1,int x2,int y2)
{
	double dis=(x1-x2)*(x1-x2)+(y1-y2)*(y1-y2);
	return sqrt(dis);
}

double  RefineWithTrimap::coldistance(RGBQUAD c1,RGBQUAD c2)
{
	return (c1.rgbRed-c2.rgbRed)*(c1.rgbRed-c2.rgbRed)+(c1.rgbGreen-c2.rgbGreen)*(c1.rgbGreen-c2.rgbGreen)+(c1.rgbBlue-c2.rgbBlue)*(c1.rgbBlue-c2.rgbBlue);
}

double RefineWithTrimap::calN(double distance, double coldis, double beta)
{
	return 50*Got_Exp(-beta*coldis)/distance;
}

double RefineWithTrimap::calBeta(CxImage& colimage, int x1, int x2, int y1, int y2)
{
	double total=0.0;
	int num=0;
	int width = colimage.GetWidth();
	int height = colimage.GetHeight();
	for (int i=x1;i<=x2;++i)
	{
		for (int j=y1;j<=y2;++j)
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