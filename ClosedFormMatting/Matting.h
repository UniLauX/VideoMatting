#ifndef MATTING_H
#define MATTING_H

#include "Define.h"
#include "../include/Wml/WmlMathLib.h"
#include "../include/Wml/WmlPolynomial1.h"
#include "../include/CxImage/ximage.h"

extern "C" {
#include "../include/taucs/taucs.h"	
}

class Matting
{

	CxImage image;
	CxImage scrbImage;
	Wml::GMatrix<REAL> constMap;
	Wml::GMatrix<REAL> constVals;

	int winSize;                   //ÖÐÐÄµ½±ßÔµ¾àÀë
	REAL epsilon;
	int lambda;
	int level;
	int activeLevel;
	REAL thrAlpha;
	int erodeWinSize;

	void Init();

	void SetKnownVals();
	void InitLaplacian(CxImage &image, taucs_ccs_matrix &L);
	void GetLaplacian(CxImage &image, taucs_ccs_matrix &L);
	void DeleteLaplacian(taucs_ccs_matrix &L);

	void Conv2(Wml::Polynomial1<REAL> &filt, int filtS, CxImage &img);
	void Conv2(Wml::Polynomial1<REAL> &filt, int filtS, Wml::GMatrix<REAL> &mat);
	void RoundMat(Wml::GMatrix<REAL> &mat);

	void GetLinearCoeff(REAL *alpha, 
						CxImage &image, 
						Wml::GMatrix<REAL> &A_R,
						Wml::GMatrix<REAL> &A_G,
						Wml::GMatrix<REAL> &A_B,
						Wml::GMatrix<REAL> &B);

public:

	Matting();
	~Matting();

	void SetImage(char *imageName, char *scrbImageName);
	void SetImage(CxImage *image, CxImage *scrbImage);

	REAL *SolveAlpha(CxImage &image,
					 Wml::GMatrix<REAL> &known, 
					 Wml::GMatrix<REAL> &knownVals);

	REAL *SolveAlphaC2F(CxImage &image, 
						Wml::GMatrix<REAL> &constMap, 
						Wml::GMatrix<REAL> &constVals, 
						int level,
						int activeLevelNum);

	CxImage DownSmpIm(CxImage &img, int filtS);

	void DownSmpIm(Wml::GMatrix<REAL> &mat, 
				   int filtS, 
				   Wml::GMatrix<REAL> &res);

	CxImage UpSmpIm(CxImage &img, int newX, int newY, int filtS);

	void UpSmpIm(Wml::GMatrix<REAL> &mat, 
				 int newX, 
				 int newY, 
				 int filtS, 
				 Wml::GMatrix<REAL> &res);

	REAL *UpSampleAlphaUsingImg(REAL *alpha, 
								CxImage &I, 
								CxImage &bI, 
								int filtS);

	void UpDateConstMap(Wml::GMatrix<REAL> &constMap, 
						REAL *alpha, REAL thrAlpha, int erodeWin);

	REAL *DoMatting();

	void SetWinSize(int size);
	void SetLevelNum(int l);
	void SetActiveLevelNum(int l);
	void SetThrAlpha(REAL alpha);
	void SetEpsilon(REAL e);
	void SetErodeWinSize(int size);

	int GetWinSize();
	int GetLevelNum();
	int GetActiveLevelNum();
	double GetThrAlpha();
	double GetEpsilon();

     void GetResult(CxImage &image);

	////////////////
	void Test(CxImage &image, int newX, int newY, char *name);
	void TestSolveAlpha(char *name);
	void TestMatValues(Wml::GMatrix<REAL> &mat, char *name);
	void TestMatValues(Wml::GMatrix<REAL> &mat);
	void TestAlpha(REAL *alpha, int w, int h, char *name);
	void TestLinearCoeff(CxImage &image, CxImage &alpha);
	void TestErode(CxImage &image, char *name);
	void TestErodeAlpha(CxImage &image, char *name);
};


#endif