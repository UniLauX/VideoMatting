#include "StdAfx.h"
#include ".\flowconstructor.h"
#include "global.h"
#include "CxImageFun.h"
#include "imageIO.h"
#include <cmath>
#include <assert.h>
#include <cstdio>

FlowConstructor::FlowConstructor(void):
l1(10.0), l2(1.0), s1(10.0 / sqrt(2.0)), s2(1.0/ sqrt(2.0)),
s1_end(10.0 / sqrt(2.0)), s2_end(0.05 / sqrt(2.0)), 
iters(5), filter(0), skip(15),
by(1),
om(1.95)
{
}

FlowConstructor::~FlowConstructor(void)
{
}

void FlowConstructor::fix_flow(Wml::GMatrixd &flow, double max_flow) 
{
	for(int i = 0; i < flow.GetRows(); i ++)
		for(int j = 0; j < flow.GetColumns(); j ++) {
			flow[i][j] = max(min(flow[i][j], max_flow), -max_flow);
		}
}

void FlowConstructor::sor_1_iter(const Wml::GMatrixd &grad, const Wml::GMatrixd &err, Wml::GMatrixd &dflow, const Wml::GMatrixd &flow, 
								 const Wml::GMatrixd &scale, double s1, double s2)
{
	int ny = err.GetRows(), nx = err.GetColumns();

	// Update white blocks
	for(int i = 1; i < ny - 1; i ++)
		for(int j = 1 + (i % 2); j < nx - 1; j += 2) {
			double ts2 = 2 * s2 * s2;
			double new_flow = flow[i][j] + dflow[i][j];
			dflow[i][j] -= (om * scale[i][j]) * (
				(l1 * grad[i][j] * psi1(err[i][j], s1)) +
				(l2 * (psi2(new_flow - flow[i - 1][j] - dflow[i - 1][j], ts2) +
				psi2(new_flow - flow[i][j - 1] - dflow[i][j - 1], ts2) +
				psi2(new_flow - flow[i][j + 1] - dflow[i][j + 1], ts2) +
				psi2(new_flow - flow[i + 1][j] - dflow[i + 1][j], ts2)))
				);
		}

		// Update black blocks
		for(int i = 1; i < ny - 1; i ++)
			for(int j = 2 + (i % 2); j < nx - 1; j += 2) {
				double ts2 = 2 * s2 * s2;
				double new_flow = flow[i][j] + dflow[i][j];
				dflow[i][j] -= (om * scale[i][j]) * (
					(l1 * grad[i][j] * psi1(err[i][j], s1)) +
					(l2 * (psi2(new_flow - flow[i - 1][j] - dflow[i - 1][j], ts2) +
					psi2(new_flow - flow[i][j - 1] - dflow[i][j - 1], ts2) +
					psi2(new_flow - flow[i][j + 1] - dflow[i][j + 1], ts2) +
					psi2(new_flow - flow[i + 1][j] - dflow[i + 1][j], ts2)))
					);
			}
}

void FlowConstructor::inten_const(Wml::GMatrixd &err, const Wml::GMatrixd &Ix, const Wml::GMatrixd &Iy, const Wml::GMatrixd &It, const Wml::GMatrixd &u, const Wml::GMatrixd &v)
{
	assert(err.GetRows() == u.GetRows() && err.GetColumns() == u.GetColumns());
	assert(err.GetRows() == v.GetRows() && err.GetColumns() == v.GetColumns());

	for(int i = 0; i < err.GetRows(); i ++)
		for(int j = 0; j < err.GetColumns(); j ++) {
			err[i][j] = Ix[i][j] * u[i][j] + Iy[i][j] * v[i][j] + It[i][j];
		}
}

inline void FlowConstructor::add_image(Wml::GMatrixd &image, const Wml::GMatrixd &add)
{
//	assert(image.GetRows() == add.GetRows() && image.GetColumns() == add.GetColumns());
	image += add;
}

void FlowConstructor::horn_derives(const Wml::GMatrixd &image1, const Wml::GMatrixd &image2, Wml::GMatrixd &dx, Wml::GMatrixd &dy, Wml::GMatrixd &dt)
{
	int ny = image1.GetRows(), nx = image1.GetColumns();
	assert(image2.GetRows() == ny && image2.GetColumns() == nx);

	for(int i = 0; i < ny - 1; i ++)
		for(int j = 0; j < nx - 1; j ++) {
			dy[i][j] = (image1[i + 1][j] + image2[i + 1][j] + image1[i + 1][j + 1] + image2[i + 1][j + 1] -
				image1[i][j] - image2[i][j] - image1[i][j + 1] - image2[i][j + 1]) / 4.0;

			dx[i][j] = (image1[i][j + 1] + image2[i][j + 1] + image1[i + 1][j + 1] + image2[i + 1][j + 1] -
				image1[i][j] - image2[i][j] - image1[i + 1][j] - image2[i + 1][j]) / 4.0;

			dt[i][j] = (image2[i][j] + image2[i][j + 1] + image2[i + 1][j] + image2[i + 1][j + 1] -
				image1[i][j] - image1[i][j + 1] - image1[i + 1][j] - image1[i + 1][j + 1]) / 4.0;
		}

	for(int i = 0; i < ny; i ++) {
		dx[i][nx - 1] = dy[i][nx - 1] = dt[i][nx - 1] = 0.0;
	}
	for(int j = 0; j < nx; j ++) {
		int index = (ny - 1) * nx + j;
		dx[ny - 1][j] = dy[ny - 1][j] = dt[ny - 1][j] = 0.0;
	}
}

void FlowConstructor::sor_scale(Wml::GMatrixd &scale, const Wml::GMatrixd &grad, double s1, double s2)
{
	int ny = scale.GetRows(), nx = scale.GetColumns();

	for(int i = 0; i < ny; i ++)
		for(int j = 0; j < nx; j ++) {
			scale[i][j] = 1.0 / (l1 * grad[i][j] * grad[i][j] / s1 / s1 + 4.0 * l2 / s2 / s2);
		}
}

void FlowConstructor::sor(const Wml::GMatrixd &image1, const Wml::GMatrixd &image2,
						  Wml::GMatrixd &Ix, Wml::GMatrixd &Iy, Wml::GMatrixd &It, Wml::GMatrixd &err,
						  Wml::GMatrixd &u, Wml::GMatrixd &v, Wml::GMatrixd &u_scale, Wml::GMatrixd &v_scale,
						  double s1, double s2)
{
	Wml::GMatrixd du(u.GetRows(), u.GetColumns()), dv(v.GetRows(), v.GetColumns());
	for(int i = 0; i < u.GetRows(); i ++) for(int j = 0; j < u.GetColumns(); j ++) du[i][j] = dv[i][j] = 0.0;
	horn_derives(image1, image2, Ix, Iy, It);

	sor_scale(u_scale, Ix, s1, s2);
	sor_scale(v_scale, Iy, s1, s2);

	for(int i = 0; i < iters; i ++) {
		inten_const(err, Ix, Iy, It, du, dv);
		sor_1_iter(Ix, err, du, u, u_scale, s1, s2);
		inten_const(err, Ix, Iy, It, du, dv);
		sor_1_iter(Iy, err, dv, v, v_scale, s1, s2);
	}

	add_image(u, du);
	add_image(v, dv);
}

void FlowConstructor::horn_dt(const Wml::GMatrixd &image1, const Wml::GMatrixd &image2, Wml::GMatrixd &dt)
{
	int ny = image1.GetRows(), nx = image1.GetColumns();
	assert(image2.GetRows() == ny && image2.GetColumns() == nx);
	assert(dt.GetRows() == ny && dt.GetColumns() == nx);

	for(int i = 0; i < ny - 1; i ++)
		for(int j = 0; j < nx - 1; j ++) {
			dt[i][j] = (image2[i][j] + image2[i][j + 1] + image2[i + 1][j] + image2[i + 1][j + 1]
			- image1[i][j] - image1[i][j + 1] - image1[i + 1][j] - image1[i + 1][j + 1]) / 4.0;
		}
	for(int i = 0; i < ny; i ++)
		dt[i][nx - 1] = 0.0;
	for(int j = 0; j < nx; j ++)
		dt[ny - 1][j] = 0.0;
}

void FlowConstructor::data_discont(const Wml::GMatrixd &image1, const Wml::GMatrixd &image2, const Wml::GMatrixd &u, const Wml::GMatrixd &v, 
								   double s1, Wml::GMatrixd &wrap, Wml::GMatrixd &It, Wml::GMatrixd &discont)
{
	int ny = image1.GetRows(), nx = image1.GetColumns();
	wrap_image(wrap, image1, u, v);
	horn_dt(image1, image2, It);

	for(int i = 1; i < ny - 1; i ++)
		for(int j = 1; j < nx - 1; j ++) {
			double t1 = s1 * sqrt(2.0);
			if (fabs(It[i][j]) >= t1)
				discont[i][j] = 0.0;
			else discont[i][j] = 1.0;
		}
	for(int i = 0; i < ny; i ++) {
		discont[i][0] = 0.0;
		discont[i][nx - 1] = 0.0;
	}
	for(int j = 0; j < nx; j ++) {
		discont[0][j] = 0.0;
		discont[ny - 1][j] = 0.0;
	}
}

void FlowConstructor::spatial_discont(const Wml::GMatrixd &u, const Wml::GMatrixd &v, double s2, Wml::GMatrixd &discont)
{
	int ny = u.GetRows(), nx = u.GetColumns();
	for(int i = 1; i < ny - 1; i ++)
		for(int j = 1; j < nx + 1; j ++) {
			double t2 = s2 * sqrt(2.0);
			if (fabs(u[i][j] - u[i][j - 1]) >= t2 || fabs(u[i][j] - u[i - 1][j]) >= t2 ||
				fabs(v[i][j] - v[i][j - 1]) >= t2 || fabs(v[i][j] - v[i - 1][j]) >= t2)
				discont[i][j] = 0.0;
			else discont[i][j] = 1.0;
		}
	for(int i = 0; i < ny; i ++) {
		discont[i][0] = discont[i][nx - 1] = 0.0;
	}
	for(int j = 0; j < nx; j ++) {
		discont[0][j] = discont[ny - 1][j] = 0.0;
	}
}
/*
void FlowConstructor::save_pgm(CxImage &image, const double *data, int nx, int ny)
{
	image.Create(nx, ny, 24, GetImageFileType("test.pgm"));
	double min_value = data[0], max_value = data[0];
	for(int i = 0; i < ny; i ++)
		for(int j = 0; j < nx; j ++) {
			int index = i * nx + j;
			min_value = min(min_value, data[index]);
			max_value = max(max_value, data[index]);
		}
	
	double scale;
	if (max_value - min_value < 1e-9) scale = 1.0; else scale = 255.0 / (max_value - min_value);

	int length = nx * 3;
	if (length % 4 != 0) length = (length / 4 + 1) * 4;
	for(int i = 0; i < ny; i ++)
		for(int j = 0; j < nx; j ++) {
			int index = (ny - 1 - i) * nx + j;
			double value = min(255.0, max(0.0, floor((data[index] - min_value) * scale + 0.5)));
//			(image.GetBits())[index] = (unsigned char)value;
			(image.GetBits())[length * i + j * 3] = (image.GetBits())[length * i + j * 3 + 1] = (image.GetBits())[length * i + j * 3 + 2] = (unsigned char)value;
		}
}
*/
void FlowConstructor::getFlow(CxImage &image1, CxImage &image2, int stages, int min_level, int max_lever,  
							  CxImage *image_u, CxImage *image_v, CxImage *image_data, CxImage *image_spatial, CxImage *image_stable)
{
	int nx = image1.GetWidth(), ny = image1.GetHeight();
	double s1_factor = pow((double)s1_end / s1, 1.0 / stages), s2_factor = pow((double)s2_end / s2, 1.0 / stages);
	Wml::GMatrixd u(ny, nx), v(ny, nx), err(ny, nx), Ix(ny, nx), Iy(ny, nx), It(ny, nx), prev(ny, nx), curr(ny, nx), outliers(ny, nx), discont(ny, nx);

	load_pgm(image1, prev);
	load_pgm(image2, curr);

	for(int i = 0; i < ny; i ++)
		for(int j = 0; j < nx; j ++) {
			u[i][j] = 0.0;
			v[i][j] = 0.0;
			outliers[i][j] = 0.0;
			discont[i][j] = 0.0;
			Ix[i][j] = 0.0;
			Iy[i][j] = 0.0;
			It[i][j] = 0.0;
			err[i][j] = 0.0;
		}

	for(int i = 0; i < stages; i ++) {
		pyramid_sor(prev, curr, s1, s2, min_level, max_lever, Ix, Iy, It, u, v, err);

		save_pgm(image_u[i], u);
		save_pgm(image_v[i], v);

		data_discont(prev, curr, u, v, s1, err, It, outliers);
		save_pgm(image_data[i], outliers);

		spatial_discont(u, v, s2, discont);
		save_pgm(image_spatial[i], discont);

		wrap_image(err, prev, u, v);
		save_pgm(image_stable[i], err);

		s1 = max(s1_end, s1 * s1_factor);
		s2 = max(s2_end, s2 * s2_factor);
	}
}

void FlowConstructor::pyramid_sor(const Wml::GMatrixd &image1, const Wml::GMatrixd &image2, double s1, double s2, int min_level, int max_level, 
								  Wml::GMatrixd &Ix, Wml::GMatrixd &Iy, Wml::GMatrixd &It, Wml::GMatrixd &u, Wml::GMatrixd &v, Wml::GMatrixd &err)
{
	int ny = image1.GetRows(), nx = image1.GetColumns();
	Wml::GMatrixd u_scale(ny, nx), v_scale(ny, nx);

	Wml::GMatrixd lap1(ny, nx), lap2(ny, nx), wrap(ny, nx);

	for(int i = 0; i < ny; i ++)
		for(int j = 0; j < nx; j ++){
			u_scale[i][j] = 0.0;
			v_scale[i][j] = 0.0;
			lap1[i][j] = 0.0;
			lap2[i][j] = 0.0;
			wrap[i][j] = 0.0;
		}

	if (max_level <= min_level) {
		wrap_image(wrap, image1, u, v);
		if (filter == 1) {
			laplacian(lap1, wrap);
			laplacian(lap2, image2);
		} else {
			copy_image(lap1, wrap);
			copy_image(lap2, image2);
		}

		sor(image1, image2, Ix, Iy, It, err, u, v, u_scale, v_scale, s1, s2);

	} else {
		int snx = nx / 2, sny = ny / 2;
		Wml::GMatrixd p_im1(sny, snx), p_im2(sny, snx), p_u(sny, snx), p_v(sny, snx), p_err(sny, snx), p2_u(ny, nx), p2_v(ny, nx);

		for(int i = 0; i < sny; i ++)
			for(int j = 0; j < snx; j ++) {
				p_im1[i][j] = 0.0;
				p_im2[i][j] = 0.0;
				p_u[i][j] = 0.0;
				p_v[i][j] = 0.0;
				p_err[i][j] = 0.0;
			}

		reduce_image(p_im1, image1);
		reduce_image(p_im2, image2);
		reduce_flow(p_u, u);
		reduce_flow(p_v, v);

		pyramid_sor(p_im1, p_im2, s1, s2 / 2.0, min_level, max_level - 1, Ix, Iy, It, p_u, p_v, p_err);

		project_image(p2_u, p_u);
		project_image(p2_v, p_v);

		for(int i = 0; i < ny; i ++)
			for(int j = 0; j < nx; j ++) {
				if (fabs(u[i][j] - p2_u[i][j]) >= 0.5 || fabs(v[i][j] - p2_v[i][j]) >= 0.5) {
					u[i][j] = p2_u[i][j];
					v[i][j] = p2_v[i][j];
				}
			}

		wrap_image(wrap, image1, u, v);
		if (filter == 1) {
			laplacian(lap1, wrap);
			laplacian(lap2, image2);
		} else {
			copy_image(lap1, wrap);
			copy_image(lap2, image2);
		}

		sor(lap1, lap2, Ix, Iy, It, err, u, v, u_scale, v_scale, s1, s2);
	}
}

void FlowConstructor::getFlow(const Wml::GMatrixd &image1, const Wml::GMatrixd &image2, int stages, int min_level, int max_lever, 
							  Wml::GMatrixd &u, Wml::GMatrixd &v, Wml::GMatrixd &data, Wml::GMatrixd &spatial, Wml::GMatrixd &stable) 
{
	int ny = image1.GetRows(), nx = image1.GetColumns();
	double s1_factor = pow((double)s1_end / s1, 1.0 / stages), s2_factor = pow((double)s2_end / s2, 1.0 / stages);
	double ss1 = s1, ss2 = s2;
	assert(image2.GetRows() == ny && image2.GetColumns() == nx);
	Wml::GMatrixd Ix(ny, nx), Iy(ny, nx), It(ny, nx);

	u.SetSize(ny, nx);
	v.SetSize(ny, nx);
	data.SetSize(ny, nx);
	spatial.SetSize(ny, nx);
	stable.SetSize(ny, nx);

	for(int i = 0; i < ny; i ++) for(int j = 0; j < nx; j ++) {
		v[i][j] = u[i][j] = data[i][j] = spatial[i][j] = stable[i][j] = 0.0;
		Ix[i][j] = Iy[i][j] = It[i][j] = 0.0;
	}

	if (ss1 < s1_end) ss1 = s1_end;
	if (ss2 < s2_end) ss2 = s2_end;

	for(int i = 0; i < stages; i ++) {
		pyramid_sor(image1, image2, ss1, ss2, min_level, max_lever, Ix, Iy, It, u, v, stable);

		data_discont(image1, image2, u, v, ss1, stable, It, data);

		spatial_discont(u, v, ss2, spatial);

		wrap_image(stable, image1, u, v);

		ss1 = max(s1_end, ss1 * s1_factor);
		ss2 = max(s2_end, ss2 * s2_factor);
	}

}
