#include "../GrabCut/Cutting.h"
#include "../GrabCut/gmm.h"
#include "../GrabCut/Global.h"
#include <iostream>
#include "../grab/BiImageProcess.h"

using namespace std;

void Cutting::GetLocalCuttingRegion(vector<POINT> &pixels, CLUSTER *fgCluster, CLUSTER *bgCluster)
{

     int i,index;
     int size;
     double data[3];
     POINT pixel;

     fgPixelsNum=0;
     bgPixelsNum=0;
     size=(int)pixels.size();
     for(i=0;i<size;++i)
     {
          pixel=pixels[i];
          index=pixel.y*width+pixel.x;
          GetData(*image, pixel.x, pixel.y, data);
          if(label[index]==1)
          {
		  fgPixels.push_back(data[0]);
		  fgPixels.push_back(data[1]);
		  fgPixels.push_back(data[2]);
          }
          else
          {
		  bgPixels.push_back(data[0]);
		  bgPixels.push_back(data[1]);
		  bgPixels.push_back(data[2]);
          }
     }
	 fgPixelsNum = fgPixels.size()/3;
	 bgPixelsNum = bgPixels.size()/3;
	 std::cout<<"BG Pixels "<<bgPixelsNum<<"FG Pixels: "<<fgPixelsNum<<std::endl;
	 if (bgPixelsNum==0||fgPixelsNum==0)
	 {
		 return;
	 }
     GMM::times=2;
     GMM::InitGMM(&bgPixels[0], bgPixelsNum, bgCluster->clusterNum, bgCluster->mean, 
                  bgCluster->weight, bgCluster->det, bgCluster->cov);

     GMM::InitGMM(&fgPixels[0], fgPixelsNum, fgCluster->clusterNum, fgCluster->mean, 
                  fgCluster->weight, fgCluster->det, fgCluster->cov);

     GMM::GetGMM(&bgPixels[0], bgPixelsNum, bgCluster->clusterNum, bgCluster->mean, 
                 bgCluster->weight, bgCluster->det, bgCluster->cov);

     GMM::GetGMM(&fgPixels[0], fgPixelsNum, fgCluster->clusterNum, fgCluster->mean, 
                 fgCluster->weight, fgCluster->det, fgCluster->cov);
}

void Cutting::InitLocalGraph(vector<POINT> &pixels, CxImage &alpha,
                             CLUSTER *fgCluster, CLUSTER *bgCluster, CxImage& localTrimap)
{
     int i,k;
     int x1,y1;
     int size;
     int index1,index2;
     double prob;
     double back,fore;
     POINT pixel;
     BYTE alp;
     double K=100000000;
     static const int direct[4][2]={{1,0},{0,1},{1,1},{1,-1}};

     size=(int)pixels.size();
     graphPoint = new void *[size];
     memset(graphPoint, 0, sizeof(void *)*size);
     delete graph;
     graph=new Graph;
     int index=0;
     for(i=0;i<size;++i)
     {
	    pixel=pixels[i];
		//graphIndex.insert(make_pair(make_pair(pixel.x, pixel.y),index));
		graphIndex[pixel.y*width+pixel.x] = index;
		graphPoint[index]=graph->add_node();
		index++;
     }
     for (i=0; i<size; ++i)
     {
	     pixel = pixels[i];
	     for (int k=0; k<4; ++k)
	     {
		       x1=pixel.x+direct[k][0];
		       y1=pixel.y+direct[k][1];
			   if (x1>=0&&x1<width&&y1>=0&&y1<height)
				   if (localTrimap.GetPixelGray(x1, y1)==128)
				   {
					   prob=CalNLinkWeight(beta, pixel.x, pixel.y, x1, y1);
					   //graph->add_edge(graphPoint[graphIndex[make_pair(pixel.x, pixel.y)]], graphPoint[graphIndex[make_pair(x1,y1)]], (float)prob, (float)prob);
					   graph->add_edge(graphPoint[graphIndex[pixel.y*width+pixel.x]], graphPoint[graphIndex[y1*width+x1]], (float)prob, (float)prob);

				   }
	     }
     }
	//vector<POINT> vec_bound;
	// GetTrimapBound(pixels, vec_bound, localTrimap);
     for(i=0;i<size;++i)
     {
          pixel=pixels[i];
          alp=alpha.GetPixelGray(pixel.x, pixel.y);
          if(alp==0)
			  graph->set_tweights(graphPoint[graphIndex[pixel.y*width+pixel.x]], (float)0, (float)K);
		
          else
          if(alp==255)
			  graph->set_tweights(graphPoint[graphIndex[pixel.y*width+pixel.x]], (float)K, (float)0);
		                                                          
          else
          {
               back=GetTLinkWeight(pixel.x, pixel.y, bgCluster);
               fore=GetTLinkWeight(pixel.x, pixel.y, fgCluster);			
               graph->set_tweights(graphPoint[graphIndex[pixel.y*width+pixel.x]], (float)fore, (float)back);
          }
     }
	 //for (int i=0; i<vec_bound.size(); ++i)
	 //{
		// int x = vec_bound[i].x;
		// int y = vec_bound[i].y;
		// int index = y*width+x;
		// if (label[index])
		// {
		//	   graph->set_tweights(graphPoint[graphIndex[make_pair(x, y)]], (float)0, (float)K);
		//	    proim.SetPixelIndex(x, y, 0);
		// }
		// else
		// {
		//	   graph->set_tweights(graphPoint[graphIndex[make_pair(pixel.x, pixel.y)]], (float)K, (float)0);
		//	   proim.SetPixelIndex(x, y, 255);
		// }
	 //}
}

void Cutting::LocalUpdate(vector<POINT> &pixels, CxImage &alpha, 
                          CLUSTER *fgCluster, CLUSTER *bgCluster)
{
	int i,k;
	int index;
	int size;
	double back,fore;
	double data[3];
	POINT pixel;
	BYTE alp;

	bgPixelsNum=0;
	fgPixelsNum=0;
	size=(int)pixels.size();
	for(i=0;i<size;++i)
	{
		pixel=pixels[i];
		k=pixel.y*width+pixel.x;
		GetData(*image, pixel.x, pixel.y, data);
		alp=alpha.GetPixelGray(pixel.x, pixel.y);
		//if(graph->what_segment(graphPoint[graphIndex[make_pair(pixel.x, pixel.y)]])==Graph::SOURCE ||
		//	alp==255)
		if(graph->what_segment(graphPoint[graphIndex[pixel.y*width+pixel.x]])==Graph::SOURCE ||
			alp==255)
		{
			bgPixels.push_back(data[0]);
			bgPixels.push_back(data[1]);
			bgPixels.push_back(data[2]);
			label[k]=0;
		}
		//else if(graph->what_segment(graphPoint[graphIndex[make_pair(pixel.x, pixel.y)]])==Graph::SINK&&alp!=255)
		else if(graph->what_segment(graphPoint[graphIndex[pixel.y*width+pixel.x]])==Graph::SINK&&alp!=255)
		{
			fgPixels.push_back(data[0]);
			fgPixels.push_back(data[1]);
			fgPixels.push_back(data[2]);
			label[k]=1;
		}
	}

	bgPixelsNum = bgPixels.size()/3;
	fgPixelsNum = fgPixels.size()/3;
	if (bgPixelsNum==0||fgPixelsNum==0)
		return;
	GMM::times=1;
	GMM::GetGMM(&bgPixels[0], bgPixelsNum, bgCluster->clusterNum, bgCluster->mean, 
		bgCluster->weight, bgCluster->det, bgCluster->cov);

	GMM::GetGMM(&fgPixels[0], fgPixelsNum, fgCluster->clusterNum, fgCluster->mean, 
		fgCluster->weight, fgCluster->det, fgCluster->cov);
	//Update Graph
	for(i=0;i<size;++i)
	{
		pixel=pixels[i];
		index=pixel.y*width+pixel.x;
		back=GetTLinkWeight(pixel.x, pixel.y, bgCluster);
		fore=GetTLinkWeight(pixel.x, pixel.y, fgCluster);			
		//graph->set_tweights(graphPoint[graphIndex[make_pair(pixel.x, pixel.y)]], (float)fore, (float)back);
		graph->set_tweights(graphPoint[graphIndex[pixel.y*width+pixel.x]], (float)fore, (float)back);

	}
}

//void Cutting::LocalCalculate(vector<POINT> &pixels, CxImage &scrib, CxImage &alpha, CxImage& localTrimap)
void Cutting::LocalCalculate(vector<POINT> &pixels, UserScribble &scrib, CxImage &alpha, CxImage& localTrimap)
{
	int Times=4;
	int i;
	graphIndex = new int[localTrimap.GetWidth()*localTrimap.GetHeight()];
	memset(graphIndex, 0, sizeof(int)*localTrimap.GetWidth()*localTrimap.GetHeight());
	Cluster *FG,*BG;
	BG=new Cluster(3);	
	FG=new Cluster(3);   
	GetLocalCuttingRegion(pixels, FG, BG);
	InitLocalGraph(pixels, alpha, FG, BG, localTrimap);
	for(i=0;i<Times;++i)
	{
		graph->maxflow();
		LocalUpdate(pixels, alpha, FG, BG);
		fgPixels.clear();
		bgPixels.clear();
	}
	//BiImageProcess::checkConnectivity(label,width,height);
	//BiImageProcess::checkConnectivityfast(label, width, height);
	delete BG;
	delete FG;
	delete[] graphPoint;
	delete[] graphIndex;
	graphPoint=NULL;
}

void Cutting::UpdateLabel(CxImage &scrib)
{
     int i,j;
     BYTE rgb;

     for(i=0;i<height;++i)
     {
          for(j=0;j<width;++j)
          {
               rgb=scrib.GetPixelGray(j, i);
               if(rgb==0)
                    label[i*width+j]=0;
               else
               if(rgb==255)
                    label[i*width+j]=1;
          }
     }
}

void Cutting::UpdateLabel(UserScribble& scrib)
{
	for (int i=0; i<scrib.forgptsize(); ++i)
	{
		int x = scrib.ForgPt(i).x;
		int y = scrib.ForgPt(i).y;
		label[y*width+x] = 1;
	}
	for (int i=0; i<scrib.backgptsize(); ++i)
	{
		int x = scrib.BackgPt(i).x;
		int y = scrib.BackgPt(i).y;
		label[y*width+x] = 0;
	}
}

void Cutting::UpdateLabel(CxImage &alpha, CxImage &scrib)
{
     int i,j;
     RGBQUAD rgb;
     for(i=0;i<height;++i)
     {
          for(j=0;j<width;++j)
          {            
               rgb=alpha.GetPixelColor(j, i);
               if(rgb.rgbRed==0)
                    label[i*width+j]=0;
               else
               if(rgb.rgbRed==255)
                    label[i*width+j]=1;
          }
     }

     BYTE rgbscrib;
     for(i=0;i<height;++i)
     {
          for(j=0;j<width;++j)
          {
               rgbscrib=scrib.GetPixelGray(j, i);
               if(rgbscrib==0)
                    label[i*width+j]=0;
               else
               if(rgbscrib==255)
                    label[i*width+j]=1;
          }
     }
}
void Cutting::UpdateLabel(CxImage& alpha, UserScribble& scrib)
{
	int i,j;
	RGBQUAD rgb;
	for(i=0;i<height;++i)
	{
		for(j=0;j<width;++j)
		{            
			rgb=alpha.GetPixelColor(j, i);
			if(rgb.rgbRed==0)
				label[i*width+j]=0;
			else
				if(rgb.rgbRed==255)
					label[i*width+j]=1;
		}
	}
	UpdateLabel(scrib);

}

void Cutting::SetLabel(unsigned char *label)
{
	
	//CxImage imagetemp;
	//imagetemp.Copy(*(this->image));
	//imagetemp.Clear();
	int index;
	for (int i=0;i<height;++i )
	{
		for (int j=0;j<width;++j)
		{
			index=i*width+j;
			this->label[index]=label[index];
			//if (this->label[index])
			//{
			//	imagetemp.SetPixelColor(j,i, RGB(this->label[index]*255,this->label[index]*255,this->label[index]*255));
			//}

		}
	}
	//imagetemp.Save("E:/im.bmp",CXIMAGE_FORMAT_BMP);
}

void Cutting::GetTrimap(CxImage &trimap,int dis)
{
	trimap.Create(image->GetWidth(), image->GetHeight(),8);
	trimap.SetGrayPalette();
	trimap.Clear();
	CxImage trimap2;
	trimap2.Create(image->GetWidth(), image->GetHeight(),8);
	CxImage trimap3;
	trimap3.Create(image->GetWidth(), image->GetHeight(), 8);
	CxImage trimap4;
	trimap4.Create(image->GetWidth(), image->GetHeight(),8);
	int index;
	for (int j=0;j<height;++j)
	{
		for (int i=0;i<width;++i)
		{
			index=j*width+i;
			trimap.SetPixelIndex(i,j, this->label[index]*255);
		}
	}
	BiImageProcess::GetTrimap(this->label,trimap,dis,width,height);
}

void Cutting::GetTrimapBound(vector<POINT>& pixels, vector<POINT>& vec_bound, CxImage& _trimap)
{
	vec_bound.clear();
	const int direct[4][2]={{1,0},{0,1},{-1,0},{0,-1}};

	int width = this->image->GetWidth();
	int height = this->image->GetHeight();
	for (int i=0; i<pixels.size(); ++i)
	{
		for (int j=0; j<4; ++j)
		{
			int x = pixels[i].x;
			int y = pixels[i].y;
			x += direct[j][0];
			y +=direct[j][1];
			if (x>=0&&x<width&&y>=0&&y<height)
			{
				if (_trimap.GetPixelGray(x, y)!=128)
				{
					POINT p;
					p.x = x;
					p.y = y;
					vec_bound.push_back(p);
					break;
				}
			}
		}
	}
}