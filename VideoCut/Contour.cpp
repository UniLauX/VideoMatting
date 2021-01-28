#include "../VideoEditing/stdafx.h"
#include "Contour.h"
#include "../ClosedFormMatting/Matting.h"
#include <iostream>
#include <fstream>
#include <omp.h>
#include "../grab/BiImageProcess.h"

using namespace std;

Contour::Contour()
{
    width=0;
    height=0;
    pixelNum=0;
    localWinSize=80;

    label=NULL;
    image=NULL;
    winCenters.clear();

    boundMask=NULL;
    graph=NULL;
    graphPoint=NULL;
}

Contour::~Contour()
{
    Clear();
}

void Contour::ClearLocalWins()
{
    vector<LocalClassifier *>::iterator iter;

    for(iter=localWins.begin();iter!=localWins.end();++iter)
    {
        delete [] *iter;
    }
    localWins.clear();
	localWins.swap(vector<LocalClassifier*>());
}

void Contour::Clear()
{
    ClearLocalWins();
	if (label)
	{
		delete [] label;
		label=NULL;
	}
	if (graph)
	{
		delete graph;
		graph=NULL;
	}
	if (boundMask)
	{
		delete [] boundMask;
		boundMask=NULL;
	}
    
    //delete [] graphPoint;
    //graphPoint=NULL;
    
}

void Contour::SetFrameInfo(CxImage *frame, unsigned char *l)
{
    image=frame;
    width=image->GetWidth();
    height=image->GetHeight();
    pixelNum=width*height;

    Clear();
    try
    {
	    label=new unsigned char[pixelNum];
		boundMask=new char[pixelNum];
		graph=new Graph;
    }
    catch (...)
    {
	    std::cout<<pixelNum<<std::endl;
	    std::cout<<"out of memory"<<std::endl;
    }
    memcpy(label, l, sizeof(unsigned char)*pixelNum);
   //graphPoint=new void *[pixelNum];
}

void Contour::SetNewLabel(unsigned char* l, int size)
{
	if (label == NULL)
	{
		label = new unsigned char[width*height];
	}
    memcpy(label, l, sizeof(unsigned char)*size);
}
unsigned char* Contour::GetLabel()
{
	return this->label;
}

void Contour::SetLocalWinSize(int newWinSize)
{
    localWinSize=newWinSize;
}

int Contour::GetLocalWinSize()
{
    return localWinSize;
}

vector<LocalClassifier *> *Contour::GetLocalWins()
{
    return &localWins;
}

const VVPOINTS *Contour::GetLocalWinCenters()
{
    return &winCenters;
}

void Contour::GetLocalClassifier(const TRANS_CLASSIFIER_PARAM &param, int nfirst)
{
    if(logText)
        logText<<"in GetLocalClassifier"<<endl;
    int i,j;
    int m,n;
    VVPOINTS centers;
    LocalClassifier *classifier;
    vector<LocalClassifier *>::iterator iter;
/*
	CxImage temp;
	temp.Copy(*(this->image));
	temp.Clear();
	*/
    if(nfirst==0)
    {
        winCenters.clear();
	
		
        GetAllWinowCenters(winCenters);

        m=(int)winCenters.size();
	//	cout<<"m= "<<m<<endl;
        ClearLocalWins();

		RGBQUAD rgb2;
		rgb2.rgbRed=255;
		rgb2.rgbGreen=0;
		rgb2.rgbBlue=0;

        for(i=0;i<m;++i)
        {
            n=(int)winCenters[i].size();
            classifier=new LocalClassifier[n];
            // Should check if there is "new" operation!!!
        //    #pragma omp parallel for
            for(j=0;j<n;++j)
            {
	/*
				temp.SetPixelColor(winCenters[i][j].x, winCenters[i][j].y, RGB(255,0,0));
      
				int x11,x12;
				int y11,y12;
				
				x11=winCenters[i][j].x-localWinSize/2;
				x12=winCenters[i][j].x+localWinSize/2;
				y11=winCenters[i][j].y-localWinSize/2;
				y12=winCenters[i][j].y+localWinSize/2;

				(*image).DrawLine(x11,x11,y11,y12,rgb2);
				(*image).DrawLine(x11,x12,y11,y11,rgb2);
				(*image).DrawLine(x12,x12,y11,y12,rgb2);
				(*image).DrawLine(x12,x11,y12,y12,rgb2);
			 */

                classifier[j].InitParam(winCenters[i][j].x, winCenters[i][j].y, 
                                        localWinSize, label, image);


                classifier[j].InitModel(param.mode);
            }
            localWins.push_back(classifier);
        }
    }
    else
    {
        centers.clear();
        GetAllWinowCenters(centers);
        for(i=0,iter=localWins.begin();iter!=localWins.end();++iter,++i)
        {
            n=(int)winCenters[i].size();
            for(j=0;j<n;++j)
            {
				//temp.SetPixelColor(winCenters[i][j].x, winCenters[i][j].y, RGB(255,0,0));
                (*iter)[j].Move(label, image, centers);
                (*iter)[j].GetModelParam();
            }
        }
    }

	//temp.Save("H:/test/win_first.jpg", CXIMAGE_FORMAT_JPG);
}

void Contour::TransClassifier(CxImage *nextFrame,
                              VVPOINTS &newCenters, 
                              TRANS_CLASSIFIER_PARAM &param)
{
     int i,j,n;
     POINT point;
     vector<LocalClassifier *>::iterator iter;
     image=nextFrame;  //To next frame, assign new image
     for(i=0,iter=localWins.begin();iter!=localWins.end();++iter,++i)     //localWins的个数为1
     {
        
          n=(int)winCenters[i].size();

		//  cout<<"winCenters的个数="<<n<<endl;
          for(j=0;j<n;++j)
          {
             // if(winCenters[i][j].x!=newCenters[i][j].x)                newCenters到此仍未改变
			//	  cout<<"have changed"<<endl;
              (*iter)[j].Move(label, nextFrame, newCenters);        
              (*iter)[j].GetModelParam();
          }
     }
	 
}

void Contour::GetProb(CxImage& boundPtsIndict)
{
     int i,j;
     int winNum;
     int index;
     POINT point;
     double *F,*B,*P;
	 F = new double[pixelNum];
	 B = new double[pixelNum];
	 P = new double[pixelNum];
	 memset(F, 0, sizeof(double)*pixelNum);
	 memset(B, 0, sizeof(double)*pixelNum);
	 memset(P, 0, sizeof(double)*pixelNum);
     //std::map<std::pair<int, int>, double> F, B, P;
	 //boost::unordered_map<point2, double> F, B, P;
	 //stdext::hash_map<int, double>F, B, P;// for large pic
     vector<LocalClassifier *>::iterator iter;
     boundPts.clear();
     boundPtsIndict.Clear();
     Pf.clear();
     Pb.clear();
     memset(boundMask, 0, sizeof(char)*pixelNum);

     for(i=0,iter=localWins.begin();iter!=localWins.end();++iter,++i)
     {
          winNum=(int)winCenters[i].size();
          for(j=0;j<winNum;++j)
          {
               (*iter)[j].AddProbValues(F, B, P);
          }
     }

     int left,right,top,bottom;
     GetLocalWinBound(left, right, top, bottom);
     for(i=bottom;i<top;++i)
     {
          for(j=left;j<right;++j)
		  {
			  index=i*width+j;
			  if((F[index]!=0||B[index]!=0)&&boundPtsIndict.GetPixelGray(j,i)==0)
			  {
				  point.x=j;
				  point.y=i;
				  boundPtsIndict.SetPixelIndex(j,i,128);
				  boundPts.push_back(point);
				  Pf.push_back((F[index]/P[index]));
				  Pb.push_back(B[index]/P[index]);
				  boundMask[index]=1;
			  }
          }
     }
	 delete[] F; delete[] B; delete[] P;
}

void Contour::GetLocalWinBound(int &left, int &right, int &top, int &bottom)
{
    int i,j;
    int x1,y1,x2,y2;
    int winNum;
    vector<LocalClassifier *>::iterator iter;

    left=width; right=0;
    top=0; bottom=height;
    for(i=0,iter=localWins.begin();iter!=localWins.end();++iter,++i)
    {
        winNum=(int)winCenters[i].size();
        for(j=0;j<winNum;++j)
        {
            (*iter)[j].GetLeftCorner(x1, y1);
            (*iter)[j].GetRightCorner(x2, y2);
            if(x1<left)
                left=x1;
            if(x2>right)
                right=x2;
            if(y2>top)
                top=y2;
            if(y1<bottom)
                bottom=y1;
        }
    }
}

double Contour::GetBeta()
{
     int i,size;
     int k,count;
     int x,y;
     int index1,index2;
     double Z;
     double tmp1,tmp2,tmp3;
     POINT point;
     RGBQUAD rgb1,rgb2;
     int direct[4][2]={{1,0},{0,1},{1,1},{1,-1}};

     size=(int)boundPts.size();
     for(i=0,Z=0,count=0;i<size;++i)
     {
          point=boundPts[i];
          rgb1=image->GetPixelColor(point.x, point.y);
          index1=point.y*width+point.x;
          for(k=0;k<4;++k)
          {
               y=point.y+direct[k][1];
               x=point.x+direct[k][0];
               index2=y*width+x;
               if(x>=0 && x<width && y>=0 && y<height)
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
     Z/=count;	
     this->beta=0.5/Z;
     return this->beta;
}

double Contour::GetBeta_WholeImg()
{
	int i,size;
	int k,count = 0;
	int x,y;
	int index1,index2;
	double Z = 0;
	double tmp1,tmp2,tmp3;
	POINT point;
	RGBQUAD rgb1,rgb2;
	int direct[4][2]={{1,0},{0,1},{1,1},{1,-1}};

	for(i=0;i<this->image->GetWidth();++i)
	{
		for (int j=0; j<this->image->GetHeight(); ++j)
		{
			point.x = i;
			point.y = j;

			rgb1=image->GetPixelColor(point.x, point.y);
			index1=point.y*width+point.x;
			for(k=0;k<4;++k)
			{
				y=point.y+direct[k][1];
				x=point.x+direct[k][0];
				index2=y*width+x;
				if(x>=0 && x<width && y>=0 && y<height)
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
	this->beta=0.5/Z;
	return this->beta;
}


double Contour::GetNeighborWeight(double Beta, int x1, int y1, int x2, int y2)
{
     double dist,Z;
     double tmp1,tmp2,tmp3;
     RGBQUAD rgb1,rgb2;
     const double C(50);	

     if(x2>=0 && x2<width && y2>=0 && y2<height)
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

// Need to be optimized
void Contour::InitGraph(CxImage& boundPtsIndict, std::map<std::pair<int, int>, int>& nodeIndex)
{
     int i,k;
     int size;
     int x,y;
     int idx1,idx2;
     POINT point;
     double prob;
     int direct[4][2]={{1,0},{0,1},{1,1},{1,-1}};

     size=(int)boundPts.size();
     delete graph;
     graph=new Graph;
     graphPoint=new void *[size];
     memset(graphPoint, 0, sizeof(void *)*size);
      for (int i=0; i<size; ++i)
     {
	     point = boundPts[i];
	     nodeIndex.insert(make_pair(make_pair(point.x, point.y), i));
	     graphPoint[i] = graph->add_node();
	     graph->set_tweights(graphPoint[i], (float)-log(Pb[i]), (float)-log(Pf[i]));
     }

	  Pb.swap(std::vector<double>());
	  Pf.swap(std::vector<double>());//release pf and pb memory
     for (int i=0; i<size; ++i)
     {
	     point = boundPts[i];
	     for (int k=0; k<4; ++k)
	     {
		     int x = point.x +direct[k][0];
		     int y = point.y + direct[k][1];
		     if (x>=0&&x<width&&y>=0&&y<height&&boundPtsIndict.GetPixelGray(x,y)==128)
		     {
			      idx2=y*width+x;
			     if(boundMask[idx2]==0)
				     break;
			     prob=GetNeighborWeight(beta, point.x, point.y, x, y);
			     if(prob!=-1)
			     {
				     graph->add_edge(graphPoint[nodeIndex[make_pair(point.x, point.y)]], graphPoint[nodeIndex[make_pair(x,y)]], (float)prob, (float)prob);
			     }
		     }
	     }
     }
}

void Contour::InitGraph(CxImage& boundPtsIndict, int* nodeIndex)
{
	int i,k;
	int size;
	int x,y;
	int idx1,idx2;
	POINT point;
	double prob;
	int direct[4][2]={{1,0},{0,1},{1,1},{1,-1}};

	size=(int)boundPts.size();
	delete graph;
	graph=new Graph;
	graphPoint=new void *[size];
	memset(graphPoint, 0, sizeof(void *)*size);
	for (int i=0; i<size; ++i)
	{
		point = boundPts[i];
		int index = point.y*width+point.x;
		nodeIndex[index] = i;
		graphPoint[i] = graph->add_node();
		graph->set_tweights(graphPoint[i], (float)-log(Pb[i]), (float)-log(Pf[i]));
	}
	for (int i=0; i<size; ++i)
	{
		point = boundPts[i];
		for (int k=0; k<4; ++k)
		{
			int x = point.x +direct[k][0];
			int y = point.y + direct[k][1];
			if (x>=0&&x<width&&y>=0&&y<height&&boundPtsIndict.GetPixelGray(x,y)==128)
			{
				idx2=y*width+x;
				if(boundMask[idx2]==0)
					break;
				prob=GetNeighborWeight(beta, point.x, point.y, x, y);
				if(prob!=-1)
				{
					graph->add_edge(graphPoint[nodeIndex[point.y*width+point.x]], graphPoint[nodeIndex[y*width+x]], (float)prob, (float)prob);
				}
			}
		}
	}

}
void Contour::DoCut(unsigned char *label, CxImage& boundPtsIndict)
{
	int i,size;
	int index;
	POINT point;
	//std::map<std::pair<int, int>, int> nodeIndex;// for large image;
	int* nodeIndex = new int[pixelNum];
	memset(nodeIndex, 0, sizeof(int)*pixelNum);
	InitGraph(boundPtsIndict, nodeIndex);
	graph->maxflow();
	size=(int)boundPts.size();
	for(i=0;i<size;++i)
	{
		point=boundPts[i];
		index=point.y*width+point.x;

		if(graph->what_segment(graphPoint[nodeIndex[index]])==Graph::SINK)
		{
			// BG
			label[index]=0;
			this->label[index]=0;
		}
		else
		{
			// FG
			label[index]=1;
			this->label[index]=1;
		}
	}
	delete[] graphPoint;
	graphPoint = NULL;
	delete[] nodeIndex;
}

void Contour::GetCutResultUC(unsigned char *label)
{
	CxImage ptsIndict;
	ptsIndict.Create(width, height, 8);
	ptsIndict.SetGrayPalette();
	ptsIndict.Clear();
	if (boundMask==NULL)
		boundMask = new char[pixelNum];
	GetProb(ptsIndict);
	if(boundPts.size())
		DoCut(label, ptsIndict);
	delete[] boundMask;
	boundMask = NULL;
}
void Contour::GetCutResult(CxImage &res)
{
     int i,j;
     int w,h;
     int index;
     RGBQUAD rgb,r;

     w=image->GetWidth();
     h=image->GetHeight();
     res.Copy(*image);
     for (i=0;i<h;++i)
     {
          for (j=0;j<w;++j)
          {
               index=i*w+j;
               r=image->GetPixelColor(j, i);

	       if (label[index])
	       {
		       rgb.rgbBlue=label[index]*r.rgbBlue;
		       rgb.rgbGreen=label[index]*r.rgbGreen;
		       rgb.rgbRed=label[index]*r.rgbRed;
		       res.SetPixelColor(j, i, rgb);
	       }
	       else
		       res.SetPixelColor(j,i,this->rgb);
			   //res.SetPixelColor(j,i, RGB(0,0,0));
             
			 //  res.SetPixelColor(j,i,r);
          }
     }
}

void Contour::GetAlphaResult(CxImage &res)
{
     int i,j;
     int w,h;
     int index;
     RGBQUAD rgb;

     w=image->GetWidth();
     h=image->GetHeight();
     res.Copy(*image);
     for (i=0;i<h;++i)
     {
          for (j=0;j<w;++j)
          {
               index=i*w+j;
               rgb.rgbBlue=(BYTE)(label[index]*255);
               rgb.rgbGreen=rgb.rgbBlue;
               rgb.rgbRed=rgb.rgbBlue;
               res.SetPixelColor(j, i, rgb);
          }
     }
}

void Contour::RunMatting(Matting &matting, CxImage &result)
{
     int i,j;
     int m,n;
     int index;
     int *count;
     double *alpha;
     RGBQUAD rgb;
     LocalClassifier *classifier;
     list<LocalClassifier *>::iterator iter;

	 std::cout<<"in runmatting before new "<<std::endl;
     alpha=new double[pixelNum];
     count=new int[pixelNum];
     memset(alpha, 0, sizeof(double)*pixelNum);
     memset(count, 0, sizeof(int)*pixelNum);
     result.Copy(*image);

     winCenters.clear();
     GetAllWinowCenters(winCenters);
     m=(int)winCenters.size();
     ClearLocalWins();
	 std::cout<<"in run matting before for"<<std::endl;
     for(i=0;i<m;++i)
     {
          n=(int)winCenters[i].size();
          classifier=new LocalClassifier[n];
          for(j=0;j<n;++j)
          {
               classifier[j].InitParam(winCenters[i][j].x, winCenters[i][j].y, 
                                       80, label, image);
               classifier[j].DoMatting(matting, alpha, count);
          }
          localWins.push_back(classifier);
     }

     for(i=0;i<height;++i)
     {
          for(j=0;j<width;++j)
          {
               index=i*width+j;
               if(count[index]==0)
                    rgb.rgbRed=(BYTE)(label[index]*255);
               else
                    rgb.rgbRed=(BYTE)(alpha[index]/count[index]*255);
               rgb.rgbGreen=rgb.rgbRed;
               rgb.rgbBlue=rgb.rgbRed;
               result.SetPixelColor(j, i, rgb);
          }
     }
     result.Save("D:/Result/alpha.jpg", CXIMAGE_FORMAT_JPG);

     delete [] alpha;
     delete [] count;
}

void Contour::RunMatting(MATTING_PARAM &param, CxImage &result)
{
     Matting matting;

     matting.SetWinSize(param.winSize);
     matting.SetActiveLevelNum(param.activeLevel);
     matting.SetEpsilon(param.epsilon);
     matting.SetErodeWinSize(param.erodeWinSize);
     matting.SetLevelNum(param.level);
     matting.SetThrAlpha(param.thrAlpha);

     RunMatting(matting, result);
}

void Contour::SetColor(BYTE r,BYTE g,BYTE b)
{
	this->rgb.rgbRed=r;
	this->rgb.rgbGreen=g;
	this->rgb.rgbBlue=b;
}
//////////////////////////////////////////////////////////////////////////
// Test
//////////////////////////////////////////////////////////////////////////

#include <fstream>

using namespace std;

void Contour::Test()
{
     int i,size;
     POINT point;
     RGBQUAD rgb;
     CxImage pic(*image);
     TRANS_CLASSIFIER_PARAM pam;

     double start,stop;
     double duration;

     pam.mode=3;
     start=clock();

     GetLocalClassifier(pam);
     //cout << "Local num: " << localWins.size() << endl;
	 if(logText)
		 logText<<"Local num: "<<localWins.size()<<endl;

     stop=clock();
     duration=((double)(stop-start))/CLK_TCK; 
     //printf("Create local classifier time: %f\n", duration);
	 if(logText)
		 logText<<"Create local classifier time: "<<duration<<endl;
     
     GetCutResultUC(label);

     size=(int)boundPts.size();
     for(i=0;i<size;++i)
     {
          point=boundPts[i];
          rgb.rgbRed=(BYTE)(255*Pf[i]);
          rgb.rgbGreen=rgb.rgbRed;
          rgb.rgbBlue=rgb.rgbRed;
          pic.SetPixelColor(point.x, point.y, rgb);
     }

     pic.Save("D:\\Result\\All_Prob.jpg", CXIMAGE_FORMAT_JPG);
}

void Contour::TestLocalWinPosition(CxImage *image,const VVPOINTS &newcenters)
{
     int i,j,n;
     CxImage pic;
     vector<LocalClassifier *>::iterator iter;

	 int winsize;
     pic.Copy(*image);
     for(i=0,iter=localWins.begin();iter!=localWins.end();++iter,++i)
     {
          n=(int)winCenters[i].size();
          for(j=0;j<n;++j)
          {
               ((*iter)[j]).TestPos(&pic);
			   winsize=((*iter)[j]).GetWidth();
          }
     }

     CString dir;
     static int naindex(0);
	 dir.Format("%s%d%s", "H:\\Test\\localwins", naindex++, ".jpg");
     pic.Save(dir, CXIMAGE_FORMAT_JPG);
	 
	 CxImage pic2;
	 pic2.Copy(*image);

	 RGBQUAD rgb2;
	 rgb2.rgbRed=255;
	 rgb2.rgbGreen=0;
	 rgb2.rgbBlue=0;
	 for (int i=0;i<newcenters.size();++i)
	 {
		 for (int j=0;j<newcenters[i].size();++j)
		 {
			 int x11,x12;
			 int y11,y12;
			 x11=newcenters[i][j].x-winsize/2;
			 x12=newcenters[i][j].x+winsize/2;
			 y11=newcenters[i][j].y-winsize/2;
			 y12=newcenters[i][j].y+winsize/2;

			 pic2.DrawLine(x11,x11,y11,y12,rgb2);
			 pic2.DrawLine(x11,x12,y11,y11,rgb2);
			 pic2.DrawLine(x12,x12,y11,y12,rgb2);
			 pic2.DrawLine(x12,x11,y12,y12,rgb2);
		 }
	 }

	 CString dir2;
	 static int naindex2(0);
	 dir2.Format("%s%d%s", "D:\\Test\\Clocalwins", naindex2++, ".jpg");
	 pic2.Save(dir2,CXIMAGE_FORMAT_JPG);
}

void Contour::TestParameters()
{
     int i,j,n;
     int size;
     POINT point;
     CxImage pic;
     RGBQUAD rgb;
     vector<LocalClassifier *>::iterator iter;
     CString dir;
     static int naindex(0);

     // Fs
     pic.Copy(*image);
     for(i=0,iter=localWins.begin();iter!=localWins.end();++iter,++i)
     {
          n=(int)winCenters[i].size();
          for(j=0;j<n;++j)
          {
               ((*iter)[j]).TestFs(&pic);
          }
     }
     dir.Format("%s%d%s", "D:\\Result\\Test\\Fs", naindex, ".jpg");
     pic.Save(dir, CXIMAGE_FORMAT_JPG);
/*
     // Dis
     pic.Copy(*image);
     for(i=0,iter=localWins.begin();iter!=localWins.end();++iter,++i)
     {
          n=(int)winCenters[i].size();
          for(j=0;j<n;++j)
          {
               ((*iter)[j]).TestDis(&pic);
          }
     }
     dir.Format("%s%d%s", "D:\\Result\\Test\\Dis", naindex, ".jpg");
     pic.Save(dir, CXIMAGE_FORMAT_JPG);
*/
     // Prob
//     dir.Format("%s%d%s", "D:\\Result\\\\Test\\Prob", naindex, ".txt");
//     ofstream out(dir.GetBuffer());

     pic.Copy(*image);
     size=(int)boundPts.size();
     for(i=0;i<size;++i)
     {
          point=boundPts[i];
          rgb.rgbRed=Pf[i]*255;
          rgb.rgbGreen=rgb.rgbRed;
          rgb.rgbBlue=rgb.rgbRed;
          pic.SetPixelColor(point.x, point.y, rgb);
     }
     dir.Format("%s%d%s", "D:\\Result\\Test\\Prob_FG", naindex, ".jpg");
     pic.Save(dir, CXIMAGE_FORMAT_JPG);

     pic.Copy(*image);
     size=(int)boundPts.size();
     for(i=0;i<size;++i)
     {
          point=boundPts[i];
          rgb.rgbRed=Pb[i]*255;
          rgb.rgbGreen=rgb.rgbRed;
          rgb.rgbBlue=rgb.rgbRed;
          pic.SetPixelColor(point.x, point.y, rgb);
     }
     dir.Format("%s%d%s", "D:\\Result\\Test\\Prob_BG", naindex++, ".jpg");
     pic.Save(dir, CXIMAGE_FORMAT_JPG);

//     TestLocalWinPosition(image);
}

void Contour::TestResult(int *label)
{
     int i,j,index;
     RGBQUAD rgb,r;
     CxImage pic;
     CString dir;
     static int naindex(0);

     pic.Copy(*image);
     for (i=0;i<image->GetHeight();++i)
     {
          for (j=0;j<image->GetWidth();++j)
          {
               r=image->GetPixelColor(j, i);
               index=i*image->GetWidth()+j;
               rgb.rgbBlue=label[index]*r.rgbBlue;
               rgb.rgbGreen=label[index]*r.rgbGreen;
               rgb.rgbRed=label[index]*r.rgbRed;
               pic.SetPixelColor(j, i, rgb);
          }
     }
     dir.Format("%s%d%s", "D:\\Result\\Cut", naindex++, ".jpg");
     pic.Save(dir, CXIMAGE_FORMAT_JPG);

     TestParameters();
}

RGBQUAD Contour::GetRGB()
{
	return rgb;
}
