#pragma once

#include <vector>
#include "ZImage.h"
#include "WmlMathLib.h"

#include "ZMattingInterface.h"
#define REGION_BG	0
#define REGION_BG_EDIT	1
#define REGION_UNINIT	2
#define REGION_UNKNOWN	128
#define	REGION_FG		255

class CSparseMatrix;
class CSparseMatrix_ListType;

namespace MattingAlgorithm{

	class CClosedFormMatting : public ZMattingInterface
	{
	public:
		CClosedFormMatting(void);
	public:
		~CClosedFormMatting(void);

	public:
		//Input & output Interface: if not available, fill 0
		virtual bool ImageSolve(ZFloatImage* pSrcImg, /*Source Input Image*/
			ZByteImage* pTriMap,/*Input Trimap*/
			ZFloatImage* pBgPrior, /*Input Bg Prior: (r,g,b,w)*/
			ZFloatImage* pDataCost, /*Input Data cost Prior: (bg,fg,weight)*/
			ZFloatImage* pSmoothCost, /*Input Smoothness Prior: (EAST,SOUTH,WEST,NORTH,weight)*/	

			ZByteImage* pAlpha, /*Output Alpha*/
			ZFloatImage* pBgImg, /*Output Bg Image*/
			ZFloatImage* pFgImg /*Output Fg Image*/
			);

		//Input & output Interface: if not available, fill 0
		virtual bool VideoSolve(FloatImgList* pSrcImg, /*Source Input Images*/
			ByteImgList* pTriMap,/*Input Trimaps*/
			FloatImgList* pBgPrior, /*Input Bg Priors: (r,g,b,w)*/
			FloatImgList* pDataCost, /*Input Data cost Priors: (bg,fg,weight)*/
			FloatImgList* pSmoothCost, /*Input Smoothness Priors: (EAST,SOUTH,WEST,NORTH,weight)*/	
			IntImgList* pTMaps, /*Input termproal prior: (x1,y1,x2,y2)*/
			bool bStaticCamera, /*Camera is static or not*/

			ByteImgList* pAlpha, /*Output Alpha Imgs*/
			FloatImgList* pBgImg, /*Output Bg Images*/
			FloatImgList* pFgImg /*Output Fg Images*/
			);


	public:
		void Test();
		void Test2();
		ZFloatImage* RunMatting(ZFloatImage& srcImg, ZByteImage& triMap);
		bool RunMatting(ZFloatImage& srcImg,ZByteImage& triMap,ZFloatImage& bgInfo);

		ZFloatImage* SolveAlphaC2F(ZFloatImage& img,ZByteImage& triMap,int levels_num, int active_levels_num);
		ZFloatImage* SolveAlpha(ZFloatImage& srcImg, ZByteImage& triMap);
		ZFloatImage* SolveAlpha_WithBG(ZFloatImage& srcImg,ZByteImage& triMap,ZFloatImage& bgInfo);

		ZFloatImage* SolveAlpha_Generic(ZFloatImage& srcImg, ZByteImage& triMap, ZFloatImage* pDataCost, ZFloatImage* pSmoothCost);

		 void SolveFB(ZFloatImage& srcImg, ZFloatImage& alpha, ZFloatImage& F, ZFloatImage& B);
		
		CSparseMatrix_ListType* _GetLaplacian(ZFloatImage& srcImg, ZByteImage& triMap, Wml::GVectord& b, ZIntImage& indsM);
		CSparseMatrix_ListType* _GetLaplacian_WithBG(ZFloatImage& srcImg, ZByteImage& triMap, ZFloatImage& bgImg, Wml::GVectord& b, ZIntImage& indsM);
		
		CSparseMatrix_ListType* _CreateSparseMatrix(std::vector<int>& row_inds, std::vector<int>& col_inds, std::vector<double>& vals, int nRows,int nCols);

		void _imErode(ZByteImage& triMap, ZByteImage& o_constsMap, ZByteImage& constsMap);
		void _imErode(ZByteImage& img);
		void _Conv2(ZFloatImage& img, ZFloatVector& filterW, ZFloatVector& filterH);
		ZByteImage* _DownSmpTrimap(ZByteImage& triMap);

		void _CalTvals(Wml::GMatrixd& tvals,const Wml::GMatrixd& winI, const Wml::GMatrixd& win_var);
		void _CalWinVar(Wml::GMatrixd& win_var, const Wml::GMatrixd& winI, const Wml::GMatrixd& win_mu);

		ZFloatImage* _DownSmpImg(ZFloatImage& img);
		ZByteImage*  _DownSmpImg(ZByteImage& img);
		ZFloatImage* _UpSampleAlphaUsingImg(ZFloatImage& alphaImg,ZFloatImage& img, ZFloatImage& b_img);
		ZFloatImage* _GetLinearCoeff(ZFloatImage& alphaImg, ZFloatImage& img);
		ZFloatImage* _UpSmpImg(ZFloatImage* coeff, int iNewW, int iNewH, int filterS);

		void _UpdateTrimap(ZByteImage& triMap,ZFloatImage& alphaImg);

	public:
		void SolveAlpha_MultiFrames(FloatImgList& srcImgList,ByteImgList& triMapList,IntImgList& csMapList,FloatImgList& wMapList,
			FloatImgList& alphaImgList);

		void SolveAlpha_MultiFrames_Fix(FloatImgList& srcImgList,ByteImgList& triMapList,IntImgList& csMapList,FloatImgList& wMapList,
			FloatImgList& alphaImgList, FloatImgList& fixAlphaImgList, std::vector<int>& fixImgList);

		void SolveAlpha_MultiFrames_SpatialSmooth(FloatImgList& srcImgList,ByteImgList& triMapList,IntImgList& csMapList,FloatImgList& wMapList,
			FloatImgList& alphaImgList,FloatImgList& spatialWList);

		void SolveAlpha_MultiFrames_SoftConstraint(FloatImgList& srcImgList,ByteImgList& triMapList,IntImgList& csMapList,FloatImgList& wMapList,
			FloatImgList& alphaImgList, ByteImgList& constMapList, FloatImgList& constWList,FloatImgList& spatialWList);

		void SolveAlphaWithBG_MultiFrames(FloatImgList& srcImgList,ByteImgList& triMapList,IntImgList& csMapList,FloatImgList& wMapList,
			FloatImgList& bgInfoList,FloatImgList& alphaImgList);

		CSparseMatrix_ListType* _GetLaplacian_MultiFrames2(FloatImgList& srcImgList, ByteImgList& triMapList, IntImgList& csMapList,FloatImgList& wMapList,
			Wml::GVectord& b, IntImgList& indsMList);
		CSparseMatrix_ListType* _GetLaplacianWithBG_MultiFrames(FloatImgList& srcImgList, ByteImgList& triMapList, IntImgList& csMapList,FloatImgList& wMapList,
			FloatImgList& bgInfoList, Wml::GVectord& b, IntImgList& indsMList);

		int _CalIndsM(ByteImgList& triMapList,IntImgList& csMapList,IntImgList& indsMList);

	private:
		double m_thr_alpha;
		double epsilon;

		int win_size;
	};


}