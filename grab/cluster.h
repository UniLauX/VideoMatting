#ifndef GRAB_CLUSTER
#define GRAB_CLUSTER

#include "../include/Matrix Pro/cmatrix"
#include <vector>
using namespace std;
#include "../VideoEditing/stdafx.h"
/************************************************************************/
/* member for color cluster                                                           */
/************************************************************************/
//#define  CLUSTERTIME 4

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

typedef struct Cnode
{
	dMatrix colorinfo;
	long indexX;
	long indexY;
} colorNode;

typedef struct pixel
{
	int x;
	int y;
}pixelNode;


dMatrix calM(const vector<colorNode> &vec);
int calN(const std::vector<colorNode> &vec);
dMatrix calR(const std::vector<colorNode> &vec);
void cluster(vector<vector<colorNode >> & vec,int CLUSTERTIME=4);
dMatrix calRC(const dMatrix r, const dMatrix m, const int n);
/************************************************************************/
/*                   end of  member for color cluster                              */
/************************************************************************/
#endif