#pragma once

#include "WmlMathLib.h"
//#include "../lib/WmlLib_Load.h"
#include "Vision/Image/cximage/include/cximage.h"

class FlowConstructor
{
	double l1, l2;	//lambda
	double s1, s2;	//sigma
	double s1_end, s2_end;
	int iters, filter, skip;
	int by;
	double om;

	inline int sgn(double x) {
		if (x < -1e-9) return -1;
		else if (x > 1e-9) return 1;
		else return 0;
	}
	inline double psi1(double x, double sigma) {
		return 2.0 * x / (x * x + 2.0 * sigma * sigma);
	}
	inline double psi2(double x, double sigma) {
		return 2.0 * x / (x * x + sigma);
	}

	void fix_flow(Wml::GMatrixd &flow, double max_flow);
	void add_image(Wml::GMatrixd &image, const Wml::GMatrixd &add);
	void inten_const(Wml::GMatrixd &err, const Wml::GMatrixd &Ix, const Wml::GMatrixd &Iy, const Wml::GMatrixd &It, const Wml::GMatrixd &u, const Wml::GMatrixd &v);
	void horn_derives(const Wml::GMatrixd &image1, const Wml::GMatrixd &image2, Wml::GMatrixd &dx, Wml::GMatrixd &dy, Wml::GMatrixd &dt);
	void sor_scale(Wml::GMatrixd &scale, const Wml::GMatrixd &grad, double s1, double s2);

	void sor_1_iter(const Wml::GMatrixd &grad, const Wml::GMatrixd &err, Wml::GMatrixd &dflow, const Wml::GMatrixd &flow, 
		const Wml::GMatrixd &scale, double s1, double s2);

	void sor(const Wml::GMatrixd &image1, const Wml::GMatrixd &image2,
		Wml::GMatrixd &Ix, Wml::GMatrixd &Iy, Wml::GMatrixd &It, Wml::GMatrixd &err,
		Wml::GMatrixd &u, Wml::GMatrixd &v, Wml::GMatrixd &u_scale, Wml::GMatrixd &v_scale,
		double s1, double s2
		);

	void pyramid_sor(const Wml::GMatrixd &image1, const Wml::GMatrixd &image2, double s1, double s2, int min_level, int max_level, 
		Wml::GMatrixd &Ix, Wml::GMatrixd &Iy, Wml::GMatrixd &It, Wml::GMatrixd &u, Wml::GMatrixd &v, Wml::GMatrixd &err);

	void horn_dt(const Wml::GMatrixd &image1, const Wml::GMatrixd &image2, Wml::GMatrixd &dt);
	void data_discont(const Wml::GMatrixd &image1, const Wml::GMatrixd &image2, const Wml::GMatrixd &u, const Wml::GMatrixd &v, 
		double s1, Wml::GMatrixd &wrap, Wml::GMatrixd &It, Wml::GMatrixd &discont);
	void spatial_discont(const Wml::GMatrixd &u, const Wml::GMatrixd &v, double s2, Wml::GMatrixd &discont);

	//void save_pgm(CxImage &image, const double *data, int nx, int ny);
public:
	FlowConstructor(void);
	~FlowConstructor(void);

	inline int getIters() { return iters; }
	inline void setIters(int val) { iters = val; }

	void setS1(double ds1) { s1 = ds1; }
	void setS2(double ds2) { s2 = ds2; }

	void getFlow(const Wml::GMatrixd &image1, const Wml::GMatrixd &image2, int stages, int min_level, int max_lever,
		Wml::GMatrixd &u, Wml::GMatrixd &v, Wml::GMatrixd &data, Wml::GMatrixd &spatial, Wml::GMatrixd &stable);
	void getFlow(CxImage &image1, CxImage &image2, int stages, int min_level, int max_lever, 
		CxImage *u, CxImage *v, CxImage *data, CxImage *spatial, CxImage *stable);
};
