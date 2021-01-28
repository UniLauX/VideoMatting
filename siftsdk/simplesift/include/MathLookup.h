#ifndef __MATHLOOKUP__H
#define __MATHLOOKUP__H
#include "math/basic/wmlmath/include/wmlgmatrix.h"

#define LU_PI	3.14159265358979323f
class AtanLookupF32
{
public:
	AtanLookupF32(){
		Init();
	}
	~AtanLookupF32(){
		delete []m_dATAN_LU[0];
		delete []m_dATAN_LU[1];
		delete []m_dATAN_LU[2];
		delete []m_dATAN_LU[3];
	}

	void Init();

	static __forceinline float ValueDirect(int y, int x, int idx)
	{
		//y ++;
		//x ++;
		
		x = x * 2 + y + 3;

		return m_dATAN_LU[x][idx];
	}

	static inline float Value(float y,float x){
		if( x > 0.0 ){
			if( y > 0.0 )
				return  m_dATAN_LU[0][(int)(N_DOUBLE * y / ( x + y ))];
			else
				return -m_dATAN_LU[0][(int)(N_DOUBLE * (-y) / ( x - y ))];
		}

		if( x == 0.0 ){
			if( y > 0 )
				return  LU_PI/2;
			else
				return  -LU_PI/2;
		}

		if( y < 0.0 )
			return  m_dATAN_LU[0][(int)(N_DOUBLE * y / ( x + y ))] - LU_PI;
		else
			return -m_dATAN_LU[0][(int)(N_DOUBLE * (-y) / ( x - y ))] + LU_PI;
	}

	static float NDOUBLE(){return N_DOUBLE;}
		
private:
	static float* m_dATAN_LU[4];
	//static float* m_dATAN_LUxy;
	//static float* m_dATAN_LUx;
	//static float* m_dATAN_LUy;
	

	Wml::GMatrixf m_dATAN_Direct;

	static int	N;
	static float N_DOUBLE;
};

class ExpLookUpF32
{
public:
	ExpLookUpF32(){
		Init();
	}
	~ExpLookUpF32(){
		delete m_dEXP_LU;
	}

	void Init();

	static inline float Value(float x){

		return m_dEXP_LU[(int)(x*N_MULTIPLE)];
	}
		
public:
	static float* m_dEXP_LU;

	static float	MAX_THRESHOLD;
	static int		N;
	static float	N_MULTIPLE;
};

#endif