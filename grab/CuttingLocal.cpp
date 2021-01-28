//#include "Grabcut.h"
//#include "../GrabCut/Cutting.h"
//#include "../GrabCut/gmm.h"
//#include "../GrabCut/Global.h"
//#include <iostream>
//
//using namespace std;
//
//void Grabcut::GetLocalCuttingRegion(vector<POINT> &pixels, CLUSTER *fgCluster, CLUSTER *bgCluster)
//{
//     int i,index;
//     int size;
//     double data[3];
//     POINT pixel;
//
//     fgPixelsNum=0;
//     bgPixelsNum=0;
//     size=(int)pixels.size();
//
//     for(i=0;i<size;++i)
//     {
//          pixel=pixels[i];
//          index=pixel.y*width+pixel.x;
//          GetData(originalImage, pixel.x, pixel.y, data);
//          if(label[index]==1)
//          {
//               index=3*fgPixelsNum;
//               fgPixels[index]=data[0];
//               fgPixels[index+1]=data[1];
//               fgPixels[index+2]=data[2];
//               ++fgPixelsNum;
//          }
//          else
//          {
//               index=3*bgPixelsNum;
//               bgPixels[index]=data[0];
//               bgPixels[index+1]=data[1];
//               bgPixels[index+2]=data[2];
//               ++bgPixelsNum;	
//          }
//     }
//
//     GMM::times=2;
//     GMM::InitGMM(bgPixels, bgPixelsNum, bgCluster->clusterNum, bgCluster->mean, 
//                  bgCluster->weight, bgCluster->det, bgCluster->cov);
//
//     GMM::InitGMM(fgPixels, fgPixelsNum, fgCluster->clusterNum, fgCluster->mean, 
//                  fgCluster->weight, fgCluster->det, fgCluster->cov);
//
//     GMM::GetGMM(bgPixels, bgPixelsNum, bgCluster->clusterNum, bgCluster->mean, 
//                 bgCluster->weight, bgCluster->det, bgCluster->cov);
//
//     GMM::GetGMM(fgPixels, fgPixelsNum, fgCluster->clusterNum, fgCluster->mean, 
//                 fgCluster->weight, fgCluster->det, fgCluster->cov);
//}
//
//void Grabcut::InitLocalGraph(vector<POINT> &pixels, CxImage &scrib, 
//                             CLUSTER *fgCluster, CLUSTER *bgCluster)
//{
//     int i,k;
//     int x1,y1;
//     int size;
//     int index1,index2;
//     double prob,beta;
//     double back,fore;
//     POINT pixel;
//     RGBQUAD rgb;
//     double K=100000000;
//     static const int direct[4][2]={{1,0},{0,1},{1,1},{1,-1}};
//
//     beta=GetBeta();
//     memset(graphPoint, 0, sizeof(void *)*pixelNum);
//     delete graph;
//     graph=new Graph;
//     size=(int)pixels.size();
//     for(i=0;i<size;++i)
//     {
//          pixel=pixels[i];
//          index1=pixel.y*width+pixel.x;
//          for(k=0;k<4;++k)
//          {
//               x1=pixel.x+direct[k][0];
//               y1=pixel.y+direct[k][1];
//               prob=CalNLinkWeight(beta, pixel.x, pixel.y, x1, y1);
//               if(prob!=-1)
//               {
//                    if(graphPoint[index1]==NULL)
//                    {
//                         graphPoint[index1]=graph->add_node();
//                         back=GetTLinkWeight(pixel.x, pixel.y, bgCluster);
//                         fore=GetTLinkWeight(pixel.x, pixel.y, fgCluster);
//                         graph->set_tweights(graphPoint[index1], (float)fore, (float)back);
//                    }
//                    index2=y1*width+x1;
//                    if(graphPoint[index2]==NULL)
//                    {
//                         graphPoint[index2]=graph->add_node();
//                         back=GetTLinkWeight(x1, y1, bgCluster);
//                         fore=GetTLinkWeight(x1, y1, fgCluster);
//                         graph->set_tweights(graphPoint[index2], (float)fore, (float)back);
//                    }
//                    graph->add_edge(graphPoint[index1], graphPoint[index2], (float)prob, (float)prob);
//               }
//          }
//     }
//
//     for(i=0;i<size;++i)
//     {
//          pixel=pixels[i];
//          rgb=scrib.GetPixelColor(pixel.x, pixel.y);
//          if(rgb.rgbRed==255)
//               graph->set_tweights(graphPoint[pixel.y*width+pixel.x], 0, (float)K);
//          else
//          if(rgb.rgbRed==0)
//               graph->set_tweights(graphPoint[pixel.y*width+pixel.x], (float)K, 0);
//          else
//          {
//               back=GetTLinkWeight(pixel.x, pixel.y, bgCluster);
//               fore=GetTLinkWeight(pixel.x, pixel.y, fgCluster);			
//               graph->set_tweights(graphPoint[pixel.y*width+pixel.x], (float)fore, (float)back);
//          }
//     }
//}
//
//void Grabcut::LocalUpdate(vector<POINT> &pixels, CLUSTER *fgCluster, CLUSTER *bgCluster)
//{
//     int i,k;
//     int index;
//     int size;
//     double back,fore;
//     double K=100000000;
//     double data[3];
//     POINT pixel;
//
//     bgPixelsNum=0;
//     fgPixelsNum=0;
//     size=(int)pixels.size();
//     for(i=0;i<size;++i)
//     {
//          pixel=pixels[i];
//          k=pixel.y*width+pixel.x;
//          GetData(*image, pixel.x, pixel.y, data);
//          if(graphPoint[k]==NULL)
//          {
//               index=3*bgPixelsNum;
//               bgPixels[index]=data[0];
//               bgPixels[index+1]=data[1];
//               bgPixels[index+2]=data[2];
//               ++bgPixelsNum;
//          }
//          else
//          {
//               if (graph->what_segment(graphPoint[k])==Graph::SOURCE)
//               {
//                    index=3*bgPixelsNum;
//                    bgPixels[index]=data[0];
//                    bgPixels[index+1]=data[1];
//                    bgPixels[index+2]=data[2];
//                    ++bgPixelsNum;
//                    label[k]=0;
//               }
//               else
//               {
//                    index=3*fgPixelsNum;
//                    fgPixels[index]=data[0];
//                    fgPixels[index+1]=data[1];
//                    fgPixels[index+2]=data[2];
//                    ++fgPixelsNum;
//                    label[k]=1;
//               }
//          }
//     }
//
//     GMM::times=1;
//     GMM::GetGMM(bgPixels, bgPixelsNum, bgCluster->clusterNum, bgCluster->mean, 
//          bgCluster->weight, bgCluster->det, bgCluster->cov);
//
//     GMM::GetGMM(fgPixels, fgPixelsNum, fgCluster->clusterNum, fgCluster->mean, 
//          fgCluster->weight, fgCluster->det, fgCluster->cov);
//
//     //Update Graph
//     for(i=0;i<size;++i)
//     {
//          pixel=pixels[i];
//          index=pixel.y*width+pixel.x;
//          back=GetTLinkWeight(pixel.x, pixel.y, bgCluster);
//          fore=GetTLinkWeight(pixel.x, pixel.y, fgCluster);			
//          graph->set_tweights(graphPoint[index], (float)fore, (float)back);
//     }
//}
//
//void Grabcut::LocalCalculate(vector<POINT> &pixels, CxImage &scrib)
//{
//     int Times=4;
//     int i;
//     Cluster *FG,*BG;
//
//     BG=new Cluster(3);	
//     FG=new Cluster(3);
//
//     GetLocalCuttingRegion(pixels, FG, BG);
//     InitLocalGraph(pixels, scrib, FG, BG);
//
//     for(i=0;i<Times;++i)
//     {
//          graph->maxflow();
//          LocalUpdate(pixels, FG, BG);
//     }
//
//     delete BG;
//     delete FG;
//}
//
//void Grabcut::UpdateLabel(CxImage &scrib)
//{
//     int i,j;
//     RGBQUAD rgb;
//
//     for(i=0;i<height;++i)
//     {
//          for(j=0;j<width;++j)
//          {
//               rgb=scrib.GetPixelColor(j, i);
//               if(rgb.rgbRed==0)
//                    label[i*width+j]=0;
//               else
//               if(rgb.rgbRed==255)
//                    label[i*width+j]=1;
//          }
//     }
//}
//
//void Grabcut::UpdateLabel(CxImage &alpha, CxImage &scrib)
//{
//     int i,j;
//     RGBQUAD rgb;
//
//     for(i=0;i<height;++i)
//     {
//          for(j=0;j<width;++j)
//          {            
//               rgb=alpha.GetPixelColor(j, i);
//               if(rgb.rgbRed==0)
//                    label[i*width+j]=0;
//               else
//               if(rgb.rgbRed==255)
//                    label[i*width+j]=1;
//          }
//     }
//
//     for(i=0;i<height;++i)
//     {
//          for(j=0;j<width;++j)
//          {
//               rgb=scrib.GetPixelColor(j, i);
//               if(rgb.rgbRed==0)
//                    label[i*width+j]=0;
//               else
//               if(rgb.rgbRed==255)
//                    label[i*width+j]=1;
//          }
//     }
//}
//
//
//double Grabcut::GetBeta()
//{
//	int i,j;
//	int k,count;
//	int x,y;
//	int index1,index2;
//	double Z;
//	double tmp1,tmp2,tmp3;
//	RGBQUAD rgb1,rgb2;
//	int direct[4][2]={{1,0},{0,1},{1,1},{1,-1}};
//
//	Z=0;
//	count=0;
//	for(i=bottom;i<=top;++i)
//	{
//		for(j=left;j<=right;++j)
//		{
//			rgb1=image->GetPixelColor(j, i);
//			index1=i*width+j;
//			for(k=0;k<4;++k)
//			{
//				y=i+direct[k][1];
//				x=j+direct[k][0];
//				index2=y*width+x;
//				if(IsValid(x, y))
//				{
//					rgb2=image->GetPixelColor(x, y);
//					tmp1=rgb1.rgbRed-rgb2.rgbRed;
//					tmp2=rgb1.rgbGreen-rgb2.rgbGreen;
//					tmp3=rgb1.rgbBlue-rgb2.rgbBlue;
//					Z+=tmp1*tmp1+tmp2*tmp2+tmp3*tmp3;
//					count++;
//				}
//			}
//		}
//	}
//	Z/=count;	
//	return 0.5/Z;
//}