#pragma once
#include "Vision/Image/cximage/include/cximage.h"

#include "cmatrix"
#include <iostream>
#include <fstream>
#include <vector>
#include "../grab/cluster.h"
using namespace std;

#define maxab(a, b)  (((a) > (b)) ? (a) : (b)) 
#define minab(a, b)  (((a) < (b)) ? (a) : (b)) 
#define MINSAMPLE 300
#define  CLUSTERTIME 2
#define SD 20
using namespace std;
using techsoft::mslice;
typedef techsoft::matrix<float>        fMatrix;
typedef std::valarray<float>           fVector;
typedef techsoft::matrix<double>       dMatrix;
typedef std::valarray<double>          dVector;
typedef techsoft::matrix<long double>  ldMatrix;
typedef std::valarray<long double>     ldVector;
typedef std::complex<float>            fComplex;
typedef techsoft::matrix<fComplex>     cfMatrix;
typedef std::valarray<fComplex>        cfVector;
typedef std::complex<double>           dComplex;
typedef techsoft::matrix<dComplex>     cdMatrix;
typedef std::valarray<dComplex>        cdVector;
typedef std::complex<long double>      ldComplex;
typedef techsoft::matrix<ldComplex>    cldMatrix;
typedef std::valarray<ldComplex>       cldVector;
//typedef struct node3
//{
//	dMatrix colorinfo;
//	long indexX;
//	long indexY;
//} colorNode;
typedef struct fnode
{
	dMatrix avgf;
	dMatrix cmatrixf;

}FNode;
typedef struct bnode
{
	dMatrix avgb;
	dMatrix cmatrixb;
}BNode;
class ProImage
{
public:
	ProImage(void);
	ProImage(string sname,string  triname,string desname);
	void init(CxImage si,CxImage ti);
	~ProImage(void);
	void getImageName();
	bool isValidName();
	void mattingImage();
	void process(long i,long j);
	void save();
	void test();
private:
	double getAvgAlpha(long x,long y);
	double getGi(long sx,long sy,long dx,long dy);
	void getlrMat(dMatrix &lmat,dMatrix &rmat,const FNode& fm,const BNode &bm ,double avgalpha,const dMatrix centercol);
	double getalpha(dMatrix f,dMatrix b,dMatrix c);
	void cluster(vector<  vector < colorNode > >& vec);
	dMatrix calR(const vector<colorNode>& vec);
	dMatrix calM(const vector<colorNode>& vec);
	int calN(const vector<colorNode>& vec);
	dMatrix calRC(const dMatrix r,const dMatrix m,const int n);//º∆À„Rc
	void forMatrix(const vector< vector <colorNode>  >& forvec,vector<FNode>& fmvec,long x,long y);
	void backMatrix(const vector< vector <colorNode> >& backvec,vector<BNode>& bmvec,long x,long y);
	bool isvalid(long i,long j);
	void addsample(long i,long j,vector<colorNode> &f,vector<colorNode>& b);
	void refine();
public:
	string SImageName;
	string TriImageName;
	string DesImageName;
	CxImage simage;//colorimage
	CxImage timage;//trimap
	CxImage dimage;
	CxImage backImage;
	CxImage  forImage;
	CxImage alphaImage;
	long width;
	long  height;
	double **alpha;//[1000][1000];
	int **tag;//[1000][1000];

};
