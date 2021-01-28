#include "StdAfx.h"
#include "ClosedFormMatting.h"
//#include "globalMath.h"
#include <vector>
#include "WmlMathLib.h"
//#include "globalpublic.h"
#include "cximage.h"
#include "CxImageFun.h"
#include "ImageConvert.h"
#include "SparseMatrix.h"
#include "SparseMatrix_ListType.h"
//#include "TaucsSparseMatrix.h"
#include "MatrixUtil.h"
#include "WmlLinearSystem.h"
//#include "TimeUtil.h"

using namespace MattingAlgorithm;

CClosedFormMatting::CClosedFormMatting(void)
{
	m_thr_alpha = 0.02;;
	epsilon = 1.0000e-007;
	win_size = 1;
}

CClosedFormMatting::~CClosedFormMatting(void)
{

}

//Input & output Interface: if not available, fill 0
bool CClosedFormMatting::ImageSolve(ZFloatImage* pSrcImg, /*Source Input Image*/
									ZByteImage* pTriMap,/*Input Trimap*/
									ZFloatImage* pBgPrior, /*Input Bg Prior: (r,g,b,w)*/
									ZFloatImage* pDataCost, /*Input Data cost Prior: (bg,fg,weight)*/
									ZFloatImage* pSmoothCost, /*Input Smoothness Prior: (EAST,SOUTH,WEST,NORTH,weight)*/	

									ZByteImage* pAlpha, /*Output Alpha*/
									ZFloatImage* pBgImg, /*Output Bg Image*/
									ZFloatImage* pFgImg /*Output Fg Image*/
									)
{
	ZFloatImage* alphaImg;

	//alphaImg = SolveAlpha_Generic(*pSrcImg,*pTriMap,pDataCost,pSmoothCost);
	alphaImg = SolveAlpha(*pSrcImg,*pTriMap);
	
	FloatToByte(*alphaImg,*pAlpha);



	if(pFgImg && pBgImg){
		SolveFB(*pSrcImg,*alphaImg,*pFgImg,*pBgImg);
	}

	delete alphaImg;	
	

	return true;
}

//Input & output Interface: if not available, fill 0
bool CClosedFormMatting::VideoSolve(FloatImgList* pSrcImg, /*Source Input Images*/
									ByteImgList* pTriMap,/*Input Trimaps*/
									FloatImgList* pBgPrior, /*Input Bg Priors: (r,g,b,w)*/
									FloatImgList* pDataCost, /*Input Data cost Priors: (bg,fg,weight)*/
									FloatImgList* pSmoothCost, /*Input Smoothness Priors: (EAST,SOUTH,WEST,NORTH,weight)*/	
									IntImgList* pTMaps, /*Input termproal prior: (x1,y1,x2,y2)*/
									bool bStaticCamera, /*Camera is static or not*/

									ByteImgList* pAlpha, /*Output Alpha Imgs*/
									FloatImgList* pBgImg, /*Output Bg Images*/
									FloatImgList* pFgImg /*Output Fg Images*/
									)
{

	return true;

}


void CClosedFormMatting::Test()
{
	ZFloatImage srcImg,diffImg;
	CxImage I,mI;
	I.Load("D:\\projects\\VideoMatting_VC7\\test\\mid-teddy.bmp",GetImageFileType(".bmp"));
	mI.Load("D:\\projects\\VideoMatting_VC7\\test\\mid-teddy_m.bmp",GetImageFileType(".bmp"));

	CxToZFloatImg(I,srcImg);
	CxToZFloatImg(mI,diffImg);

	ZByteImage triMap;

	int iWidth = srcImg.GetWidth();
	int iHeight = srcImg.GetHeight();

	RGBQUAD color1,color2;
	triMap.Create(iWidth,iHeight);
	for(int j=0;j<iHeight;++j)
		for(int i=0;i<iWidth;++i){
			float diff = fabs(srcImg.at(i,j,0) - diffImg.at(i,j,0)) + fabs(srcImg.at(i,j,0) - diffImg.at(i,j,0))
				+ fabs(srcImg.at(i,j,0) - diffImg.at(i,j,0));
			if(diff>0.001){
				if(diffImg.at(i,j,0)+diffImg.at(i,j,1)+diffImg.at(i,j,2)<0.001){
					triMap.at(i,j) = REGION_BG;
				}
				else{
					triMap.at(i,j) = REGION_FG;
				}
			}
			else{
				triMap.at(i,j) = REGION_UNKNOWN;
			}
		}

		//CxImage testImg;
		//ByteToCxImg(triMap,testImg);
		//testImg.Save("D:\\projects\\VideoMatting_VC7\\test\\test.bmp",GetImageFileType(".bmp"));
		//printf("test.bmp");
		RunMatting(srcImg,triMap);
}

void CClosedFormMatting::Test2()
{
	ZFloatImage srcImg;
	CxImage I,triI;
	I.Load("D:\\projects\\VideoMatting_VC7\\test\\test0300.jpg",GetImageFileType(".jpg"));
	triI.Load("D:\\projects\\VideoMatting_VC7\\test\\trimap.bmp",GetImageFileType(".bmp"));

	CxToZFloatImg(I,srcImg);


	int iWidth = triI.GetWidth();
	int iHeight = triI.GetHeight();
	ZByteImage triMap(iWidth,iHeight);

	for(int j=0;j<iHeight;++j)
		for(int i=0;i<iWidth;++i){
			RGBQUAD color = triI.GetPixelColor(i,iHeight-1-j);
			triMap.at(i,j) = color.rgbGreen;
		}



		ZFloatImage bgImg;
		bgImg.Create(iWidth,iHeight,4);

		CxImage image;

		//image.Load("D:\\projects\\VideoMatting_VC7\\test\\test0_bgsmp.png",GetImageFileType(".png"));
		image.Load("c:\\test\\BB.png",GetImageFileType(".png"));
		CxToZFloatImg(image,bgImg);

		/*
		RGBQUAD color;
		for(int i=0; i<iWidth; ++i){
		for(int j=0; j<iHeight; ++j){
		double r=0,g=0,b=0,w=0;
		std::vector<RGBQUAD> colorlist;
		colorlist.reserve(BG_OTHERFRAME_SAMPLE_COUNT);
		for(int k=0;k<BG_OTHERFRAME_SAMPLE_COUNT;k++){
		color = image.GetPixelColor(i,(iHeight-1-j)+iHeight*k,true);
		if(color.rgbReserved>100){
		colorlist.push_back(color);
		r += color.rgbRed * color.rgbReserved;
		g += color.rgbGreen * color.rgbReserved;
		b += color.rgbBlue * color.rgbReserved;
		w += color.rgbReserved;
		}
		}
		r /= w;
		g /= w;
		b /= w;
		w /= (int)colorlist.size();


		if(colorlist.size()>=2){
		//double dist2=0;
		//std::vector<RGBQUAD>::iterator iter = colorlist.begin();
		//for(; iter!=colorlist.end(); ++iter){
		//	double w2 = (iter->rgbReserved * iter->rgbReserved)/(255.0*255.0);
		//	dist2 += (r - iter->rgbRed)*(r - iter->rgbRed)*w2 + (g - iter->rgbGreen)*(g - iter->rgbGreen)*w2
		//		+ (b - iter->rgbBlue)*(g - iter->rgbBlue)*w2;			
		//}

		bgImg.at(i,j,0) = r/255.0;
		bgImg.at(i,j,1) = g/255.0;
		bgImg.at(i,j,2) = b/255.0;
		bgImg.at(i,j,3) = w/255.0;
		}
		else{
		bgImg.at(i,j,0) = 0;
		bgImg.at(i,j,1) = 0;
		bgImg.at(i,j,2) = 0;
		bgImg.at(i,j,3) = 0;
		}

		}
		}
		*/
		//FloatToCxImg(bgImg,image);
		//image.Save("c:\\test\\sample-bg.bmp",GetImageFileType(".bmp"));

		//Save Diff;
		//ZFloatImage diff(iWidth,iHeight,3);
		//for(int j=0;j<iHeight;++j)
		//	for(int i=0;i<iWidth;++i){
		//		if(bgImg.at(i,j,3)!=0){
		//			float dist = 0;
		//			for(int k=0;k<3;++k)
		//				dist += fabs(srcImg.at(i,j,k) - bgImg.at(i,j,k))/3.0;
		//			for(int k=0;k<3;++k)
		//				diff.at(i,j,k) = dist;
		//		}			
		//	}

		//FloatToCxImg(diff,image);
		//image.Save("c:\\test\\sample-diff.bmp",GetImageFileType(".bmp"));



		RunMatting(srcImg,triMap,bgImg);
		//RunMatting(srcImg,triMap);
}

ZFloatImage* CClosedFormMatting::RunMatting(ZFloatImage& srcImg, ZByteImage& triMap)
{

	//ZFloatImage* alpha = SolveAlphaC2F(srcImg,triMap,2,2);
	ZFloatImage* alpha = SolveAlpha(srcImg,triMap);

	ZFloatImage F,B;
	F.CreateAndInit(srcImg.GetWidth(),srcImg.GetHeight(),srcImg.GetChannel()+1);
	B.CreateAndInit(srcImg.GetWidth(),srcImg.GetHeight(),srcImg.GetChannel()+1);

	//CxImage image;
	//FloatToCxImg(*alpha,image);

	//image.Save("C:\\test\\normal-alpha.bmp",GetImageFileType(".bmp"));

	SolveFB(srcImg,*alpha,F,B);

	//FloatToCxImg(F,image);
	//image.Save("C:\\test\\F.png",GetImageFileType(".png"));
	//FloatToCxImg(B,image);
	//image.Save("C:\\test\\B.png",GetImageFileType(".png"));

	return alpha;
}

bool CClosedFormMatting::RunMatting(ZFloatImage& srcImg,ZByteImage& triMap,ZFloatImage& bgInfo)
{
	ZFloatImage* alpha = SolveAlpha_WithBG(srcImg,triMap,bgInfo);

	ZFloatImage F,B;
	F.CreateAndInit(srcImg.GetWidth(),srcImg.GetHeight(),srcImg.GetChannel()+1);
	B.CreateAndInit(srcImg.GetWidth(),srcImg.GetHeight(),srcImg.GetChannel()+1);

	//CxImage image;
	//FloatToCxImg(*alpha,image);

	//image.Save("C:\\test\\normal-alpha.bmp",GetImageFileType(".bmp"));

	SolveFB(srcImg,*alpha,F,B);

	//FloatToCxImg(F,image);
	//image.Save("C:\\test\\F.png",GetImageFileType(".png"));
	//FloatToCxImg(B,image);
	//image.Save("C:\\test\\B.png",GetImageFileType(".png"));


	delete alpha;

	return true;
}

CSparseMatrix_ListType* CClosedFormMatting::_GetLaplacian(ZFloatImage& srcImg, ZByteImage& triMap, Wml::GVectord& b, ZIntImage& indsM)
{
	int neb_size = (win_size*2+1)*(win_size*2+1);
	int w = srcImg.GetWidth();
	int h = srcImg.GetHeight();
	int c = srcImg.GetChannel();
	int n = h;
	int m = w;
	int img_size = w*h;
	ZIntVector win_inds;
	//ZIntImage indsM;
	int vairableCount=0;
	Wml::GMatrixd winI,win_mu,win_var,tvals;
	Wml::GMatrixd IdentityM;
	int index=0;

	int tlen=0;
	std::vector<int> row_inds,col_inds;
	std::vector<double> vals;

	//need imerode
	ZByteImage constsMap,o_constsMap, constsMap2;
	_imErode(triMap,o_constsMap,constsMap);
	constsMap2 = constsMap;
	_imErode(constsMap2);


	for(int j=0;j<h;++j)
		for(int i=0;i<w;++i){
			tlen += constsMap.at(i,j);			
		}
		tlen = (w*h-tlen)*neb_size*neb_size;

		indsM.CreateAndInit(w,h,1,-1);
		win_inds.Create(neb_size);
		winI.SetSize(neb_size,c);
		win_mu.SetSize(c,1);
		win_var.SetSize(c,c);
		tvals.SetSize(neb_size,neb_size);
		IdentityM.SetSize(c,c);
		for(int i=0;i<c;++i){
			for(int j=0;j<c;++j)
				IdentityM(i,j) = 0;
			IdentityM(i,i) = 1.0;
		}

		vairableCount=0;	
		for(int j=0;j<h;++j)
			for(int i=0;i<w;++i){
				if(constsMap2.at(i,j)==0){
					indsM.at(i,j) = vairableCount++;
				}
				//else
				//	indsM.at(i,j) = -1;
			}


			row_inds.resize(tlen);
			col_inds.resize(tlen);
			vals.resize(tlen);
			for(int i=0;i<tlen;++i){
				row_inds[i] = 0;
				col_inds[i] = 0;
				vals[i] = 0;
			}

			int len = 0;

			for(int i=win_size;i<w-win_size;++i)
				for(int j=win_size;j<h-win_size;++j){		
					if(constsMap.at(i,j)!=0)
						continue;
					//Get win_inds & winI;
					index=0;
					for(int kj=j-win_size;kj<=j+win_size;++kj)
						for(int ki=i-win_size;ki<=i+win_size;++ki){
							for(int kk=0;kk<c;++kk)
								winI(index,kk) = srcImg.at(ki,kj,kk);						
							win_inds.at(index) = indsM.at(ki,kj);	
							if(indsM.at(ki,kj)==-1)
								printf("error");
							++index;
						}


						//win_mu: mean value
						for(int kj=0;kj<c;++kj){
							win_mu(kj,0) = 0;
							for(int ki=0;ki<neb_size;++ki)				
								win_mu(kj,0) += winI(ki,kj);
							win_mu(kj,0) /= neb_size;

						}


						//Wml::GMatrixd tmpwin_var= (winI.Transpose() * winI)/neb_size - win_mu*win_mu.Transpose() +
						//	epsilon/neb_size*IdentityM;

						//Wml::LinearSystemd::Inverse(tmpwin_var,win_var); 

						_CalWinVar(win_var,winI,win_mu);

						for(int ki=0;ki<neb_size;++ki){
							for(int kj=0;kj<c;++kj)
								winI(ki,kj) -= win_mu(kj,0);
						}


						//tvals = (winI*win_var*winI.Transpose())/neb_size;
						_CalTvals(tvals,winI,win_var);


						for(int ki=0;ki<neb_size;++ki)
							for(int kj=0;kj<neb_size;++kj)
								tvals(ki,kj) += 1.0/neb_size;

						//printf("tvals:\n");
						//for(int ki=0;ki<neb_size;++ki){
						//	for(int kj=0;kj<neb_size;++kj)
						//		printf("%f\t",tvals(ki,kj));
						//	printf("\n");
						//}

						index = 0;
						for(int ki=0;ki<neb_size;++ki){
							for(int kj=0;kj<neb_size;++kj){
								row_inds[len+index] = win_inds.at(ki);
								col_inds[len+index] = win_inds.at(kj);
								vals[len+index] = tvals(ki,kj);
								++index;
							}				
						}
						len += neb_size*neb_size;
				}



		//CSparseMatrix* pMtx = _CreateSparseMatrix(row_inds,col_inds,vals,img_size,img_size);
		CSparseMatrix_ListType* pMtx = _CreateSparseMatrix(row_inds,col_inds,vals,vairableCount,vairableCount);
		printf("variable count:%d, image size:%d\n",vairableCount,img_size);
		printf("matrix unzero count:%d\n",pMtx->GetElementCount());


		Wml::GVectord D(vairableCount);
		pMtx->GetRowSum(D);
		pMtx->Scale(-1.0);


		for(int i=0;i<vairableCount;++i){
			pMtx->Element_Add(i,i,D[i]);
			//printf("%f\t",D[i]);
			//printf("%f\n",pMtx->GetElement(i,i));
		}


		b.SetSize(vairableCount);

		double lambda=100;
		index=0;
		for(int j=0;j<h;++j)
			for(int i=0;i<w;++i){
				if(constsMap2.at(i,j)==0){
					switch(triMap.at(i,j))
					{
					case REGION_BG:
					case REGION_BG_EDIT:							
					case REGION_FG:			
						pMtx->Element_Add(indsM.at(i,j),indsM.at(i,j),lambda);		
						break;			
					}		
				}
			}		

		for(int j=0;j<h;++j)
			for(int i=0;i<w;++i){
				if(constsMap2.at(i,j)==0){
					switch(triMap.at(i,j))
					{
					case REGION_FG:
						b[indsM.at(i,j)] = lambda;
						break;
					default:
						b[indsM.at(i,j)] = 0;
						break;
					}			
				}
			}

			return pMtx;
}



CSparseMatrix_ListType* CClosedFormMatting::_GetLaplacian_WithBG(ZFloatImage& srcImg, ZByteImage& triMap, ZFloatImage& bgImg, 
																 Wml::GVectord& b, ZIntImage& indsM)
{
	int neb_size = (win_size*2+1)*(win_size*2+1);
	int w = srcImg.GetWidth();
	int h = srcImg.GetHeight();
	int c = srcImg.GetChannel();
	int n = h;
	int m = w;
	int img_size = w*h;
	ZIntVector win_inds;
	//ZIntImage indsM;
	int vairableCount=0;
	Wml::GMatrixd winI,win_var,tvals;
	Wml::GMatrixd IdentityM;
	int index=0;

	int tlen=0;
	std::vector<int> row_inds,col_inds;
	std::vector<double> vals;

	//need imerode
	ZByteImage constsMap,o_constsMap, constsMap2;
	_imErode(triMap,o_constsMap,constsMap);
	constsMap2 = constsMap;
	_imErode(constsMap2);


	for(int j=0;j<h;++j)
		for(int i=0;i<w;++i){
			tlen += constsMap.at(i,j);			
		}
		tlen = (w*h-tlen)*neb_size*neb_size;

		indsM.Create(w,h);
		win_inds.Create(neb_size);
		winI.SetSize(neb_size,c+1);		
		win_var.SetSize(c+1,c+1);
		tvals.SetSize(neb_size,neb_size);
		IdentityM.SetSize(c+1,c+1);		
		for(int i=0;i<IdentityM.GetRows();++i){
			for(int j=0;j<IdentityM.GetColumns();++j){
				if(i==j&&i<c)
					IdentityM(i,j) = 1;
				else
					IdentityM(i,j) = 0;
			}			
		}

		vairableCount=0;	
		for(int j=0;j<h;++j)
			for(int i=0;i<w;++i){
				if(constsMap2.at(i,j)==0){
					indsM.at(i,j) = vairableCount++;
				}
			}


			row_inds.resize(tlen);
			col_inds.resize(tlen);
			vals.resize(tlen);
			for(int i=0;i<tlen;++i){
				row_inds[i] = 0;
				col_inds[i] = 0;
				vals[i] = 0;
			}

			int len = 0;

			for(int i=win_size;i<w-win_size;++i)
				for(int j=win_size;j<h-win_size;++j){		
					if(constsMap.at(i,j)!=0)
						continue;
					//Get win_inds & winI;
					index=0;
					double ww=1.0;
					double colordiff=0.0;

					for(int kj=j-win_size;kj<=j+win_size;++kj)
						for(int ki=i-win_size;ki<=i+win_size;++ki){							
							ww = min(ww,bgImg.at(ki,kj,c));
							for(int kk=0;kk<c;++kk){
								colordiff += (srcImg.at(ki,kj,kk) - bgImg.at(ki,kj,kk))*(srcImg.at(ki,kj,kk) - bgImg.at(ki,kj,kk));
							}
						}
						colordiff = colordiff/neb_size;

						ww = ww*exp(-colordiff/(2*0.1*0.1*3));

						//ww=0;
						for(int kj=j-win_size;kj<=j+win_size;++kj)
							for(int ki=i-win_size;ki<=i+win_size;++ki){
								for(int kk=0;kk<c;++kk){
									winI(index,kk) = srcImg.at(ki,kj,kk) - bgImg.at(ki,kj,kk) * ww;								
								}							
								winI(index,c) = 1.0 - ww;							
								win_inds.at(index) = indsM.at(ki,kj);							
								++index;
							}

							Wml::GMatrixd tmpwin_var= (winI.Transpose() * winI) + epsilon * IdentityM;
							Wml::LinearSystemd::Inverse(tmpwin_var,win_var);		
							tvals = winI*win_var*winI.Transpose();

							for(int ki=0;ki<neb_size;++ki){
								for(int kj=0;kj<neb_size;++kj)
									tvals(ki,kj) =  - tvals(ki,kj);
								tvals(ki,ki) += 1.0;
							}

							index = 0;
							for(int ki=0;ki<neb_size;++ki){
								for(int kj=0;kj<neb_size;++kj){
									row_inds[len+index] = win_inds.at(ki);
									col_inds[len+index] = win_inds.at(kj);
									vals[len+index] = tvals(ki,kj);
									++index;
								}				
							}
							len += neb_size*neb_size;
				}



				CSparseMatrix_ListType* pMtx = _CreateSparseMatrix(row_inds,col_inds,vals,vairableCount,vairableCount);
				printf("variable count:%d, image size:%d\n",vairableCount,img_size);
				printf("matrix unzero count:%d\n",pMtx->GetElementCount());


				b.SetSize(vairableCount);			

				double lambda=100;
				index=0;

				float colordist;
				for(int j=0;j<h;++j)
					for(int i=0;i<w;++i){
						if(constsMap2.at(i,j)==0){
							switch(triMap.at(i,j))
							{
							case REGION_FG:
								pMtx->Element_Add(indsM.at(i,j),indsM.at(i,j),lambda);
								b[indsM.at(i,j)] = lambda;
								break;
							case REGION_BG:
							case REGION_BG_EDIT:															
								pMtx->Element_Add(indsM.at(i,j),indsM.at(i,j),lambda);	
								b[indsM.at(i,j)] = 0;
								break;
							default:
								b[indsM.at(i,j)] = 0;
								int local_win = 2;
								if(i>local_win&&i<w-local_win&&j>local_win&&j<h-local_win&&bgImg.at(i,j,3)>0.5){

									colordist = (srcImg.at(i,j,0) - bgImg.at(i,j,0))*(srcImg.at(i,j,0) - bgImg.at(i,j,0)) + (srcImg.at(i,j,1) - bgImg.at(i,j,1))*(srcImg.at(i,j,1) - bgImg.at(i,j,1))
										+ (srcImg.at(i,j,2) - bgImg.at(i,j,2))*(srcImg.at(i,j,2) - bgImg.at(i,j,2));
									if(colordist < 0.05*0.05*3){
										//colordist = 0.1*exp(-colordist/(2.0*0.05*0.05))*bgImg.at(i,j,3);
										colordist = 0.1*exp(-colordist/(2.0*0.02*0.02*3))*bgImg.at(i,j,3);
										pMtx->Element_Add(indsM.at(i,j),indsM.at(i,j),colordist);
									}									
									
								}
							}		
						}
					}		


					return pMtx;
}

//CSparseMatrix* CClosedFormMatting::_GetLaplacian(ZFloatImage& srcImg, ZByteImage& triMap)
//{
//	int neb_size = (win_size*2+1)*(win_size*2+1);
//	int w = srcImg.GetWidth();
//	int h = srcImg.GetHeight();
//	int c = srcImg.GetChannel();
//	int n = h;
//	int m = w;
//	int img_size = w*h;
//	ZIntVector win_inds;
//	ZIntImage indsM;
//	Wml::GMatrixd winI,win_mu,win_var,tvals;
//	Wml::GMatrixd IdentityM;
//	int index=0;
//
//	int tlen=0;
//	std::vector<int> row_inds,col_inds;
//	std::vector<double> vals;
//
//	//need imerode
//	ZByteImage constsMap;
//	_imErode(triMap,constsMap);
//
//	for(int j=0;j<h;++j)
//		for(int i=0;i<w;++i){
//			tlen += constsMap.at(i,j);			
//		}
//	tlen = (w*h-tlen)*neb_size*neb_size;
//
//	indsM.Create(w,h);
//	win_inds.Create(neb_size);
//	winI.SetSize(neb_size,c);
//	win_mu.SetSize(c,1);
//	win_var.SetSize(c,c);
//	tvals.SetSize(neb_size,neb_size);
//	IdentityM.SetSize(c,c);
//	for(int i=0;i<c;++i){
//		for(int j=0;j<c;++j)
//			IdentityM(i,j) = 0;
//		IdentityM(i,i) = 1.0;
//	}
//	index=0;
//	//for(int i=0;i<w;++i)
//	for(int j=0;j<h;++j)
//		for(int i=0;i<w;++i){
//			indsM.at(i,j) = index++;
//		}
//	
//
//	row_inds.resize(tlen);
//	col_inds.resize(tlen);
//	vals.resize(tlen);
//	for(int i=0;i<tlen;++i){
//		row_inds[i] = 0;
//		col_inds[i] = 0;
//		vals[i] = 0;
//	}
//
//	int len = 0;
//
//	for(int i=win_size;i<w-win_size;++i)
//		for(int j=win_size;j<h-win_size;++j){		
//			if(constsMap.at(i,j)!=0)
//				continue;
//			//Get win_inds & winI;
//			index=0;
//			for(int kj=j-win_size;kj<=j+win_size;++kj)
//				for(int ki=i-win_size;ki<=i+win_size;++ki){
//					for(int kk=0;kk<c;++kk)
//						winI(index,kk) = srcImg.at(ki,kj,kk);						
//					win_inds.at(index) = indsM.at(ki,kj);
//					++index;
//				}
//				
//			
//			//win_mu: mean value
//			for(int kj=0;kj<c;++kj){
//				win_mu(kj,0) = 0;
//				for(int ki=0;ki<neb_size;++ki)				
//					win_mu(kj,0) += winI(ki,kj);
//				win_mu(kj,0) /= neb_size;
//				//printf("%f\n",win_mu(kj,0));
//			}
//			//printf("\n");
//
//			//for(int ki=0;ki<neb_size;++ki){
//			//	for(int kj=0;kj<c;++kj)
//			//		printf("%f\t",winI(ki,kj));
//			//	printf("\n");
//			//}
//
//			//printf("\n");
//
//			Wml::GMatrixd tmpwin_var= (winI.Transpose() * winI)/neb_size - win_mu*win_mu.Transpose() +
//				epsilon/neb_size*IdentityM;
//
//			Wml::LinearSystemd::Inverse(tmpwin_var,win_var); 
//
//			//for(int ki=0;ki<c;++ki){
//			//	for(int kj=0;kj<c;++kj)
//			//		printf("%f\t",win_var(ki,kj));
//			//	printf("\n");
//			//}
//
//			
//			for(int ki=0;ki<neb_size;++ki){
//				for(int kj=0;kj<c;++kj)
//					winI(ki,kj) -= win_mu(kj,0);
//			}
//
//			//printf("\n");
//			//for(int ki=0;ki<neb_size;++ki){
//			//	for(int kj=0;kj<c;++kj)
//			//		printf("%f\t",winI(ki,kj));
//			//	printf("\n");
//			//}
//
//			tvals = (winI*win_var*winI.Transpose())/neb_size;
//			//printf("\n");
//			//for(int ki=0;ki<neb_size;++ki){
//			//	for(int kj=0;kj<neb_size;++kj){
//			//		printf("%f\t",tvals(ki,kj));
//			//	}
//			//	printf("\n");
//			//}
//
//
//			for(int ki=0;ki<neb_size;++ki)
//				for(int kj=0;kj<neb_size;++kj)
//					tvals(ki,kj) += 1.0/neb_size;
//
//			//printf("\n");
//			//for(int ki=0;ki<neb_size;++ki){
//			//	for(int kj=0;kj<neb_size;++kj){
//			//		printf("%f\t",tvals(ki,kj));
//			//	}
//			//	printf("\n");
//			//}
//
//			index = 0;
//			for(int ki=0;ki<neb_size;++ki){
//				for(int kj=0;kj<neb_size;++kj){
//				row_inds[len+index] = win_inds.at(ki);
//				col_inds[len+index] = win_inds.at(kj);
//				vals[len+index] = tvals(ki,kj);
//				++index;
//				}				
//			}
//			len += neb_size*neb_size;
//		}
//
//
//	//CSparseMatrix* pMtx = _CreateSparseMatrix(row_inds,col_inds,vals,img_size,img_size);
//	CSparseMatrix_ListType pMtx = _CreateSparseMatrix(row_inds,col_inds,vals,img_size,img_size);
//
//	//PrintSparseMatrix(pMtx);
//
//	Wml::GVectord D(img_size);
//	for(int i=0;i<img_size;++i){
//		D[i] = 0;
//		SparseElement::iterator pos;
//							
//		for(pos = pMtx->m_dataArray[i].begin();pos!=pMtx->m_dataArray[i].end();pos++)
//			D[i] += pos->second;				
//		
//	}
//	ScaleSparseMatrix(pMtx,-1.0);
//	//PrintSparseMatrix(pMtx);
//
//
//	for(int i=0;i<img_size;++i){
//		(*pMtx)(i,i) = D[i] + (*pMtx)(i,i);
//		//printf("%f\n",D[i]);
//	}
//
//	//PrintSparseMatrix(pMtx);
//
//	return pMtx;
//}

//CSparseMatrix* CClosedFormMatting::_CreateSparseMatrix(std::vector<int>& row_inds, std::vector<int>& col_inds, std::vector<double>& vals, 
//													   int nRows,int nCols)
//{
//	CSparseMatrix* pMtx = new CSparseMatrix(nRows,nCols);
//
//	int iCount = row_inds.size();
//
//	for(int i=0;i<iCount;++i){
//		(*pMtx)(row_inds[i],col_inds[i]) += vals[i];
//	}
//
//	return pMtx;
//}


CSparseMatrix_ListType* CClosedFormMatting::_CreateSparseMatrix(std::vector<int>& row_inds, std::vector<int>& col_inds, std::vector<double>& vals, 
																int nRows,int nCols)
{
	CSparseMatrix_ListType* pMtx = new CSparseMatrix_ListType(nRows,nCols);

	int iCount = row_inds.size();

	for(int i=0;i<iCount;++i){
		//(*pMtx)(row_inds[i],col_inds[i]) += vals[i];
		pMtx->Element_Add(row_inds[i],col_inds[i],vals[i]);
	}

	return pMtx;
}

ZFloatImage* CClosedFormMatting::SolveAlpha(ZFloatImage& srcImg, ZByteImage& triMap)
{
	double wtime;
	int w = srcImg.GetWidth();
	int h = srcImg.GetHeight();
	int c = srcImg.GetChannel();
	int img_size = w*h;

	ZIntImage indsM;
	Wml::GVectord b;
	CSparseMatrix_ListType* A = _GetLaplacian(srcImg,triMap,b,indsM);

	Wml::GVectord x;
	x.SetSize(b.GetSize());

	taucs_ccs_matrix* mtx = A->ToTaucsMatrix();


	delete A;
	printf("delete A time:%f\n",wtime);

	void* F = NULL;

	char* m_defFactor[]={"taucs.factor.LLT=true", NULL};


	if(taucs_linsolve(mtx,&F,1, x,b,m_defFactor,NULL)!=TAUCS_SUCCESS){
		printf("Linear Matrix No Solution!\n");
	}
	taucs_ccs_free(mtx);
	taucs_linsolve(NULL,&F,0,NULL,NULL,NULL,NULL);


	ZFloatImage* pAlphaImg = new ZFloatImage;
	pAlphaImg->Create(w,h);
	for(int j=0;j<h;++j){
		for(int i=0;i<w;++i){
			switch(triMap.at(i,j))
			{
			case REGION_BG:
			case REGION_BG_EDIT:
				pAlphaImg->at(i,j) = 0;
				break;
			case REGION_FG:
				pAlphaImg->at(i,j) = 1.0;
				break;
			default:
				pAlphaImg->at(i,j) = max(0,min(1,x[indsM.at(i,j)]));
				break;
			}						
		}		
	}

	return pAlphaImg;
}

ZFloatImage* CClosedFormMatting::SolveAlpha_WithBG(ZFloatImage& srcImg,ZByteImage& triMap,ZFloatImage& bgInfo)
{
	double wtime;
	int w = srcImg.GetWidth();
	int h = srcImg.GetHeight();
	int c = srcImg.GetChannel();
	int img_size = w*h;

	ZIntImage indsM;
	Wml::GVectord b;
	CSparseMatrix_ListType* A = _GetLaplacian_WithBG(srcImg,triMap,bgInfo,b,indsM);

	Wml::GVectord x;
	x.SetSize(b.GetSize());

	taucs_ccs_matrix* mtx = A->ToTaucsMatrix();


	delete A;
	printf("delete A time:%f\n",wtime);

	void* F = NULL;

	char* m_defFactor[]={"taucs.factor.LLT=true", NULL};


	if(taucs_linsolve(mtx,&F,1, x,b,m_defFactor,NULL)!=TAUCS_SUCCESS){
		printf("Linear Matrix No Solution!\n");
	}
	taucs_ccs_free(mtx);
	taucs_linsolve(NULL,&F,0,NULL,NULL,NULL,NULL);

	printf("Solve Time:%f\n",wtime);

	ZFloatImage* pAlphaImg = new ZFloatImage;
	pAlphaImg->Create(w,h);
	for(int j=0;j<h;++j){
		for(int i=0;i<w;++i){
			switch(triMap.at(i,j))
			{
			case REGION_BG:
			case REGION_BG_EDIT:
				pAlphaImg->at(i,j) = 0;
				break;
			case REGION_FG:
				pAlphaImg->at(i,j) = 1.0;
				break;
			default:
				pAlphaImg->at(i,j) = max(0,min(1,x[indsM.at(i,j)]));
				break;
			}						
		}		
	}

	return pAlphaImg;

}

ZFloatImage* CClosedFormMatting::SolveAlpha_Generic(ZFloatImage& srcImg, ZByteImage& triMap, ZFloatImage* pDataCost, ZFloatImage* pSmoothCost)
{
	double wtime;
	int w = srcImg.GetWidth();
	int h = srcImg.GetHeight();
	int c = srcImg.GetChannel();
	int img_size = w*h;

	ZIntImage indsM;
	Wml::GVectord b;
	CSparseMatrix_ListType* A = _GetLaplacian(srcImg,triMap,b,indsM);


	//Add Data Cost Constraint
	if(pDataCost != NULL){
		for(int j=0; j<h; ++j)
			for(int i=0; i<w; ++i){
				if(indsM.at(i,j) >= 0 && pDataCost->at(i,j,2) > 0.01){
					//Assuming data cost is already normalized!
					double sw = fabs(pDataCost->at(i,j,0) - pDataCost->at(i,j,1))/(pDataCost->at(i,j,0) + pDataCost->at(i,j,1));
					sw *= pDataCost->at(i,j,2);
					sw *= 0.1;
					A->Element_Add(indsM.at(i,j),indsM.at(i,j),sw);
					//If bias to foreground
					if(pDataCost->at(i,j,0) > pDataCost->at(i,j,1)){
						b[indsM.at(i,j)] += sw;
					}
					
					//printf("w=%f\t",w);
				}
			}
	}


	//Add Smoothness Constraint
/*	if(pSmoothCost != NULL){
		for(int j=0; j<h; ++j)
			for(int i=0; i<w; ++i){
				if(indsM.at(i,j) >= 0 && pSmoothCost->at(i,j,4) > 0.01){
					
					int inds_xi = indsM.at(i,j);
					int inds_xj = -1;

					//East
					{
						double sw = pSmoothCost->at(i,j,0);
						inds_xj = indsM.at(i+1,j);

						sw *= 1.0;
						if(inds_xj>=0){						
							A->Element_Add(inds_xi,inds_xi,sw);
							A->Element_Add(inds_xi,inds_xj,-sw);
							A->Element_Add(inds_xj,inds_xi,-sw);
							A->Element_Add(inds_xj,inds_xj,sw);
						}						
					}
				
					//South
					{
						double sw = pSmoothCost->at(i,j,1);
						inds_xj = indsM.at(i,j+1);

						sw *= 1.0;
						if(inds_xj>=0){
							A->Element_Add(inds_xi,inds_xi,sw);
							A->Element_Add(inds_xi,inds_xj,-sw);
							A->Element_Add(inds_xj,inds_xi,-sw);
							A->Element_Add(inds_xj,inds_xj,sw);
						}						
					}				

					//West
					{
						double sw = pSmoothCost->at(i,j,2);
						inds_xj = indsM.at(i-1,j);

						sw *= 1.0;
						if(inds_xj>=0){						
							A->Element_Add(inds_xi,inds_xi,sw);
							A->Element_Add(inds_xi,inds_xj,-sw);
							A->Element_Add(inds_xj,inds_xi,-sw);
							A->Element_Add(inds_xj,inds_xj,sw);
						}						
					}

					//North
					{
						double sw = pSmoothCost->at(i,j,3);
						inds_xj = indsM.at(i,j-1);

						sw *= 1.0;
						if(inds_xj>=0){
							A->Element_Add(inds_xi,inds_xi,sw);
							A->Element_Add(inds_xi,inds_xj,-sw);
							A->Element_Add(inds_xj,inds_xi,-sw);
							A->Element_Add(inds_xj,inds_xj,sw);
						}						
					}				
				}
			}
	}*/
	if(pSmoothCost != NULL){

		for(int j=0; j<h; ++j)
			for(int i=0; i<w; ++i){
				if(indsM.at(i,j) >= 0){
					int inds_xi = indsM.at(i,j);
					int inds_xj = -1;
					//East
					{
						double sw = pSmoothCost->at(i,j,0);
						inds_xj = indsM.at(i+1,j);

						sw *= 1.0;
						if(inds_xj>=0){						
							A->Element_Add(inds_xi,inds_xi,sw);
							A->Element_Add(inds_xi,inds_xj,-sw);
							A->Element_Add(inds_xj,inds_xi,-sw);
							A->Element_Add(inds_xj,inds_xj,sw);
						}			

					}

						//South
						{
							double sw = pSmoothCost->at(i,j,1);
							inds_xj = indsM.at(i,j+1);

							sw *= 1.0;
							if(inds_xj>=0){
								A->Element_Add(inds_xi,inds_xi,sw);
								A->Element_Add(inds_xi,inds_xj,-sw);
								A->Element_Add(inds_xj,inds_xi,-sw);
								A->Element_Add(inds_xj,inds_xj,sw);
							}						
						}				
				}

			}
	}
	else
	{
		for(int j=1; j<h-1; ++j)
			for(int i=1; i<w-1; ++i){
				//if(indsM.at(i,j) >= 0 && pSmoothCost->at(i,j,4) > 0.01){
				if(indsM.at(i,j) >= 0 ){

					int inds_xi = indsM.at(i,j);
					int inds_xj = -1;

					//East
					{
						inds_xj = indsM.at(i+1,j);

						//double sw = pSmoothCost->at(i,j,0);
						double sw = 0.2;

						double contrast = 0;

						for(int c=0; c<srcImg.GetChannel(); ++c){
							contrast += 255 * fabs(srcImg.at(i,j,c) - srcImg.at(i+1,j,c));
						}
						contrast /= srcImg.GetChannel();
						sw *= exp( -(contrast*contrast) / (2.0*10*10) );

						if(inds_xj>=0){						
							A->Element_Add(inds_xi,inds_xi,sw);
							A->Element_Add(inds_xi,inds_xj,-sw);
							A->Element_Add(inds_xj,inds_xi,-sw);
							A->Element_Add(inds_xj,inds_xj,sw);
						}						
					}

					//South
					{
						inds_xj = indsM.at(i,j+1);

						//double sw = pSmoothCost->at(i,j,1);						
						double sw = 0.2;

						double contrast = 0;

						for(int c=0; c<srcImg.GetChannel(); ++c){
							contrast += 255 * fabs(srcImg.at(i,j,c) - srcImg.at(i,j+1,c));
						}
						contrast /= srcImg.GetChannel();
						sw *= exp( -(contrast*contrast) / (2.0*10*10) );

						if(inds_xj>=0){
							A->Element_Add(inds_xi,inds_xi,sw);
							A->Element_Add(inds_xi,inds_xj,-sw);
							A->Element_Add(inds_xj,inds_xi,-sw);
							A->Element_Add(inds_xj,inds_xj,sw);
						}						
					}				

					//West
					{

						inds_xj = indsM.at(i-1,j);

						//double sw = pSmoothCost->at(i,j,2);
						double sw = 0.2;

						double contrast = 0;

						for(int c=0; c<srcImg.GetChannel(); ++c){
							contrast += 255 * fabs(srcImg.at(i,j,c) - srcImg.at(i-1,j,c));
						}
						contrast /= srcImg.GetChannel();
						sw *= exp( -(contrast*contrast) / (2.0*10*10) );

						if(inds_xj>=0){						
							A->Element_Add(inds_xi,inds_xi,sw);
							A->Element_Add(inds_xi,inds_xj,-sw);
							A->Element_Add(inds_xj,inds_xi,-sw);
							A->Element_Add(inds_xj,inds_xj,sw);
						}						
					}

					//North
					{

						inds_xj = indsM.at(i,j-1);

						//double sw = pSmoothCost->at(i,j,3);
						double sw = 0.2;

						double contrast = 0;

						for(int c=0; c<srcImg.GetChannel(); ++c){
							contrast += 255 * fabs(srcImg.at(i,j,c) - srcImg.at(i,j-1,c));
						}
						contrast /= srcImg.GetChannel();
						sw *= exp( -(contrast*contrast) / (2.0*10*10) );

						if(inds_xj>=0){
							A->Element_Add(inds_xi,inds_xi,sw);
							A->Element_Add(inds_xi,inds_xj,-sw);
							A->Element_Add(inds_xj,inds_xi,-sw);
							A->Element_Add(inds_xj,inds_xj,sw);
						}						
					}				
				}
			}
	}


	Wml::GVectord x;
	x.SetSize(b.GetSize());

	taucs_ccs_matrix* mtx = A->ToTaucsMatrix();


	delete A;
	printf("delete A time:%f\n",wtime);

	void* F = NULL;

	char* m_defFactor[]={"taucs.factor.LLT=true", NULL};


	if(taucs_linsolve(mtx,&F,1, x,b,m_defFactor,NULL)!=TAUCS_SUCCESS){
		printf("Linear Matrix No Solution!\n");
	}
	taucs_ccs_free(mtx);
	taucs_linsolve(NULL,&F,0,NULL,NULL,NULL,NULL);

	printf("Solve Time:%f\n",wtime);

	ZFloatImage* pAlphaImg = new ZFloatImage;
	pAlphaImg->Create(w,h);
	for(int j=0;j<h;++j){
		for(int i=0;i<w;++i){
			switch(triMap.at(i,j))
			{
			case REGION_BG:
			case REGION_BG_EDIT:
				pAlphaImg->at(i,j) = 0;
				break;
			case REGION_FG:
				pAlphaImg->at(i,j) = 1.0;
				break;
			default:
				pAlphaImg->at(i,j) = max(0,min(1,x[indsM.at(i,j)]));
				break;
			}						
		}		
	}

	return pAlphaImg;
}

ZFloatImage* CClosedFormMatting::SolveAlphaC2F(ZFloatImage& img,ZByteImage& triMap, 
											   int levels_num, int active_levels_num)
{
	ZFloatImage* alpha = 0;
	if(levels_num>1){
		CxImage image;
		//FloatToCxImg(img,image);
		//image.Save("c:\\test\\img.bmp",GetImageFileType("*.bmp"));

		ZFloatImage* s_img = _DownSmpImg(img);
		ZByteImage* s_triMap = _DownSmpTrimap(triMap);
		//Test 

		//FloatToCxImg(*s_img,image);
		//image.Save("c:\\test\\s_img.bmp",GetImageFileType("*.bmp"));
		//ByteToCxImg(*s_triMap,image);
		//image.Save("c:\\test\\s_triMapimg.bmp",GetImageFileType("*.bmp"));



		ZFloatImage* s_alpha = SolveAlphaC2F(*s_img, *s_triMap, levels_num-1, min(levels_num-1,active_levels_num));

		//FloatToCxImg(*s_alpha,image,255);
		//image.Save("c:\\test\\s_alpha.bmp",GetImageFileType("*.bmp"));


		alpha = _UpSampleAlphaUsingImg(*s_alpha,*s_img,img);
		delete s_img;
		delete s_triMap;
		delete s_alpha;

		FloatToCxImg(*alpha,image,255);
		image.Save("c:\\test\\alpha.bmp",GetImageFileType("*.bmp"));

		_UpdateTrimap(triMap,*alpha);
		delete alpha;
	}

	if(active_levels_num>=levels_num)
		alpha=SolveAlpha(img,triMap);

	return alpha;
}

void CClosedFormMatting::_imErode(ZByteImage& triMap, ZByteImage& o_constsMap, ZByteImage& constsMap)
{
	int iWidth = triMap.GetWidth();
	int iHeight = triMap.GetHeight();


	constsMap.Create(iWidth,iHeight);
	memset(constsMap.GetMap(),1,constsMap.GetSize());
	o_constsMap = constsMap;

	for(int j=0;j<iHeight;++j)
		for(int i=0;i<iWidth;++i){
			if(triMap.at(i,j)>REGION_BG_EDIT&&triMap.at(i,j)<REGION_FG){
				int x,y;					
				for(x=i-1;x<=i+1;++x)
					for(y=j-1;y<=j+1;++y){
						if(x>=0&&x<iWidth&&y>=0&&y<iHeight){
							constsMap.at(x,y) = 0;
						}
					}
					o_constsMap.at(i,j) = 0;
			}
		}
		//CxImage tempim;
		//ByteToCxImg(constsMap,tempim,255);
		//CxImage tempim2;
		//ByteToCxImg(o_constsMap, tempim2,255);
		//tempim.Save("H:/constsMap.bmp", CXIMAGE_FORMAT_BMP);
		//tempim2.Save("H:/o_constsMap.bmp", CXIMAGE_FORMAT_BMP);

}

void CClosedFormMatting::_Conv2(ZFloatImage& img, ZFloatVector& filterW, ZFloatVector& filterH)
{
	int iWidth = img.GetWidth();
	int iHeight = img.GetHeight();
	int iChannel = img.GetChannel();
	int hW = filterW.GetSize()/2;
	int hH = filterH.GetSize()/2;

	ZFloatImage tmpImg=img;
	//Horizontal Filter Pass
	for(int j=0;j<iHeight;++j)
		for(int i=0;i<iWidth;++i){
			for(int k=0;k<iChannel;++k){
				float value=0;
				for(int t=max(0,i-hW);t<=min(i+hW,iWidth-1);++t){
					value += tmpImg.at(t,j,k)*filterW.at(t-i+hW);
				}
				img.at(i,j,k) = value;
			}
		}

		tmpImg = img;
		for(int i=0;i<iWidth;++i)
			for(int j=0;j<iHeight;++j){			
				for(int k=0;k<iChannel;++k){
					float value=0;
					for(int t=max(0,j-hH);t<=min(j+hH,iHeight-1);++t){
						value += tmpImg.at(i,t,k)*filterW.at(t-j+hH);
					}
					img.at(i,j,k) = value;
				}
			}

}

ZByteImage* CClosedFormMatting::_DownSmpTrimap(ZByteImage& triMap)
{
	int iWidth = triMap.GetWidth();
	int iHeight = triMap.GetHeight();

	ZFloatImage consts_Map,consts_Vals;

	consts_Map.Create(iWidth,iHeight);
	consts_Vals.Create(iWidth,iHeight);


	for(int j=0;j<iHeight;++j)
		for(int i=0;i<iWidth;++i){
			switch(triMap.at(i,j))
			{
			case REGION_BG:
			case REGION_BG_EDIT:
				consts_Map.at(i,j) = 1.0;
				consts_Vals.at(i,j) = 0.0;
			case REGION_FG:
				consts_Map.at(i,j) = 1.0;
				consts_Vals.at(i,j) = 1.0;
			default:
				consts_Map.at(i,j) = 0.0;
				consts_Vals.at(i,j) = 0.0;
				break;
			}
		}

		ZFloatVector filter;
		filter.Create(5);
		filter.at(0) = 1.0/16.0;filter.at(1) = 4.0/16.0;filter.at(2) = 6.0/16.0;filter.at(3) = 4.0/16.0;filter.at(4) = 1.0/16.0;

		_Conv2(consts_Map,filter,filter);
		_Conv2(consts_Vals,filter,filter);


		ZByteImage tmpTriMap = triMap;
		for(int j=0;j<iHeight;++j)
			for(int i=0;i<iWidth;++i){
				int v1 = consts_Map.at(i,j)+0.5;
				int v2 = consts_Vals.at(i,j)+0.5;
				if(v1==1){
					if(v2==1)
						triMap.at(i,j) = REGION_FG;
					else
						triMap.at(i,j) = REGION_BG;
				}
				else{
					triMap.at(i,j) = REGION_UNKNOWN;
				}

				switch(triMap.at(i,j))
				{
				case REGION_BG:
				case REGION_BG_EDIT:
					consts_Map.at(i,j) = 1.0;
					consts_Vals.at(i,j) = 0.0;
				case REGION_FG:
					consts_Map.at(i,j) = 1.0;
					consts_Vals.at(i,j) = 1.0;
				default:
					consts_Map.at(i,j) = 0.0;
					consts_Vals.at(i,j) = 0.0;
					break;
				}
			}


			int iNewW = (iWidth - (filter.GetSize()-1))/2;
			int iNewH = (iHeight - (filter.GetSize()-1))/2;

			ZByteImage* s_triMap = new ZByteImage(iNewW,iNewH);

			int iStart = filter.GetSize()/2;

			for(int j=0;j<iNewH;++j)
				for( int i=0;i<iNewW;++i){
					s_triMap->at(i,j) = tmpTriMap.at(iStart + 2*i,iStart + 2*j);
				}

				return s_triMap;
}

void CClosedFormMatting::_CalTvals(Wml::GMatrixd& tvals, const Wml::GMatrixd& winI, const Wml::GMatrixd& win_var)
{
	int nRows = tvals.GetRows();
	int nCols = tvals.GetColumns();

	double a1,a2,a3,b1,b2,b3,t1,t2,t3;
	double c1,c2,c3,c4,c5,c6,c7,c8,c9;


	c1 = win_var(0,0);c2 = win_var(0,1);c3 = win_var(0,2);
	c4 = win_var(1,0);c5 = win_var(1,1);c6 = win_var(1,2);
	c7 = win_var(2,0);c8 = win_var(2,1);c9 = win_var(2,2);

	for(int i=0;i<nRows;++i)
		for(int j=0;j<nCols;++j){			
			a1 = winI(i,0);a2 = winI(i,1);a3 = winI(i,2);
			b1 = winI(j,0);b2 = winI(j,1);b3 = winI(j,2);
			t1 = a1*c1 + a2*c4 + a3*c7;
			t2 = a1*c2 + a2*c5 + a3*c8;
			t3 = a1*c3 + a2*c6 + a3*c9;
			tvals(i,j) = (t1*b1 + t2*b2 + t3*b3)/nRows;
		}

}

void CClosedFormMatting::_CalWinVar(Wml::GMatrixd& win_var, const Wml::GMatrixd& winI, const Wml::GMatrixd& win_mu)
{
	int nRows = winI.GetRows();
	int nCols = winI.GetColumns();

	static Wml::GMatrixd tmpwin_var(nCols,nCols);

	for(int i=0;i<nCols;++i)
		for(int j=0;j<nCols;++j){
			double value=0;
			for(int k=0;k<nRows;++k)
				value += winI(k,i)*winI(k,j);

			tmpwin_var(i,j) = value/nRows - win_mu(i,0)*win_mu(j,0);
		}

		for(int i=0;i<nCols;++i)
			tmpwin_var(i,i) += epsilon/nRows;


		Wml::LinearSystemd::Inverse(tmpwin_var,win_var); 
}

ZFloatImage* CClosedFormMatting::_DownSmpImg(ZFloatImage& img)
{
	int iWidth = img.GetWidth();
	int iHeight = img.GetHeight();
	int c = img.GetChannel();


	ZFloatImage tmpImg = img;
	ZFloatVector filter;
	filter.Create(5);
	filter.at(0) = 1.0/16.0;filter.at(1) = 4.0/16.0;filter.at(2) = 6.0/16.0;filter.at(3) = 4.0/16.0;filter.at(4) = 1.0/16.0;

	_Conv2(tmpImg,filter,filter);


	int iNewW = (iWidth - (filter.GetSize()-1))/2;
	int iNewH = (iHeight - (filter.GetSize()-1))/2;

	ZFloatImage* pScaleImg = new ZFloatImage(iNewW,iNewH,c);

	int iStart = filter.GetSize()/2;

	for(int j=0;j<iNewH;++j)
		for( int i=0;i<iNewW;++i){
			for(int k=0;k<c;++k){
				pScaleImg->at(i,j,k) = tmpImg.at(iStart + 2*i,iStart + 2*j,k);
			}
		}


		return pScaleImg;
}

ZByteImage*  CClosedFormMatting::_DownSmpImg(ZByteImage& img)
{
	int i,j;

	ZByteImage* pScaleImg = new ZByteImage;
	int iWidth = img.GetWidth();
	int iHeight = img.GetHeight();
	int c = img.GetChannel();
	int hW = iWidth/2;
	int hH = iHeight/2;


	pScaleImg->Create(iWidth,iHeight,c);
	for(int j=0;j<hH;++j)
		for(int i=0;i<hW;++i){
			for(int k=0;k<c;++k){
				pScaleImg->at(i,j,k) = img.at(i<<1,j<<1,k);
			}
		}

		return pScaleImg;
}

ZFloatImage* CClosedFormMatting::_UpSampleAlphaUsingImg(ZFloatImage& alphaImg,ZFloatImage& img, ZFloatImage& b_Img)
{
	ZFloatImage* coeff = _GetLinearCoeff(alphaImg,img);

	//Test
	ZFloatImage* testalpha = new ZFloatImage(img.GetWidth(),img.GetHeight());
	for(int j=0;j<testalpha->GetHeight();++j)
		for(int i=0;i<testalpha->GetWidth();++i){
			testalpha->at(i,j) = coeff->at(i,j,3);
			for(int k=0;k<3;++k)
				testalpha->at(i,j) += coeff->at(i,j,k) * img.at(i,j,k);
			testalpha->at(i,j) = min(1.0,max(0,testalpha->at(i,j)));
		}
		//CxImage image;
		//FloatToCxImg(*testalpha,image);
		//image.Save("c:\\test\\test-s-alpha.bmp",GetImageFileType(".bmp"));


		ZFloatImage* bcoeff = _UpSmpImg(coeff,b_Img.GetWidth(),b_Img.GetHeight(),1);
		ZFloatImage* balpha = new ZFloatImage(b_Img.GetWidth(),b_Img.GetHeight());

		for(int i=0;i<bcoeff->GetWidth();++i)
			for(int j=0;j<bcoeff->GetHeight();++j){
				balpha->at(i,j) = bcoeff->at(i,j,3);
				for(int k=0;k<3;++k)
					balpha->at(i,j) += bcoeff->at(i,j,k) * b_Img.at(i,j,k);
				balpha->at(i,j) = min(1.0,max(0,balpha->at(i,j)));
			}



			return balpha;
}

ZFloatImage* CClosedFormMatting::_GetLinearCoeff(ZFloatImage& alphaImg, ZFloatImage& img)
{
	//neb_size=(win_size*2+1)^2;
	//[h,w,c]=size(I);
	//n=h; m=w;
	//img_size=w*h;


	//indsM=reshape([1:img_size],h,w);

	//coeff=zeros(h,w,c+1);


	//for j=1+win_size:w-win_size
	//	for i=win_size+1:h-win_size

	//		win_inds=indsM(i-win_size:i+win_size,j-win_size:j+win_size);
	//win_inds=win_inds(:);
	//winI=I(i-win_size:i+win_size,j-win_size:j+win_size,:);
	//winI=reshape(winI,neb_size,c);


	//G=[[winI,ones(neb_size,1)];[eye(c)*epsilon^0.5,zeros(c,1)]];

	//tcoeff=inv(G'*G)*G'*[alpha(win_inds);zeros(c,1)];
	//coeff(i,j,:)=reshape(tcoeff,1,1,c+1);


	//end
	//	end  


	//	coeff(1:win_size,:,:)=repmat(coeff(win_size+1,:,:),win_size,1);
	//coeff(end-win_size+1:end,:,:)=repmat(coeff(end-win_size,:,:),win_size,1);
	//coeff(:,1:win_size,:)=repmat(coeff(:,win_size+1,:),1,win_size);
	//coeff(:,end-win_size+1:end,:)=repmat(coeff(:,end-win_size,:),1,win_size);

	int neb_size = (win_size*2+1)*(win_size*2+1);
	int w = img.GetWidth();
	int h = img.GetHeight();
	int c = img.GetChannel();

	ZIntImage indsM(w,h);
	ZFloatVector alphaV(w*h);

	int index=0;
	for(int j=0;j<h;++j)
		for(int i=0;i<w;++i){
			indsM.at(i,j) = index;
			alphaV.at(index) = alphaImg.at(i,j);
			++index;
		}

		//Test
		//CxImage image;
		//FloatToCxImg(alphaImg,image);
		//image.Save("c:\\test\\test-alpha.bmp",GetImageFileType(".bmp"));

		ZFloatImage* coeff = new ZFloatImage(w,h,c+1);
		ZIntVector win_inds;
		Wml::GMatrixd winI,G;

		win_inds.Create(neb_size);
		winI.SetSize(neb_size,c);
		G.SetSize(neb_size+c,c+1);


		for(int i=win_size;i<w-win_size;++i)
			for(int j=win_size;j<h-win_size;++j){

				index=0;
				for(int kj=j-win_size;kj<=j+win_size;++kj)
					for(int ki=i-win_size;ki<=i+win_size;++ki){
						for(int kk=0;kk<c;++kk)
							winI(index,kk) = img.at(ki,kj,kk);						
						win_inds.at(index) = indsM.at(ki,kj);
						++index;
					}
					for(int ki=0;ki<winI.GetRows();++ki){
						int kj;
						for( kj=0;kj<winI.GetColumns();++kj)
							G(ki,kj) = winI(ki,kj);
						G(ki,kj) = 1.0;
					}
					for(int ki=winI.GetRows();ki<winI.GetRows();++ki){
						for(int kj=0;kj<G.GetColumns();++kj)
							G(ki,kj) = 0.0;
						G(ki,ki-winI.GetRows()) = sqrt(epsilon);
					}
					Wml::GMatrixd GTGInv(c+1,c+1);
					Wml::LinearSystemd::Inverse(G.Transpose()*G,GTGInv);
					Wml::GMatrixd A(neb_size+c,1);

					for(int ki=0;ki<neb_size;++ki)
						A(ki,0) = alphaV.at(win_inds.at(ki));
					for(int ki=neb_size;ki<neb_size+c;++ki)
						A(ki,0) = 0.0;
					Wml::GMatrixd tcoeff = GTGInv*G.Transpose()*A;

					for(int ki=0;ki<c+1;++ki){
						coeff->at(i,j,ki) = tcoeff(ki,0);
						//printf("%f\t",coeff->at(i,j,ki));
					}
			}

			for(int i=0;i<win_size;++i)
				for(int j=0;j<h;++j)
					for(int k=0;k<c+1;++k)
						coeff->at(i,j,k) = coeff->at(win_size,j,k);

			for(int i=w-win_size;i<w;++i)
				for(int j=0;j<h;++j)
					for(int k=0;k<c+1;++k)
						coeff->at(i,j,k) = coeff->at(w-win_size-1,j,k);

			for(int j=0;j<win_size;++j)
				for(int i=0;i<w;++i)
					for(int k=0;k<c+1;++k)
						coeff->at(i,j,k) = coeff->at(i,win_size,k);

			for(int j=h-win_size;j<h;++j)
				for(int i=0;i<w;++i)
					for(int k=0;k<c+1;++k)
						coeff->at(i,j,k) = coeff->at(i,h-win_size-1,k);


			return coeff;
}

ZFloatImage* CClosedFormMatting::_UpSmpImg(ZFloatImage* img, int iNewW, int iNewH,int filterS)
{

	//id=floor((new_imgSize(1)-size(I,1)*2+1)/2);
	//iu=ceil((new_imgSize(1)-size(I,1)*2+1)/2);
	//jd=floor((new_imgSize(2)-size(I,2)*2+1)/2);
	//ju=ceil((new_imgSize(2)-size(I,2)*2+1)/2);

	//nI=zeros(new_imgSize(1)+2*filtS,new_imgSize(2)+2*filtS,size(I,3),size(I, ...
	//	4));
	//nI(id+filtS+1:2:end-iu-filtS,jd+filtS+1:2:end-ju-filtS,:,:)=I;
	//nI(id+filtS-1:-2:1,:,:,:)=repmat(nI(id+filtS+1,:,:,:),ceil((id+filtS-1)/2),1);

	//nI(end-iu-filtS+2:2:end,:,:,:)=repmat(nI(end-iu-filtS,:,:,:),ceil((iu+filtS-1)/2),1);
	//nI(:,jd+filtS-1:-2:1,:,:)=repmat(nI(:,jd+filtS+1,:,:),1,ceil((jd+filtS-1)/2));
	//nI(:,end-ju-filtS+2:2:end,:,:)=repmat(nI(:,end-ju-filtS,:,:),1,ceil((ju+filtS-1)/2));




	//for i=1:size(nI,3)
	//	for j=1:size(nI,4)
	//		nI(:,:,i,j)=conv2(filt,filt',nI(:,:,i,j),'same');
	//		end
	//		end
	int oldW = img->GetWidth();
	int oldH = img->GetHeight();
	int iChannel = img->GetChannel();

	int id = floor((iNewW-oldW*2+1)/2.0);
	int iu = ceil((iNewW-oldW*2+1)/2.0);
	int jd = floor((iNewH-oldH*2+1)/2.0);
	int ju = ceil((iNewH-oldH*2+1)/2.0);

	ZFloatImage* nImg = new ZFloatImage;
	nImg->CreateAndInit(iNewW+2*filterS,iNewH+2*filterS,iChannel);

	int iLeft = id+filterS;
	int iRight = nImg->GetWidth()-1-(iu+filterS);
	int iTop = jd+filterS;
	int iBottom = nImg->GetHeight()-1-(ju+filterS);

	for(int i=iLeft,ii=0; i<=iRight; i+=2,++ii)
		for(int j=iTop,jj=0; j<=iBottom; j+=2,++jj)
			for(int k=0;k<iChannel;++k){
				nImg->at(i,j,k) = img->at(ii,jj,k);
				//nImg->at(i+1,j,k) = img->at(ii,jj,k);
				//nImg->at(i,j+1,k) = img->at(ii,jj,k);
				//nImg->at(i+1,j+1,k) = img->at(ii,jj,k);
			}


			for(int i=iLeft-2; i>=0; i-=2)
				for(int j=0; j<=nImg->GetHeight()-1; ++j)
					for(int k=0;k<iChannel;++k)
						nImg->at(i,j,k) = nImg->at(iLeft,j,k);

			for(int i=iRight+2; i<=nImg->GetWidth()-1; i+=2)
				for(int j=0; j<=nImg->GetHeight()-1; ++j)
					for(int k=0;k<iChannel;++k)
						nImg->at(i,j,k) = nImg->at(iRight,j,k);

			for(int j=iTop-2; j>=0; j-=2)
				for(int i=0; i<=nImg->GetWidth()-1; ++i)
					for(int k=0;k<iChannel;++k)
						nImg->at(i,j,k) = nImg->at(i,iTop,k);

			for(int j=iBottom+2; j<=nImg->GetHeight()-1; j+=2)
				for(int i=0; i<=nImg->GetWidth()-1; ++i)
					for(int k=0;k<iChannel;++k)
						nImg->at(i,j,k) = nImg->at(i,iBottom,k);


			ZFloatVector filter;
			if(filterS==1){
				filter.Create(2*filterS+1);		
				filter.at(0) = 1.0/2.0;	filter.at(1) = 2.0/2.0;	filter.at(2) = 1.0/2.0;
			}
			else if(filterS==2){
				filter.Create(2*filterS+1);		
				filter.at(0) = 1.0/8.0;	filter.at(1) = 4.0/8.0;	filter.at(2) = 6.0/8.0;	filter.at(3) = 4.0/8.0;	filter.at(4) = 1.0/8.0;
			}
			else{
				filter.Create(1);
				filter.at(0) = 1.0;
			}

			_Conv2(*nImg,filter,filter);

			ZFloatImage* finalImg = new ZFloatImage(iNewW,iNewH,iChannel); 

			for(int j=0;j<iNewH;++j)
				for(int i=0;i<iNewW;++i)
					for(int k=0;k<iChannel;++k){
						finalImg->at(i,j,k) = nImg->at(i+filterS,j+filterS,k);
					}


					return finalImg;
}

void CClosedFormMatting::_imErode(ZByteImage& img)
{
	int iWidth = img.GetWidth();
	int iHeight = img.GetHeight();

	ZByteImage tmpImg;
	tmpImg.Create(iWidth,iHeight,1,img.GetMap());

	for(int j=0;j<iHeight;++j)
		for(int i=0;i<iWidth;++i){
			if(tmpImg.at(i,j)==0){
				for(int wi=i-1;wi<=i+1;++wi)
					for(int wj=j-1;wj<=j+1;++wj){
						if(wi>=0&&wi<iWidth&&wj>=0&&wj<iHeight)
							img.at(wi,wj) = 0;
					}
			}
		}

}

void CClosedFormMatting::_UpdateTrimap(ZByteImage& triMap,ZFloatImage& alphaImg)
{
	int iWidth = triMap.GetWidth();
	int iHeight = triMap.GetHeight();


	ZByteImage fgImg(iWidth,iHeight),bgImg(iWidth,iHeight);


	for(int j=0;j<iHeight;++j)
		for(int i=0;i<iWidth;++i){
			float v = alphaImg.at(i,j);
			if(v>=1-m_thr_alpha)
				fgImg.at(i,j)=1;
			else
				fgImg.at(i,j)=0;
			if(v<=m_thr_alpha)
				bgImg.at(i,j)=1;
			else
				bgImg.at(i,j)=0;
		}

		//erode
		_imErode(bgImg);
		_imErode(fgImg);

		for(int j=0;j<iHeight;++j)
			for(int i=0;i<iWidth;++i){
				if(bgImg.at(i,j))
					triMap.at(i,j)=REGION_BG;			
				if(fgImg.at(i,j))
					triMap.at(i,j)=REGION_FG;
			}

			//CxImage image;
			//ByteToCxImg(triMap,image);
			//image.Save("c:\\test\\test-trimap.bmp",GetImageFileType(".bmp"));

}
void CClosedFormMatting::SolveFB(ZFloatImage& srcImg, ZFloatImage& alpha, ZFloatImage& FImg, ZFloatImage& BImg)
{
	int iWidth = srcImg.GetWidth();
	int iHeight = srcImg.GetHeight();
	int iChannel = srcImg.GetChannel();
	ZFloatImage Galpha(iWidth,iHeight,2);
	ZByteImage consts_map(iWidth,iHeight);
	ZIntImage indsM(iWidth,iHeight,2);
	int iVariableCount = 0;

	for(int j=0;j<iHeight;++j)
		for(int i=0;i<iWidth;++i){
			if(alpha.at(i,j)<m_thr_alpha||alpha.at(i,j)>1.0-m_thr_alpha){
				consts_map.at(i,j) = 1;
			}
			else{
				consts_map.at(i,j) = 0;				
			}
		}

		ZByteImage consts_map2 = consts_map;
		_imErode(consts_map2);

		//Compute alpha gradient
		for(int j=0;j<=iHeight-1;++j){
			int i;
			for( i=0;i<=iWidth-2;++i){
				Galpha.at(i,j,0) = alpha.at(i+1,j) - alpha.at(i,j);			
			}
			Galpha.at(i,j,0) = Galpha.at(i-1,j,0);
		}
		for(int i=0;i<=iWidth-1;++i){
			int j;
			for( j=0;j<=iHeight-2;++j){
				Galpha.at(i,j,1) = alpha.at(i,j+1)-alpha.at(i,j);
			}
			Galpha.at(i,j,1) = Galpha.at(i,j-1,1);
		}


		//prepare
		iVariableCount = 0;
		for(int j=0;j<iHeight;++j)
			for(int i=0;i<iWidth;++i){
				iVariableCount += consts_map2.at(i,j);
				//printf("%d\n",consts_map2.at(i,j));
			}
			iVariableCount = iWidth*iHeight-iVariableCount;

			int index=0;
			for(int j=0;j<iHeight;++j)
				for(int i=0;i<iWidth;++i){
					if(consts_map2.at(i,j)==0){
						indsM.at(i,j,0) = index;
						indsM.at(i,j,1) = index+iVariableCount;
						++index;
					}
					else{				
						indsM.at(i,j,0) = -1;
						indsM.at(i,j,1) = -1;
					}
				}

				//Calculate Matrix Size!
				int iRowSize=0;
				for(int j=0;j<iHeight;++j)
					for(int i=0;i<iWidth;++i){
						if(consts_map.at(i,j)==0){
							if(i<iWidth-1)
								iRowSize += 2;
							if(j<iHeight-1)
								iRowSize += 2;

							iRowSize += 1;
						}
						if(consts_map2.at(i,j)==0)
							iRowSize += 2;

					}	



					Wml::GVectord b(iRowSize);
					FImg.Create(iWidth,iHeight,4);
					BImg.Create(iWidth,iHeight,4);

					for(int k=0;k<iChannel;++k){

						CSparseMatrix_ListType A(iRowSize,2*iVariableCount,0);
						int iEqIndex = 0;
						for(int j=0;j<iHeight;++j)
							for(int i=0;i<iWidth;++i){
								if(consts_map.at(i,j)!=0)
									continue;

								int indsf = indsM.at(i,j,0);
								int indsb = indsM.at(i,j,1);				

								A.SetElement(iEqIndex,indsf,alpha.at(i,j));
								A.SetElement(iEqIndex,indsb,1.0-alpha.at(i,j));
								b[iEqIndex] = srcImg.at(i,j,k);				
								++iEqIndex;
							}
							//Gradient:		
							for(int j=0;j<iHeight;++j)
								for(int i=0;i<iWidth;++i){
									if(consts_map.at(i,j)!=0)
										continue;

									int indsf = indsM.at(i,j,0);
									int indsb = indsM.at(i,j,1);

									float wgf = sqrt(fabs(Galpha.at(i,j,0))) + 0.003*(1.0-alpha.at(i,j));
									float wgb = sqrt(fabs(Galpha.at(i,j,1))) + 0.003*alpha.at(i,j);

									if(i<iWidth-1){
										int indsf1 = indsM.at(i+1,j,0);
										int indsb1 = indsM.at(i+1,j,1);					


										//F_ix					
										A.SetElement(iEqIndex,indsf1,wgf);
										A.SetElement(iEqIndex,indsf,-wgf);
										b[iEqIndex] = 0.0;
										++iEqIndex;

										//B_ix
										A.SetElement(iEqIndex,indsb1,wgb);
										A.SetElement(iEqIndex,indsb,-wgb);
										b[iEqIndex] = 0.0;
										++iEqIndex;
									}

									if(j<iHeight-1){
										int indsf2 = indsM.at(i,j+1,0);
										int indsb2 = indsM.at(i,j+1,1);

										//F_iy
										A.SetElement(iEqIndex,indsf2,wgf);
										A.SetElement(iEqIndex,indsf,-wgf);
										b[iEqIndex] = 0.0;
										++iEqIndex;				

										//B_iy
										A.SetElement(iEqIndex,indsb2,wgb);
										A.SetElement(iEqIndex,indsb,-wgb);
										b[iEqIndex] = 0.0;
										++iEqIndex;
									}
								}
								//Constraints:
								for(int j=0;j<iHeight;++j)
									for(int i=0;i<iWidth;++i){
										if(consts_map2.at(i,j)==0){
											float wf=1,wb=1;
											float alphav = alpha.at(i,j);					

											wf = (alphav>0.98)*100 + 0.03*alphav*(alphav>0.7)+0.01*(alphav<0.02);
											wb = (alphav<0.02)*100 + 0.03*(1.0-alphav)*(alphav<0.3)+0.01*(alphav>0.98);


											int indsf = indsM.at(i,j,0);
											int indsb = indsM.at(i,j,1);

											A.SetElement(iEqIndex,indsf,wf);
											b[iEqIndex] = wf*srcImg.at(i,j,k)*(alpha.at(i,j)>0.02);
											++iEqIndex;

											A.SetElement(iEqIndex,indsb,wb);
											b[iEqIndex] = wb*srcImg.at(i,j,k)*(alpha.at(i,j)<0.98);
											++iEqIndex;						
										}
									}


									CSparseMatrix_ListType ATA;

									A.ATA(ATA);

									//PrintSparseMatrix(&A);
									//PrintSparseMatrix(&ATA);
									//for(int ki=0;ki<2*iVariableCount;ki++)			
									//		printf("%f\n",ATA.GetElement(ki,ki));

									Wml::GVectord x,dir;		
									dir = A.AT_Multi_Vec(b);
									x.SetSize(dir.GetSize());
									//for(int ki=0;ki<b.GetSize();++ki)
									//	printf("%f\n",b[ki]);
									//for(int ki=0;ki<dir.GetSize();++ki)
									//	printf("%f\n",dir[ki]);

									taucs_ccs_matrix* mtx = ATA.ToTaucsMatrix();
									A.ClearData();
									ATA.ClearData();

									void* F = NULL;

									char* m_defFactor[]={"taucs.factor.LLT=true", NULL};


									if(taucs_linsolve(mtx,&F,1, x,dir,m_defFactor,NULL)!=TAUCS_SUCCESS){
										printf("Linear Matrix No Solution!\n");
									}
									taucs_ccs_free(mtx);
									taucs_linsolve(NULL,&F,0,NULL,NULL,NULL,NULL);



									for(int j=0;j<iHeight;++j)
										for(int i=0;i<iWidth;++i){
											if(consts_map2.at(i,j)==0){
												FImg.at(i,j,k) = max(0,min(1.0,x[indsM.at(i,j,0)]));
												BImg.at(i,j,k) = max(0,min(1.0,x[indsM.at(i,j,1)]));
												//FImg.at(i,j,k) = x[indsM.at(i,j,0)];
												//BImg.at(i,j,k) = x[indsM.at(i,j,1)];
											}
											else{
												if(alpha.at(i,j)>1.0-m_thr_alpha){
													FImg.at(i,j,k) = srcImg.at(i,j,k);
													BImg.at(i,j,k) = 0;
												}
												else{
													FImg.at(i,j,k) = 0;
													BImg.at(i,j,k) = srcImg.at(i,j,k);
												}
											}
										}


					}

					for(int j=0;j<iHeight;++j)
						for(int i=0;i<iWidth;++i){
							FImg.at(i,j,3) = alpha.at(i,j);
							BImg.at(i,j,3) = 1.0-alpha.at(i,j);
							//if(i==406&&j==135){
							//	printf("F:%f,%f,%f,%f\n",FImg.at(i,j,0),FImg.at(i,j,1),FImg.at(i,j,2),FImg.at(i,j,3));
							//	printf("B:%f,%f,%f,%f\n",BImg.at(i,j,0),BImg.at(i,j,1),BImg.at(i,j,2),BImg.at(i,j,3));
							//	printf("C:%f,%f,%f,%f\n",srcImg.at(i,j,0),srcImg.at(i,j,1),srcImg.at(i,j,2),alpha.at(i,j));
							//}
						}



						//CxImage testImg;
						//testImg.Create(iWidth,iHeight,24);

						//for(int i=0;i<iWidth;++i)
						//	for(int j=0;j<iHeight;++j){
						//		RGBQUAD color;
						//		color.rgbRed = color.rgbGreen = color.rgbBlue = 0;
						//		if(consts_map2.at(i,j)==0&&consts_map.at(i,j)==1){
						//			color.rgbRed = color.rgbGreen = color.rgbBlue = 255;				
						//		}
						//		testImg.SetPixelColor(i,iHeight-1-j,color);			
						//	}

						//	testImg.Save("c:\\test\\test-border.bmp",GetImageFileType(".bmp"));
}


void CClosedFormMatting::SolveAlpha_MultiFrames(FloatImgList& srcImgList,ByteImgList& triMapList,IntImgList& csMapList,FloatImgList& wMapList,
												FloatImgList& alphaImgList)
{
	printf("in solvealpha_multiFrames\n");
	printf("length: %d %d %d %d %d\n", srcImgList.size(), triMapList.size(), csMapList.size(), wMapList.size(), alphaImgList.size());

	//for (int i=0; i<srcImgList.size(); ++i)
	//{
	//	CxImage srcim, trim, wmim;
	//	FloatToCxImg(*srcImgList[0], srcim);
	//	FloatToCxImg(*wMapList[0], wmim);
	//	ByteToCxImg(*triMapList[0], trim);

	//	std::string path = "H:/test/";
	//	char num[10];
	//	_itoa(i, num, 10);
	//	std::string srcpath = path+std::string("src_")+std::string(num)+std::string(".png");
	//	srcim.Save(srcpath.c_str(), CXIMAGE_FORMAT_PNG);

	//	srcpath = path+std::string("tri_")+std::string(num)+std::string(".png");
	//	trim.Save(srcpath.c_str(), CXIMAGE_FORMAT_PNG);

	//	srcpath = path+std::string("wap_")+std::string(num)+std::string(".png");
	//	wmim.Save(srcpath.c_str(), CXIMAGE_FORMAT_PNG);
	//}
	int w = srcImgList[0]->GetWidth();
	int h = srcImgList[0]->GetHeight();
	int c = srcImgList[0]->GetChannel();	

	IntImgList indsMList;
	Wml::GVectord b;
	CSparseMatrix_ListType* A = _GetLaplacian_MultiFrames2(srcImgList,triMapList,csMapList,wMapList,b,indsMList);

	Wml::GVectord x;
	printf("\n  before resize: %d\n", b.GetSize());
	x.SetSize(b.GetSize());

	printf("before convert\n");
	taucs_ccs_matrix* mtx = A->ToTaucsMatrix();
	printf("after convert\n");

	delete A;


	void* F = NULL;

	char* m_defFactor[]={"taucs.factor.LLT=true", NULL};


	if(taucs_linsolve(mtx,&F,1, x,b,m_defFactor,NULL)!=TAUCS_SUCCESS){
		printf("Linear Matrix No Solution!\n");
	}
	taucs_ccs_free(mtx);
	taucs_linsolve(NULL,&F,0,NULL,NULL,NULL,NULL);


	alphaImgList.resize(srcImgList.size());

	for(int frameindex=0; frameindex<alphaImgList.size(); ++frameindex){
		ZFloatImage* pAlphaImg = new ZFloatImage(w,h);
		ZIntImage* pIndsM = indsMList[frameindex];
		ZByteImage* pTriMap = triMapList[frameindex];

		for(int j=0;j<h;++j){
			for(int i=0;i<w;++i){
				switch(pTriMap->at(i,j))
				{
				case REGION_BG:
				case REGION_BG_EDIT:
					pAlphaImg->at(i,j) = 0;
					break;
				case REGION_FG:
					pAlphaImg->at(i,j) = 1.0;
					break;
				default:
					pAlphaImg->at(i,j) = max(0,min(1,x[pIndsM->at(i,j)]));
					break;
				}						
			}		
		}
		alphaImgList[frameindex] = pAlphaImg;		
	}

	for(int i=0; i<indsMList.size(); ++i)
		delete indsMList[i];

printf("length: %d %d %d %d %d\n", srcImgList.size(), triMapList.size(), csMapList.size(), wMapList.size(), alphaImgList.size());
printf("out of solvealpha_multiframes\n");

}

void CClosedFormMatting::SolveAlpha_MultiFrames_Fix(FloatImgList& srcImgList,ByteImgList& triMapList,IntImgList& csMapList,FloatImgList& wMapList,
														 FloatImgList& alphaImgList, FloatImgList& fixAlphaImgList, std::vector<int>& fixImgList)
{
	printf("in solvealpha_multiFrames\n");
	printf("length: %d %d %d %d %d\n", srcImgList.size(), triMapList.size(), csMapList.size(), wMapList.size(), alphaImgList.size());
	for(int i=0;i<fixImgList.size();++i)
		printf("Anchor Frame%d\n",fixImgList[i]);

	
	int w = srcImgList[0]->GetWidth();
	int h = srcImgList[0]->GetHeight();
	int c = srcImgList[0]->GetChannel();	

	IntImgList indsMList;
	Wml::GVectord b;
	CSparseMatrix_ListType* A = _GetLaplacian_MultiFrames2(srcImgList,triMapList,csMapList,wMapList,b,indsMList);

	//Add soft constraint
	for(int i=0; i<fixAlphaImgList.size(); ++i){
		ZFloatImage* pFixAlphaImg = fixAlphaImgList[i];
		int iIndex = fixImgList[i];
		if(pFixAlphaImg){
			ZIntImage* pIndsM = indsMList[iIndex];
			for(int j=0;j<h;++j)
				for(int i=0;i<w;++i){
					if(pIndsM->at(i,j)>=0){
						int vIndex = pIndsM->at(i,j);
						float alphaValue = pFixAlphaImg->at(i,j);
						
						A->Element_Add(vIndex,vIndex,100);
						b[vIndex] += 100 * alphaValue;
					}
				}
		}
	}




	Wml::GVectord x;
	printf("\n  before resize: %d\n", b.GetSize());
	x.SetSize(b.GetSize());

	printf("before convert\n");
	taucs_ccs_matrix* mtx = A->ToTaucsMatrix();
	printf("after convert\n");

	delete A;


	void* F = NULL;

	char* m_defFactor[]={"taucs.factor.LLT=true", NULL};


	if(taucs_linsolve(mtx,&F,1, x,b,m_defFactor,NULL)!=TAUCS_SUCCESS){
		printf("Linear Matrix No Solution!\n");
	}
	taucs_ccs_free(mtx);
	taucs_linsolve(NULL,&F,0,NULL,NULL,NULL,NULL);


	alphaImgList.resize(srcImgList.size());

	for(int frameindex=0; frameindex<alphaImgList.size(); ++frameindex){
		ZFloatImage* pAlphaImg = new ZFloatImage(w,h);
		ZIntImage* pIndsM = indsMList[frameindex];
		ZByteImage* pTriMap = triMapList[frameindex];

		for(int j=0;j<h;++j){
			for(int i=0;i<w;++i){
				switch(pTriMap->at(i,j))
				{
				case REGION_BG:
				case REGION_BG_EDIT:
					pAlphaImg->at(i,j) = 0;
					break;
				case REGION_FG:
					pAlphaImg->at(i,j) = 1.0;
					break;
				default:
					pAlphaImg->at(i,j) = max(0,min(1,x[pIndsM->at(i,j)]));
					break;
				}						
			}		
		}
		alphaImgList[frameindex] = pAlphaImg;		
	}

	for(int i=0; i<indsMList.size(); ++i)
		delete indsMList[i];

	printf("length: %d %d %d %d %d\n", srcImgList.size(), triMapList.size(), csMapList.size(), wMapList.size(), alphaImgList.size());
	printf("out of solvealpha_multiframes\n");

}

void CClosedFormMatting::SolveAlpha_MultiFrames_SpatialSmooth(FloatImgList& srcImgList,ByteImgList& triMapList,IntImgList& csMapList,FloatImgList& wMapList,
							FloatImgList& alphaImgList,FloatImgList& spatialWList)
{
	double wtime;
	int w = srcImgList[0]->GetWidth();
	int h = srcImgList[0]->GetHeight();
	int c = srcImgList[0]->GetChannel();	

	IntImgList indsMList;
	Wml::GVectord b;
	CSparseMatrix_ListType* A = _GetLaplacian_MultiFrames2(srcImgList,triMapList,csMapList,wMapList,b,indsMList);

	//Add Spatial Term
	{
		CSparseMatrix_ListType*  pMtx = A;
		int nFrames = srcImgList.size();
		for(int frameindex=0; frameindex<nFrames; ++frameindex){
			ZFloatImage* pSrcImg = srcImgList[frameindex];
			ZIntImage* pIndsM = indsMList[frameindex];						
			ZByteImage* pTriMap = triMapList[frameindex];		
			ZFloatImage* pSpatialW = spatialWList[frameindex];

			for(int i=win_size;i<w-win_size;++i)
				for(int j=win_size;j<h-win_size;++j){

					int x,y;
					int inds_xi,inds_xj;
					inds_xi = pIndsM->at(i,j);
					float sw = 5.0;
					if(inds_xi<0)
						continue;
					//left
					//{
					//	inds_xj = pIndsM->at(i-1,j);
					//	if(inds_xj>=0){						
					//		pMtx->Element_Add(inds_xi,inds_xi,sw);
					//		pMtx->Element_Add(inds_xi,inds_xj,-sw);
					//		pMtx->Element_Add(inds_xj,inds_xi,-sw);
					//		pMtx->Element_Add(inds_xj,inds_xj,sw);
					//	}
					//	else{									
					//		switch(pTriMap->at(i-1,j))//Fix variable
					//		{
					//		case REGION_FG:
					//			pMtx->Element_Add(inds_xi,inds_xi,sw);
					//			b[inds_xi] += sw;
					//			break;
					//		case REGION_BG:
					//		case REGION_BG_EDIT:															
					//			pMtx->Element_Add(inds_xi,inds_xi,sw);									
					//			break;
					//		default:					
					//			break;
					//		}								
					//	}
					//}
					//right
					{
						sw = pSpatialW->at(i,j,0);
						inds_xj = pIndsM->at(i+1,j);
						if(inds_xj>=0){						
							pMtx->Element_Add(inds_xi,inds_xi,sw);
							pMtx->Element_Add(inds_xi,inds_xj,-sw);
							pMtx->Element_Add(inds_xj,inds_xi,-sw);
							pMtx->Element_Add(inds_xj,inds_xj,sw);
						}
						else{									
							switch(pTriMap->at(i+1,j))//Fix variable
							{
							case REGION_FG:
								pMtx->Element_Add(inds_xi,inds_xi,sw);
								b[inds_xi] += sw;
								break;
							case REGION_BG:
							case REGION_BG_EDIT:															
								pMtx->Element_Add(inds_xi,inds_xi,sw);									
								break;
							default:				
								break;
							}								
						}
					}
					//top
					//{
					//	inds_xj = pIndsM->at(i,j-1);
					//	if(inds_xj>=0){						
					//		pMtx->Element_Add(inds_xi,inds_xi,sw);
					//		pMtx->Element_Add(inds_xi,inds_xj,-sw);
					//		pMtx->Element_Add(inds_xj,inds_xi,-sw);
					//		pMtx->Element_Add(inds_xj,inds_xj,sw);
					//	}
					//	else{									
					//		switch(pTriMap->at(i,j-1))//Fix variable
					//		{
					//		case REGION_FG:
					//			pMtx->Element_Add(inds_xi,inds_xi,sw);
					//			b[inds_xi] += sw;
					//			break;
					//		case REGION_BG:
					//		case REGION_BG_EDIT:															
					//			pMtx->Element_Add(inds_xi,inds_xi,sw);									
					//			break;
					//		default:		
					//			break;
					//		}								
					//	}
					//}
					//bottom
					{
						sw = pSpatialW->at(i,j,1);
						inds_xj = pIndsM->at(i,j+1);
						if(inds_xj>=0){
							pMtx->Element_Add(inds_xi,inds_xi,sw);
							pMtx->Element_Add(inds_xi,inds_xj,-sw);
							pMtx->Element_Add(inds_xj,inds_xi,-sw);
							pMtx->Element_Add(inds_xj,inds_xj,sw);
						}
						else{									
							switch(pTriMap->at(i,j+1))//Fix variable
							{
							case REGION_FG:
								pMtx->Element_Add(inds_xi,inds_xi,sw);
								b[inds_xi] += sw;
								break;
							case REGION_BG:
							case REGION_BG_EDIT:															
								pMtx->Element_Add(inds_xi,inds_xi,sw);									
								break;
							default:		
								break;
							}								
						}
					}
				}
		}
	}
	Wml::GVectord x;
	x.SetSize(b.GetSize());	
	taucs_ccs_matrix* mtx = A->ToTaucsMatrix();

	delete A;
	printf("delete A time:%f\n",wtime);

	void* F = NULL;

	char* m_defFactor[]={"taucs.factor.LLT=true", NULL};


	if(taucs_linsolve(mtx,&F,1, x,b,m_defFactor,NULL)!=TAUCS_SUCCESS){
		printf("Linear Matrix No Solution!\n");
	}
	taucs_ccs_free(mtx);
	taucs_linsolve(NULL,&F,0,NULL,NULL,NULL,NULL);

	printf("Solve Time:%f\n",wtime);

	alphaImgList.resize(srcImgList.size());

	for(int frameindex=0; frameindex<alphaImgList.size(); ++frameindex){
		ZFloatImage* pAlphaImg = new ZFloatImage(w,h);
		ZIntImage* pIndsM = indsMList[frameindex];
		ZByteImage* pTriMap = triMapList[frameindex];

		for(int j=0;j<h;++j){
			for(int i=0;i<w;++i){
				switch(pTriMap->at(i,j))
				{
				case REGION_BG:
				case REGION_BG_EDIT:
					pAlphaImg->at(i,j) = 0;
					break;
				case REGION_FG:
					pAlphaImg->at(i,j) = 1.0;
					break;
				default:
					pAlphaImg->at(i,j) = max(0,min(1,x[pIndsM->at(i,j)]));
					break;
				}						
			}		
		}
		alphaImgList[frameindex] = pAlphaImg;		
	}

	for(int i=0; i<indsMList.size(); ++i)
		delete indsMList[i];
}

void CClosedFormMatting::SolveAlpha_MultiFrames_SoftConstraint(FloatImgList& srcImgList,ByteImgList& triMapList,IntImgList& csMapList,FloatImgList& wMapList,
										   FloatImgList& alphaImgList, ByteImgList& constMapList, FloatImgList& constWList, FloatImgList& spatialWList)
{
	double wtime;
	int w = srcImgList[0]->GetWidth();
	int h = srcImgList[0]->GetHeight();
	int c = srcImgList[0]->GetChannel();	

	IntImgList indsMList;
	Wml::GVectord b;
	CSparseMatrix_ListType* A = _GetLaplacian_MultiFrames2(srcImgList,triMapList,csMapList,wMapList,b,indsMList);

	//Add soft constraint!!!
	{
		int nFrames = constMapList.size();
		for(int frameindex=0; frameindex<nFrames; ++frameindex){
			ZIntImage* pIndsM = indsMList[frameindex];
			ZByteImage* pConstMap = constMapList[frameindex];
			ZFloatImage* pWMap = constWList[frameindex];
			for(int j=0;j<h;++j)
				for(int i=0;i<w;++i){
					if(pIndsM->at(i,j)>=0){
						int vIndex = pIndsM->at(i,j);
						float softW = pWMap->at(i,j);

						if(softW>0.001){
							A->Element_Add(vIndex,vIndex,softW);
							b[vIndex] += softW * pConstMap->at(i,j)/255.0;
						}						
					}
				}
		}
	}
	//Add Spatial Term
	{
		CSparseMatrix_ListType*  pMtx = A;
		int nFrames = srcImgList.size();
		for(int frameindex=0; frameindex<nFrames; ++frameindex){
			ZFloatImage* pSrcImg = srcImgList[frameindex];
			ZIntImage* pIndsM = indsMList[frameindex];						
			ZByteImage* pTriMap = triMapList[frameindex];		
			ZFloatImage* pSpatialW = spatialWList[frameindex];

			for(int i=win_size;i<w-win_size;++i)
				for(int j=win_size;j<h-win_size;++j){

					int x,y;
					int inds_xi,inds_xj;
					inds_xi = pIndsM->at(i,j);
					float sw = 5.0;
					if(inds_xi<0)
						continue;
					//left
					//{
					//	inds_xj = pIndsM->at(i-1,j);
					//	if(inds_xj>=0){						
					//		pMtx->Element_Add(inds_xi,inds_xi,sw);
					//		pMtx->Element_Add(inds_xi,inds_xj,-sw);
					//		pMtx->Element_Add(inds_xj,inds_xi,-sw);
					//		pMtx->Element_Add(inds_xj,inds_xj,sw);
					//	}
					//	else{									
					//		switch(pTriMap->at(i-1,j))//Fix variable
					//		{
					//		case REGION_FG:
					//			pMtx->Element_Add(inds_xi,inds_xi,sw);
					//			b[inds_xi] += sw;
					//			break;
					//		case REGION_BG:
					//		case REGION_BG_EDIT:															
					//			pMtx->Element_Add(inds_xi,inds_xi,sw);									
					//			break;
					//		default:					
					//			break;
					//		}								
					//	}
					//}
					//right
					{
						sw = pSpatialW->at(i,j,0);
						inds_xj = pIndsM->at(i+1,j);
						if(inds_xj>=0){						
							pMtx->Element_Add(inds_xi,inds_xi,sw);
							pMtx->Element_Add(inds_xi,inds_xj,-sw);
							pMtx->Element_Add(inds_xj,inds_xi,-sw);
							pMtx->Element_Add(inds_xj,inds_xj,sw);
						}
						else{									
							switch(pTriMap->at(i+1,j))//Fix variable
							{
							case REGION_FG:
								pMtx->Element_Add(inds_xi,inds_xi,sw);
								b[inds_xi] += sw;
								break;
							case REGION_BG:
							case REGION_BG_EDIT:															
								pMtx->Element_Add(inds_xi,inds_xi,sw);									
								break;
							default:				
								break;
							}								
						}
					}
					//top
					//{
					//	inds_xj = pIndsM->at(i,j-1);
					//	if(inds_xj>=0){						
					//		pMtx->Element_Add(inds_xi,inds_xi,sw);
					//		pMtx->Element_Add(inds_xi,inds_xj,-sw);
					//		pMtx->Element_Add(inds_xj,inds_xi,-sw);
					//		pMtx->Element_Add(inds_xj,inds_xj,sw);
					//	}
					//	else{									
					//		switch(pTriMap->at(i,j-1))//Fix variable
					//		{
					//		case REGION_FG:
					//			pMtx->Element_Add(inds_xi,inds_xi,sw);
					//			b[inds_xi] += sw;
					//			break;
					//		case REGION_BG:
					//		case REGION_BG_EDIT:															
					//			pMtx->Element_Add(inds_xi,inds_xi,sw);									
					//			break;
					//		default:		
					//			break;
					//		}								
					//	}
					//}
					//bottom
					{
						sw = pSpatialW->at(i,j,1);
						inds_xj = pIndsM->at(i,j+1);
						if(inds_xj>=0){
							pMtx->Element_Add(inds_xi,inds_xi,sw);
							pMtx->Element_Add(inds_xi,inds_xj,-sw);
							pMtx->Element_Add(inds_xj,inds_xi,-sw);
							pMtx->Element_Add(inds_xj,inds_xj,sw);
						}
						else{									
							switch(pTriMap->at(i,j+1))//Fix variable
							{
							case REGION_FG:
								pMtx->Element_Add(inds_xi,inds_xi,sw);
								b[inds_xi] += sw;
								break;
							case REGION_BG:
							case REGION_BG_EDIT:															
								pMtx->Element_Add(inds_xi,inds_xi,sw);									
								break;
							default:		
								break;
							}								
						}
					}
				}

		}
	}


	Wml::GVectord x;
	x.SetSize(b.GetSize());

	taucs_ccs_matrix* mtx = A->ToTaucsMatrix();


	delete A;
	printf("delete A time:%f\n",wtime);

	void* F = NULL;

	char* m_defFactor[]={"taucs.factor.LLT=true", NULL};


	if(taucs_linsolve(mtx,&F,1, x,b,m_defFactor,NULL)!=TAUCS_SUCCESS){
		printf("Linear Matrix No Solution!\n");
	}
	taucs_ccs_free(mtx);
	taucs_linsolve(NULL,&F,0,NULL,NULL,NULL,NULL);

	printf("Solve Time:%f\n",wtime);

	alphaImgList.resize(srcImgList.size());

	for(int frameindex=0; frameindex<alphaImgList.size(); ++frameindex){
		ZFloatImage* pAlphaImg = new ZFloatImage(w,h);
		ZIntImage* pIndsM = indsMList[frameindex];
		ZByteImage* pTriMap = triMapList[frameindex];

		for(int j=0;j<h;++j){
			for(int i=0;i<w;++i){
				switch(pTriMap->at(i,j))
				{
				case REGION_BG:
				case REGION_BG_EDIT:
					pAlphaImg->at(i,j) = 0;
					break;
				case REGION_FG:
					pAlphaImg->at(i,j) = 1.0;
					break;
				default:
					pAlphaImg->at(i,j) = max(0,min(1,x[pIndsM->at(i,j)]));
					break;
				}						
			}		
		}
		alphaImgList[frameindex] = pAlphaImg;
	}

	for(int i=0; i<indsMList.size(); ++i)
		delete indsMList[i];
}


void CClosedFormMatting::SolveAlphaWithBG_MultiFrames(FloatImgList& srcImgList,ByteImgList& triMapList,IntImgList& csMapList,FloatImgList& wMapList,
													  FloatImgList& bgInfoList,FloatImgList& alphaImgList)
{
	double wtime;
	int w = srcImgList[0]->GetWidth();
	int h = srcImgList[0]->GetHeight();
	int c = srcImgList[0]->GetChannel();	

	IntImgList indsMList;
	Wml::GVectord b;
	CSparseMatrix_ListType* A = _GetLaplacianWithBG_MultiFrames(srcImgList,triMapList,csMapList,wMapList,bgInfoList,b,indsMList);

	Wml::GVectord x;
	x.SetSize(b.GetSize());

	taucs_ccs_matrix* mtx = A->ToTaucsMatrix();


	delete A;
	printf("delete A time:%f\n",wtime);

	void* F = NULL;

	char* m_defFactor[]={"taucs.factor.LLT=true", NULL};


	if(taucs_linsolve(mtx,&F,1, x,b,m_defFactor,NULL)!=TAUCS_SUCCESS){
		printf("Linear Matrix No Solution!\n");
	}
	taucs_ccs_free(mtx);
	taucs_linsolve(NULL,&F,0,NULL,NULL,NULL,NULL);


	alphaImgList.resize(srcImgList.size());

	for(int frameindex=0; frameindex<alphaImgList.size(); ++frameindex){
		ZFloatImage* pAlphaImg = new ZFloatImage(w,h);
		ZIntImage* pIndsM = indsMList[frameindex];
		ZByteImage* pTriMap = triMapList[frameindex];

		for(int j=0;j<h;++j){
			for(int i=0;i<w;++i){
				switch(pTriMap->at(i,j))
				{
				case REGION_BG:
				case REGION_BG_EDIT:
					pAlphaImg->at(i,j) = 0;
					break;
				case REGION_FG:
					pAlphaImg->at(i,j) = 1.0;
					break;
				default:
					pAlphaImg->at(i,j) = max(0,min(1,x[pIndsM->at(i,j)]));
					break;
				}						
			}		
		}
		alphaImgList[frameindex] = pAlphaImg;
	}
}


CSparseMatrix_ListType* CClosedFormMatting::_GetLaplacian_MultiFrames2(FloatImgList& srcImgList, ByteImgList& triMapList, IntImgList& csMapList,FloatImgList& wMapList,
																	   Wml::GVectord& b, IntImgList& indsMList)
{
	int neb_size = (win_size*2+1)*(win_size*2+1); //spatial-temporal consistency
	int w = srcImgList[0]->GetWidth();
	int h = srcImgList[0]->GetHeight();
	int c = srcImgList[0]->GetChannel();
	int nFrames = (int)srcImgList.size();
	int variableCount = 0;

	ZIntVector win_inds;
	//ZIntImage indsM;
	Wml::GMatrixd winI,win_var,tvals;
	Wml::GMatrixd IdentityM;
	int index=0;

	std::vector<int> row_inds,col_inds;
	std::vector<double> vals;
	variableCount = _CalIndsM(triMapList,csMapList,indsMList);
	int tlen=0;
	for(int frameindex=0;frameindex<nFrames;++frameindex){
		ZIntImage* pIndsM = indsMList[frameindex];
		for(int j=0;j<h;++j)
			for(int i=0;i<w;++i){
				if(pIndsM->at(i,j,1)!=0)
					tlen++;
			}
	}
	tlen = tlen*neb_size*neb_size;
	win_inds.Create(neb_size);
	winI.SetSize(neb_size,c+1);		
	win_var.SetSize(c+1,c+1);
	tvals.SetSize(neb_size,neb_size);
	IdentityM.SetSize(c+1,c+1);		
	for(int i=0;i<IdentityM.GetRows();++i){
		for(int j=0;j<IdentityM.GetColumns();++j){
			if(i==j&&i<c)
				IdentityM(i,j) = 1;
			else
				IdentityM(i,j) = 0;
		}			
	}
	int len = 0;
	CSparseMatrix_ListType* pMtx = new CSparseMatrix_ListType(variableCount,variableCount);//pMtx 内存是否被释放了？ 用的章老师的库，看不到源码
	for(int frameindex=0; frameindex<nFrames; ++frameindex){
		ZFloatImage* pSrcImg = srcImgList[frameindex];
		ZIntImage* pIndsM = indsMList[frameindex];
		ZIntImage* pCsMap = csMapList[frameindex];
		ZFloatImage* pWMap = wMapList[frameindex];

		ZFloatImage* pPrevImg = NULL;
		ZFloatImage* pNextImg = NULL;
		ZIntImage* pPrevIndsM = NULL;
		ZIntImage* pNextIndsM = NULL;

		if(frameindex-1>=0){
			pPrevImg = srcImgList[frameindex-1];
			pPrevIndsM = indsMList[frameindex-1];
		}
		if(frameindex+1<nFrames){
			pNextImg = srcImgList[frameindex+1];
			pNextIndsM = indsMList[frameindex+1];
		}
		printf("ne: %d %d\n", win_size, neb_size);

		for(int i=win_size;i<w-win_size;++i)
			for(int j=win_size;j<h-win_size;++j){
				if(pIndsM->at(i,j,1)==0)
					continue;
				//Get win_inds & winI;
				index=0;				
				//Local Neighbor
				for(int kj=j-win_size;kj<=j+win_size;++kj)
					for(int ki=i-win_size;ki<=i+win_size;++ki){
						for(int kk=0;kk<c;++kk){
							winI(index,kk) = pSrcImg->at(ki,kj,kk);	
						}							
						winI(index,c) = 1.0;												
						win_inds.at(index) = pIndsM->at(ki,kj);
						if(pIndsM->at(ki,kj)<0)
							printf("%d\t",pIndsM->at(ki,kj));
						++index;
					}					
					Wml::GMatrixd tmpwin_var= (winI.Transpose() * winI) + epsilon * IdentityM;
					Wml::LinearSystemd::Inverse(tmpwin_var,win_var);
					tvals = winI*win_var*winI.Transpose();				
					for(int kj=0;kj<neb_size;++kj){
						for(int ki=0;ki<neb_size;++ki)
							//tvals(ki,kj) = W(ki,kj) - tvals(ki,kj)*W(kj,kj);				
							tvals(ki,kj) = -tvals(ki,kj);
						tvals(kj,kj) += 1.0;
					}
					//tvals = tvals.Transpose() * tvals;					

					for(int ki=0;ki<neb_size;++ki){
						for(int kj=0;kj<neb_size;++kj){
							pMtx->Element_Add(win_inds.at(ki),win_inds.at(kj),tvals(ki,kj));
						}				
					}

					int x,y;
					//Spatial					

					//Temporal
					
					//Backward:
					x = pCsMap->at(i,j,0);
					y = pCsMap->at(i,j,1);
					float sw = 1.0;
					if(x>=0){
						int inds_xi = pIndsM->at(i,j);
						int inds_xj = pPrevIndsM->at(x,y);
						if(inds_xi<0||inds_xj<0){
							printf("index error:%d,%d\n",inds_xi,inds_xj);
						}
						sw = pWMap->at(i,j)*0.001;	

						pMtx->Element_Add(inds_xi,inds_xi,sw);
						pMtx->Element_Add(inds_xi,inds_xj,-sw);
						pMtx->Element_Add(inds_xj,inds_xi,-sw);
						pMtx->Element_Add(inds_xj,inds_xj,sw);
					}
					//Forward:
					x = pCsMap->at(i,j,2);
					y = pCsMap->at(i,j,3);
					if(x>=0){
						int inds_xi = pIndsM->at(i,j);
						int inds_xj = pNextIndsM->at(x,y);
						if(inds_xi<0||inds_xj<0){
							printf("index error:%d,%d\n",inds_xi,inds_xj);
						}
						sw = pWMap->at(i,j)*0.001;
						pMtx->Element_Add(inds_xi,inds_xi,sw);
						pMtx->Element_Add(inds_xi,inds_xj,-sw);
						pMtx->Element_Add(inds_xj,inds_xi,-sw);
						pMtx->Element_Add(inds_xj,inds_xj,sw);
					}
			}
	}
	printf("variable count:%d\n",variableCount);
	printf("matrix unzero count:%d\n",pMtx->GetElementCount());

	b.SetSize(variableCount);

	double lambda=100;
	index=0;
	for(int frameindex=0; frameindex<nFrames; ++frameindex){
		ZIntImage* pIndsM = indsMList[frameindex];
		ZByteImage* pTriMap = triMapList[frameindex];
		for(int j=0;j<h;++j)
			for(int i=0;i<w;++i){
				if(pIndsM->at(i,j)>=0){
					int vIndex = pIndsM->at(i,j);
					switch(pTriMap->at(i,j))//Fix variable
					{
					case REGION_FG:
						pMtx->Element_Add(vIndex,vIndex,lambda);
						b[vIndex] = lambda;
						break;
					case REGION_BG:
					case REGION_BG_EDIT:															
						pMtx->Element_Add(vIndex,vIndex,lambda);	
						b[vIndex] = 0;
						break;
					default:
						b[vIndex] = 0;

					}		
				}
			}
	}

	//Add Spatial Smoothness Term
	//for(int frameindex=0; frameindex<nFrames; ++frameindex){
	//	ZFloatImage* pSrcImg = srcImgList[frameindex];
	//	ZIntImage* pIndsM = indsMList[frameindex];
	//	ZIntImage* pCsMap = csMapList[frameindex];
	//	ZFloatImage* pWMap = wMapList[frameindex];

	//	ZFloatImage* pPrevImg = NULL;
	//	ZFloatImage* pNextImg = NULL;
	//	ZIntImage* pPrevIndsM = NULL;
	//	ZIntImage* pNextIndsM = NULL;

	//	ZByteImage* pTriMap = triMapList[frameindex];

	//	if(frameindex-1>=0){
	//		pPrevImg = srcImgList[frameindex-1];
	//		pPrevIndsM = indsMList[frameindex-1];
	//	}
	//	if(frameindex+1<nFrames){
	//		pNextImg = srcImgList[frameindex+1];
	//		pNextIndsM = indsMList[frameindex+1];
	//	}

	//	for(int i=win_size;i<w-win_size;++i)
	//		for(int j=win_size;j<h-win_size;++j){

	//			int x,y;
	//			int inds_xi,inds_xj;
	//			inds_xi = pIndsM->at(i,j);
	//			float sw = 5.0;
	//			if(inds_xi<0)
	//				continue;
	//			//left
	//			{
	//				inds_xj = pIndsM->at(i-1,j);
	//				if(inds_xj>=0){						
	//					pMtx->Element_Add(inds_xi,inds_xi,sw);
	//					pMtx->Element_Add(inds_xi,inds_xj,-sw);
	//					pMtx->Element_Add(inds_xj,inds_xi,-sw);
	//					pMtx->Element_Add(inds_xj,inds_xj,sw);
	//				}
	//				else{									
	//					switch(pTriMap->at(i-1,j))//Fix variable
	//					{
	//					case REGION_FG:
	//						pMtx->Element_Add(inds_xi,inds_xi,sw);
	//						b[inds_xi] += sw;
	//						break;
	//					case REGION_BG:
	//					case REGION_BG_EDIT:															
	//						pMtx->Element_Add(inds_xi,inds_xi,sw);									
	//						break;
	//					default:					
	//						break;
	//					}								
	//				}
	//			}
	//			//right
	//			{
	//				inds_xj = pIndsM->at(i+1,j);
	//				if(inds_xj>=0){						
	//					pMtx->Element_Add(inds_xi,inds_xi,sw);
	//					pMtx->Element_Add(inds_xi,inds_xj,-sw);
	//					pMtx->Element_Add(inds_xj,inds_xi,-sw);
	//					pMtx->Element_Add(inds_xj,inds_xj,sw);
	//				}
	//				else{									
	//					switch(pTriMap->at(i+1,j))//Fix variable
	//					{
	//					case REGION_FG:
	//						pMtx->Element_Add(inds_xi,inds_xi,sw);
	//						b[inds_xi] += sw;
	//						break;
	//					case REGION_BG:
	//					case REGION_BG_EDIT:															
	//						pMtx->Element_Add(inds_xi,inds_xi,sw);									
	//						break;
	//					default:				
	//						break;
	//					}								
	//				}
	//			}
	//			//top
	//			{
	//				inds_xj = pIndsM->at(i,j-1);
	//				if(inds_xj>=0){						
	//					pMtx->Element_Add(inds_xi,inds_xi,sw);
	//					pMtx->Element_Add(inds_xi,inds_xj,-sw);
	//					pMtx->Element_Add(inds_xj,inds_xi,-sw);
	//					pMtx->Element_Add(inds_xj,inds_xj,sw);
	//				}
	//				else{									
	//					switch(pTriMap->at(i,j-1))//Fix variable
	//					{
	//					case REGION_FG:
	//						pMtx->Element_Add(inds_xi,inds_xi,sw);
	//						b[inds_xi] += sw;
	//						break;
	//					case REGION_BG:
	//					case REGION_BG_EDIT:															
	//						pMtx->Element_Add(inds_xi,inds_xi,sw);									
	//						break;
	//					default:		
	//						break;
	//					}								
	//				}
	//			}
	//			//bottom
	//			{
	//				inds_xj = pIndsM->at(i,j+1);
	//				if(inds_xj>=0){						
	//					pMtx->Element_Add(inds_xi,inds_xi,sw);
	//					pMtx->Element_Add(inds_xi,inds_xj,-sw);
	//					pMtx->Element_Add(inds_xj,inds_xi,-sw);
	//					pMtx->Element_Add(inds_xj,inds_xj,sw);
	//				}
	//				else{									
	//					switch(pTriMap->at(i,j+1))//Fix variable
	//					{
	//					case REGION_FG:
	//						pMtx->Element_Add(inds_xi,inds_xi,sw);
	//						b[inds_xi] += sw;
	//						break;
	//					case REGION_BG:
	//					case REGION_BG_EDIT:															
	//						pMtx->Element_Add(inds_xi,inds_xi,sw);									
	//						break;
	//					default:		
	//						break;
	//					}								
	//				}
	//			}
	//		}

	//}
	printf("ok");
	return pMtx;
}


CSparseMatrix_ListType* CClosedFormMatting::_GetLaplacianWithBG_MultiFrames(FloatImgList& srcImgList, ByteImgList& triMapList, IntImgList& csMapList,FloatImgList& wMapList,
																			FloatImgList& bgInfoList, Wml::GVectord& b, IntImgList& indsMList)
{
	int neb_size = (win_size*2+1)*(win_size*2+1); //spatial-temporal consistency
	int w = srcImgList[0]->GetWidth();
	int h = srcImgList[0]->GetHeight();
	int c = srcImgList[0]->GetChannel();
	int nFrames = (int)srcImgList.size();
	int variableCount = 0;


	ZIntVector win_inds;
	//ZIntImage indsM;
	Wml::GMatrixd winI,win_var,tvals;
	Wml::GMatrixd IdentityM;
	int index=0;


	std::vector<int> row_inds,col_inds;
	std::vector<double> vals;

	variableCount = _CalIndsM(triMapList,csMapList,indsMList);


	int tlen=0;
	for(int frameindex=0;frameindex<nFrames;++frameindex){
		ZIntImage* pIndsM = indsMList[frameindex];
		for(int j=0;j<h;++j)
			for(int i=0;i<w;++i){
				if(pIndsM->at(i,j,1)!=0)
					tlen++;
			}
	}
	tlen = tlen*neb_size*neb_size;



	win_inds.Create(neb_size);
	winI.SetSize(neb_size,c+1);		
	win_var.SetSize(c+1,c+1);
	tvals.SetSize(neb_size,neb_size);
	IdentityM.SetSize(c+1,c+1);		
	for(int i=0;i<IdentityM.GetRows();++i){
		for(int j=0;j<IdentityM.GetColumns();++j){
			if(i==j&&i<c)
				IdentityM(i,j) = 1;
			else
				IdentityM(i,j) = 0;
		}			
	}

	int len = 0;

	CSparseMatrix_ListType* pMtx = new CSparseMatrix_ListType(variableCount,variableCount);

	for(int frameindex=0; frameindex<nFrames; ++frameindex){
		ZFloatImage* pSrcImg = srcImgList[frameindex];
		ZIntImage* pIndsM = indsMList[frameindex];
		ZIntImage* pCsMap = csMapList[frameindex];
		ZFloatImage* pWMap = wMapList[frameindex];

		ZFloatImage* pPrevImg = NULL;
		ZFloatImage* pNextImg = NULL;
		ZIntImage* pPrevIndsM = NULL;
		ZIntImage* pNextIndsM = NULL;


		if(frameindex-1>=0){
			pPrevImg = srcImgList[frameindex-1];
			pPrevIndsM = indsMList[frameindex-1];
		}
		if(frameindex+1<nFrames){
			pNextImg = srcImgList[frameindex+1];
			pNextIndsM = indsMList[frameindex+1];
		}

		for(int i=win_size;i<w-win_size;++i)
			for(int j=win_size;j<h-win_size;++j){
				if(pIndsM->at(i,j,1)==0)
					continue;
				//Get win_inds & winI;
				index=0;				

				//Local Neighbor
				for(int kj=j-win_size;kj<=j+win_size;++kj)
					for(int ki=i-win_size;ki<=i+win_size;++ki){
						for(int kk=0;kk<c;++kk){
							winI(index,kk) = pSrcImg->at(ki,kj,kk);	
						}							
						winI(index,c) = 1.0;												
						win_inds.at(index) = pIndsM->at(ki,kj);
						if(pIndsM->at(ki,kj)<0)
							printf("%d\t",pIndsM->at(ki,kj));
						++index;
					}					

					Wml::GMatrixd tmpwin_var= (winI.Transpose() * winI) + epsilon * IdentityM;
					Wml::LinearSystemd::Inverse(tmpwin_var,win_var);
					tvals = winI*win_var*winI.Transpose();				
					for(int kj=0;kj<neb_size;++kj){
						for(int ki=0;ki<neb_size;++ki)
							//tvals(ki,kj) = W(ki,kj) - tvals(ki,kj)*W(kj,kj);				
							tvals(ki,kj) = -tvals(ki,kj);
						tvals(kj,kj) += 1.0;
					}
					//tvals = tvals.Transpose() * tvals;					

					for(int ki=0;ki<neb_size;++ki){
						for(int kj=0;kj<neb_size;++kj){
							pMtx->Element_Add(win_inds.at(ki),win_inds.at(kj),tvals(ki,kj));
						}				
					}

					//Spatial-Temporal
					int x,y;
					//Backward:
					x = pCsMap->at(i,j,0);
					y = pCsMap->at(i,j,1);
					float sw = 0.1;
					if(x>=0){
						int inds_xi = pIndsM->at(i,j);
						int inds_xj = pPrevIndsM->at(x,y);
						if(inds_xi<0||inds_xj<0){
							printf("index error:%d,%d\n",inds_xi,inds_xj);
						}
						sw = pWMap->at(i,j);

						pMtx->Element_Add(inds_xi,inds_xi,sw);
						pMtx->Element_Add(inds_xi,inds_xj,-sw);
						pMtx->Element_Add(inds_xj,inds_xi,-sw);
						pMtx->Element_Add(inds_xj,inds_xj,sw);
					}
					//Forward:
					x = pCsMap->at(i,j,2);
					y = pCsMap->at(i,j,3);
					if(x>=0){
						int inds_xi = pIndsM->at(i,j);
						int inds_xj = pNextIndsM->at(x,y);
						if(inds_xi<0||inds_xj<0){
							printf("index error:%d,%d\n",inds_xi,inds_xj);
						}
						sw = pWMap->at(i,j);
						pMtx->Element_Add(inds_xi,inds_xi,sw);
						pMtx->Element_Add(inds_xi,inds_xj,-sw);
						pMtx->Element_Add(inds_xj,inds_xi,-sw);
						pMtx->Element_Add(inds_xj,inds_xj,sw);
					}					
			}
	}


	printf("variable count:%d\n",variableCount);
	printf("matrix unzero count:%d\n",pMtx->GetElementCount());


	b.SetSize(variableCount);			

	double lambda=100;
	index=0;


	for(int frameindex=0; frameindex<nFrames; ++frameindex){
		ZIntImage* pIndsM = indsMList[frameindex];
		ZByteImage* pTriMap = triMapList[frameindex];
		ZFloatImage* pSrcImg = srcImgList[frameindex];
		ZFloatImage* pBgImg = bgInfoList[frameindex];

		for(int j=0;j<h;++j)
			for(int i=0;i<w;++i){
				if(pIndsM->at(i,j)>=0){
					int vIndex = pIndsM->at(i,j);
					switch(pTriMap->at(i,j))//Fix variable
					{
					case REGION_FG:
						pMtx->Element_Add(vIndex,vIndex,lambda);
						b[vIndex] = lambda;
						break;
					case REGION_BG:
					case REGION_BG_EDIT:															
						pMtx->Element_Add(vIndex,vIndex,lambda);	
						b[vIndex] = 0;
						break;
					default:
						b[vIndex] = 0;
						int local_win = 2;
						if(i>local_win&&i<w-local_win&&j>local_win&&j<h-local_win&&pBgImg->at(i,j,3)>0.5){
							double colordist = (pSrcImg->at(i,j,0) - pBgImg->at(i,j,0))*(pSrcImg->at(i,j,0) - pBgImg->at(i,j,0)) + (pSrcImg->at(i,j,1) - pBgImg->at(i,j,1))*(pSrcImg->at(i,j,1) - pBgImg->at(i,j,1))
								+ (pSrcImg->at(i,j,2) - pBgImg->at(i,j,2))*(pSrcImg->at(i,j,2) - pBgImg->at(i,j,2));									
							colordist = 0.1*exp(-colordist/(2.0*0.02*0.02))*pBgImg->at(i,j,3);							
							pMtx->Element_Add(pIndsM->at(i,j),pIndsM->at(i,j),colordist);
						}
					}		
				}
			}
	}
	printf("ok");
	return pMtx;
}

int CClosedFormMatting::_CalIndsM(ByteImgList& triMapList,IntImgList& csMapList,IntImgList& indsMList)
{
	int w = triMapList[0]->GetWidth();
	int h = triMapList[0]->GetHeight();
	int vairableCount=0;
	int nFrames = (int)triMapList.size();

	indsMList.resize(nFrames);
	//need imerode
	vairableCount = 0;
	for(int frameindex=0; frameindex<nFrames; ++frameindex){
		ZByteImage constsMap,o_constsMap, constsMap2;
		_imErode(*triMapList[frameindex],o_constsMap,constsMap);
		constsMap2 = constsMap;
		_imErode(constsMap2);

		indsMList[frameindex] = new ZIntImage(w,h,2);
		for(int j=0;j<h;++j)
			for(int i=0;i<w;++i){
				if(constsMap2.at(i,j)==0){
					indsMList[frameindex]->at(i,j) = vairableCount++;
				}
				else
					indsMList[frameindex]->at(i,j) = -1;

				//for equation only!
				if(constsMap.at(i,j)==0)
					indsMList[frameindex]->at(i,j,1) = 1;
				else
					indsMList[frameindex]->at(i,j,1) = 0;
			}
	}

	for(int frameindex=0; frameindex<nFrames; ++frameindex){

		int iCurrNo=0;
		ZIntImage* pIndsM = indsMList[frameindex];
		ZIntImage* pCsMap = csMapList[frameindex];

		//Backward:
		iCurrNo = frameindex-1;			

		if(iCurrNo>=0){
			ZIntImage* pPrevIndsM = indsMList[iCurrNo];	
			for(int j=0;j<h;++j)
				for(int i=0;i<w;++i){
					if(pIndsM->at(i,j,1)==1){
						int x = pCsMap->at(i,j,0);
						int y = pCsMap->at(i,j,1);
						if(x>=0&&y>=0&&pPrevIndsM->at(x,y)==-1)
							pPrevIndsM->at(x,y) = vairableCount++;
					}
				}
		}
		//Forward:
		iCurrNo = frameindex+1;
		if(iCurrNo<nFrames){
			ZIntImage* pNextIndsM = indsMList[iCurrNo];		
			for(int j=0;j<h;++j)
				for(int i=0;i<w;++i){
					if(pIndsM->at(i,j,1)==1){
						int x = pCsMap->at(i,j,2);
						int y = pCsMap->at(i,j,3);
						if(x>=0&&y>=0&&pNextIndsM->at(x,y)==-1){
							pNextIndsM->at(x,y) = vairableCount++;
							//printf("i:%d,j:%d,x:%d,y:%d\t",i,j,x,y);
						}
					}
				}
		}
	}
	return vairableCount;
}
