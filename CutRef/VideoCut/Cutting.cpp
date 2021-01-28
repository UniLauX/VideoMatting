#include "Cutting.h"
#include "../CutRef/Global.h"
#include "../include/ANN/ANN.h"
#include <vector>
#include <map>
#include <string>
#include <atlstr.h>
using namespace std;

GrabCut_ywz::GrabCut_ywz():
    bgCluster(NULL),
    fgCluster(NULL),
    label(NULL),
    info(NULL),
    prob(NULL),
    unkownWidth(15),
    bgWidth(30)
{

}

GrabCut_ywz::~GrabCut_ywz()
{
    Clear();
}

void GrabCut_ywz::Init(const CxImage &img, const int *l)
{
    int w,h;

    Clear();
    image.Copy(img);
    w=img.GetWidth();
    h=img.GetHeight();
    label=new int[w*h];
    info=new int[w*h];
    prob=new float[w*h];
    memcpy(label, l, sizeof(int)*w*h);
    GetCutInfo(info);
    InitGMM(info);
    GetProb(prob);
}

void GrabCut_ywz::Init(const CxImage &img, const int *l, const float *P)
{
    int n;

    Clear();
    image.Copy(img);
    n=image.GetWidth()*image.GetHeight();
    label=new int[n];
    info=new int[n];
    prob=new float[n];
    memcpy(label, l, sizeof(int)*n);
    memcpy(prob, P, sizeof(float)*n);
    GetCutInfo(info);
    for(int i=0;i<n;++i)
    {
        if(info[i]!=2)
            prob[i]=(float)label[i];
    }
}

void GrabCut_ywz::Clear()
{
    delete [] label;
    delete [] info;
    delete [] prob;
    delete bgCluster;
    delete fgCluster;
    label = NULL;
    info = NULL;
    prob = NULL;
    bgCluster = NULL;
    fgCluster = NULL;
}

static void GetBound(int w, int h, const int *label, vector<POINT> &boundPts)
{
    POINT pt;
    int i,j,k;
    int x,y,a;
    int direct[4][2]={{1,0},{0,1},{-1,0},{0,-1}};

    boundPts.clear();
    for(i=0;i<h;++i)
    {
        for(j=0;j<w;++j)
        {
            a=label[j+i*w];
            if(a==1)
            {			
                for(k=0;k<4;++k)
                {	
                    y=i+direct[k][0];
                    x=j+direct[k][1];
                    if(x>=0 && x<w && y>=0 && y<h)
                    {
                        if(a!=label[x+y*w])
                        {
                            pt.x=j;
                            pt.y=i;
                            boundPts.push_back(pt);
                            break;
                        }
                    }
                }
            }
        }
    }
}

void GrabCut_ywz::GetCutInfo(int *info)
{
    int w,h,i,j,k;
    vector<POINT> boundPts;
    ANNpointArray pts;
    ANNkd_tree *  tree;
    ANNpoint      pt;
    ANNidxArray   idx;
    ANNdistArray  dist;

    w=image.GetWidth();
    h=image.GetHeight();
    GetBound(w, h, label, boundPts);

    pts=annAllocPts((int)boundPts.size(), 2);
    for(i=0;i<(int)boundPts.size();++i)
    {
        pts[i][0]=boundPts[i].x;
        pts[i][1]=boundPts[i].y;
    }
    
    tree=new ANNkd_tree(pts, (int)boundPts.size(), 2);
    pt=annAllocPt(2);
    idx=new ANNidx[1];
    dist=new ANNdist[1];
    for(i=0,k=0;i<h;++i)
    {
        for(j=0;j<w;++j,++k)
        {
            pt[0]=j;
            pt[1]=i;
            tree->annkSearch(pt, 1, idx, dist);
            if(sqrt(dist[0])<=unkownWidth)
                info[k]=2;
            else
            if(sqrt(dist[0])<=bgWidth && label[k]==0)
                info[k]=0;
            else
            if(label[k]==1)
                info[k]=1;
            else
                info[k]=3;
        }
    }

    delete tree;
    delete [] idx;
    delete [] dist;
    annClose();
    annDeallocPts(pts);
    annDeallocPt(pt);
}

void GrabCut_ywz::InitGMM(const int *info)
{
    int i,j,k;
    int w,h,index;
    int bgPixelsNum(0);
    int fgPixelsNum(0);
    RGBQUAD rgb;

    w=image.GetWidth();
    h=image.GetHeight();
    bgCluster=new Cluster_ywz(5);	
    fgCluster=new Cluster_ywz(5);
    double *bgPixels=new double[w*h*3];
    double *fgPixels=new double[w*h*3];

    for(i=0,k=0;i<h;++i)
    {
        for(j=0;j<w;++j,++k)
        {
            if(info[k]==0)
            {
                rgb=image.GetPixelColor(j, i);
                index=3*bgPixelsNum;
                bgPixels[index]=rgb.rgbRed;
                bgPixels[index+1]=rgb.rgbGreen;
                bgPixels[index+2]=rgb.rgbBlue;
                ++bgPixelsNum;
            }
            else
            if(label[k]==1)
            {
                rgb=image.GetPixelColor(j, i);
                index=3*fgPixelsNum;
                fgPixels[index]=rgb.rgbRed;
                fgPixels[index+1]=rgb.rgbGreen;
                fgPixels[index+2]=rgb.rgbBlue;
                ++fgPixelsNum;
            }
        }
    }

    GMM_ywz::InitGMM(bgPixels, bgPixelsNum, bgCluster->clusterNum, bgCluster->mean, 
                 bgCluster->weight, bgCluster->det, bgCluster->cov);

    GMM_ywz::InitGMM(fgPixels, fgPixelsNum, fgCluster->clusterNum, fgCluster->mean, 
                 fgCluster->weight, fgCluster->det, fgCluster->cov);

    GMM_ywz::times=1;
    GMM_ywz::GetGMM(bgPixels, bgPixelsNum, bgCluster->clusterNum, bgCluster->mean, 
                bgCluster->weight, bgCluster->det, bgCluster->cov);

    GMM_ywz::GetGMM(fgPixels, fgPixelsNum, fgCluster->clusterNum, fgCluster->mean, 
                fgCluster->weight, fgCluster->det, fgCluster->cov);

    delete [] bgPixels;
    delete [] fgPixels;
}

void GrabCut_ywz::GetProb(float *prob)
{
    int     i,j,k;
    int     w,h;
    RGBQUAD rgb;
    double  data[3];
    float   F,B;

    w=image.GetWidth();
    h=image.GetHeight();
    for(i=0,k=0;i<h;++i)
    {
        for(j=0;j<w;++j,++k)
        {
            if(info[k]==2)
            {
                rgb=image.GetPixelColor(j, i);
                data[0]=rgb.rgbRed;
                data[1]=rgb.rgbGreen;
                data[2]=rgb.rgbBlue;
                F=fgCluster->Prob_ywz(data);
                B=bgCluster->Prob_ywz(data);
                prob[k]=F/(F+B+1e-20f);
            }
            else
            if(info[k]==3)
                prob[k]=0;
            else
                prob[k]=(float)info[k];
        }
    }
}

template<typename T>
static float Prob_ywz(int w, int h, int x, int y, int win, const T *label)
{
    int i,j;
    int half(win/2);
    T   s,c;

    for(i=y-half,s=0,c=0;i<=y+half;++i)
    {
        for(j=x-half;j<=x+half;++j)
        {
            if(j>=0 && j<w && i>=0 && i<h)
            {
                c++;
                s+=label[j+i*w];
            }
        }
    }
    return (float)s/c;
}

template<typename T>
void GetFracProb(int w, int h, float *p, const T *data, int win)
{
    for(int i=0;i<h;++i)
    {
        for(int j=0;j<w;++j)
        {
            p[j+i*w]=Prob_ywz(w, h, j, i, win, data);
        }
    }
}

void GrabCut_ywz::Cut(int *label, float *prior)
{
    int w(image.GetWidth());
    int h(image.GetHeight());
    int n(w*h);
    float *tmp=new float[n];
    MRF::CostVal *vcues=new MRF::CostVal[n];
    MRF::CostVal *hcues=new MRF::CostVal[n];
    MRF::CostVal *D=new MRF::CostVal[n*2];

    GetFracProb(w, h, tmp, label, 10);
    GetCues(image, hcues, vcues);
    for(int i=0;i<n;++i)
    {
        if(prior)
        {
            D[i*2]  =(1-prob[i])*(1-tmp[i])*(1-prior[i]);
            D[i*2+1]=prob[i]*tmp[i]*prior[i];
        }
        else
        {
            D[i*2]  =(1-prob[i])*(1-tmp[i]);
            D[i*2+1]=prob[i]*tmp[i];
        }
        D[i*2]  =D[i*2]/(D[i*2]+D[i*2+1]+1e-10f);
        D[i*2+1]=1-D[i*2];
        D[i*2]  =-log(min(1,D[i*2]+1e-10f));
        D[i*2+1]=-log(min(1,D[i*2+1]+1e-10f));
    }

    NormalizeDataCost(D, w, h, 2);
    SolveMRF(w, h, D, 2.0f, hcues, vcues, 2, 5, label);

    delete [] vcues;
    delete [] hcues;
    delete [] D;
    delete [] tmp;
}

void GrabCut_ywz::Cut_graphcut(int *label, CxImage& trimap, int pointNum, float * prior/* =NULL */)
{
	int w(image.GetWidth());
	int h(image.GetHeight());
	int n(w*h);
	float *tmp=new float[n];
	MRF::CostVal *vcues=new MRF::CostVal[n];
	MRF::CostVal *hcues=new MRF::CostVal[n];
	MRF::CostVal *D=new MRF::CostVal[n*2];

	GetFracProb(w, h, tmp, label, 20);
	GetCues(image, hcues, vcues);
	for(int i=0;i<n;++i)
	{
		if(prior)
		{
			D[i*2]  =(1-prob[i])*(1-tmp[i])*(1-prior[i]);
			D[i*2+1]=prob[i]*tmp[i]*prior[i];
		}
		else
		{
			D[i*2]  =(1-prob[i])*(1-tmp[i]);
			D[i*2+1]=prob[i]*tmp[i];
		}
		D[i*2]  =D[i*2]/(D[i*2]+D[i*2+1]+1e-10f);
		D[i*2+1]=1-D[i*2];
		D[i*2]  =-log(min(1,D[i*2]+1e-10f));
		D[i*2+1]=-log(min(1,D[i*2+1]+1e-10f));
	}

	static int count=0;
	NormalizeDataCost(D, w, h, 2);
	/*CxImage tempIM;
	tempIM.Create(image.GetWidth(), image.GetHeight(), 24);
	for (int i=0; i<image.GetWidth(); ++i)
	{
		for (int j=0; j<image.GetHeight(); ++j)
		{
			tempIM.SetPixelColor(i,j, RGB(D[(j*image.GetWidth()+i)*2+1]*255,D[(j*image.GetWidth()+i)*2+1]*255,D[(j*image.GetWidth()+i)*2+1]*255));
		}
	}
	CString path = "H:/test/d_";
	path.AppendFormat("%d.png", count);
	tempIM.Save(path.GetBuffer(), CXIMAGE_FORMAT_PNG);
	printf(path.GetBuffer());
	count++;*/
	SolveMRF_graphcut(w, h, D, hcues, vcues, label, trimap, pointNum);

	delete [] vcues;
	delete [] hcues;
	delete [] D;
	delete [] tmp;
}

void GrabCut_ywz::SolveMRF_graphcut(int w, int h, MRF::CostVal *D,MRF::CostVal *hCues, MRF::CostVal *vCues,int *res, CxImage& trimap, int pointNum)
{
	int i,k;
	int size;
	static const int direct[4][2]={{1,0},{0,1},{1,1},{1,-1}};
	size=pointNum;
	void **     graphPoint;
	graphPoint = new void *[size];
	memset(graphPoint, 0, sizeof(void *)*size);
	Graph* graph=new Graph;
	int index=0;
	std::map<std::pair<int, int>, int> graphIndex;

	for (int j=0; j<h; ++j)
	{
		for (int i=0; i<w; ++i)
		{
			BYTE gray  = trimap.GetPixelGray(i,j);
			if (gray==128)
			{
				graphIndex.insert(make_pair(make_pair(i,j), index));
				graphPoint[index] = graph->add_node();
				int index2 = (j*w+i)*2;
				graph->set_tweights(graphPoint[index],12*D[index2], 12*D[index2+1]);
				index++;
			}
		}
	}

	int x1, y1;
	for (int j=0; j<h; ++j)
		for (int i=0; i<w; ++i)
			for (int k=0; k<4; ++k)
			{
				x1=i+direct[k][0];
				y1=j+direct[k][1];
				if (x1>=0&&x1<w&&y1>=0&&y1<h)
					if (trimap.GetPixelGray(x1, y1)==128)
					{
						float smooth = hCues[j*w+i]+vCues[j*w+i];
						graph->add_edge(graphPoint[graphIndex[make_pair(i,j)]], graphPoint[graphIndex[make_pair(x1, y1)]],smooth*5, 5*smooth);
					}
			}
			vector<POINT> vec_bound;
			GetTrimapBound(trimap, vec_bound);
			float K = 1000000;
			for (int i=0; i<vec_bound.size(); ++i)
			{
				int x = vec_bound[i].x;
				int y = vec_bound[i].y;
				int index = y*w+x;
					if (res[index])
						graph->set_tweights(graphPoint[graphIndex[make_pair(x, y)]], K, 0);
					else 
						graph->set_tweights(graphPoint[graphIndex[make_pair(x, y)]], 0, K);
				
			}
			//if(alp==0)
			//	graph->set_tweights(graphPoint[graphIndex[make_pair(pixel.x, pixel.y)]], 0, (float)K);
			//else
			//	if(alp==255)
			//		graph->set_tweights(graphPoint[graphIndex[make_pair(pixel.x, pixel.y)]], (float)K, 0);
			graph->maxflow();
			for (int j=0; j<h; ++j)
			{
				for (int i=0; i<w; ++i)
				{
					if (trimap.GetPixelGray(i,j)==128)
					{
						if (graph->what_segment(graphPoint[graphIndex[make_pair(i,j)]])==Graph::SINK)
							res[j*w+i] = 0;
						else
							res[j*w+i] = 1;
					}
				}
			}

			delete graph;
			delete[] graphPoint;

}

void GrabCut_ywz::GetTrimapBound(CxImage& trimap, vector<POINT>& vec_bound)
{

	POINT p;
	vec_bound.clear();

	int w=trimap.GetWidth();
	int h=trimap.GetHeight();

	RGBQUAD r1,r2;
	for (int j=0;j<h;++j)
	{
		for (int i=0;i<w;++i)
		{
			r1=trimap.GetPixelColor(i,j);
			if (r1.rgbBlue==128)
			{
				if (i-1>=0)
				{
					r2=trimap.GetPixelColor(i-1,j);
					if (r2.rgbBlue!=128)
					{
						p.x=i;
						p.y=j;
						vec_bound.push_back(p);
						continue;
					}
				}

				if (i+1<w)
				{
					r2=trimap.GetPixelColor(i+1,j);
					if (r2.rgbBlue!=128)
					{
						p.x=i;
						p.y=j;
						vec_bound.push_back(p);
						continue;
					}
				}

				if (j-1>=0)
				{
					r2=trimap.GetPixelColor(i,j-1);
					if (r2.rgbBlue!=128)
					{
						p.x=i;
						p.y=j;
						vec_bound.push_back(p);
						continue;
					}
				}
				if (j+1<h)
				{
					r2=trimap.GetPixelColor(i,j+1);
					if (r2.rgbBlue!=128)
					{
						p.x=i;
						p.y=j;
						vec_bound.push_back(p);
						continue;
					}
				}

			}
		}
	}

}