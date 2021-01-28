#include "StdAfx.h"
#include "global.h"
#include <algorithm>

void derivation(CxImage &img, Wml::GMatrixd &dx, Wml::GMatrixd &dy)
{
	int nx = img.GetWidth(), ny = img.GetHeight();
	dx.SetSize(ny, nx);
	dy.SetSize(ny, nx);
	for(int i = 0; i < ny; i ++) for(int j = 0; j < nx; j ++)
	{
		double ax = 0.0, ay = 0.0;
		RGBQUAD cur = img.GetPixelColor(j, i);
		if (i > 0)
		{
			RGBQUAD up = img.GetPixelColor(j, i - 1);
			ay += ((int)cur.rgbRed - up.rgbRed) * ((int)cur.rgbRed - up.rgbRed);
			ay += ((int)cur.rgbGreen - up.rgbGreen) * ((int)cur.rgbGreen - up.rgbGreen);
			ay += ((int)cur.rgbBlue - up.rgbBlue) *((int)cur.rgbBlue - up.rgbBlue);
		}
		if (j > 0)
		{
			RGBQUAD left = img.GetPixelColor(j - 1, i);
			ax += ((int)cur.rgbRed - left.rgbRed) * ((int)cur.rgbRed - left.rgbRed);
			ax += ((int)cur.rgbGreen - left.rgbGreen) * ((int)cur.rgbGreen - left.rgbGreen);
			ax += ((int)cur.rgbBlue - left.rgbBlue) *((int)cur.rgbBlue - left.rgbBlue);
		}
		dy[i][j] = sqrt(ay / 3) / 255;
		dx[i][j] = sqrt(ax / 3) / 255;
	}
}

void savedata(const BYTE *data, int width, int height, int wsize, int hsize, CxImage &img)
{
	img.Create(width, height, 24);
	for(int i = 0; i < height; i ++) for(int j = 0; j < width; j ++)
	{
		for(int k = 0; k < 3; k ++) (img.GetBits(i))[j * 3 + k] = data[(i * wsize + j) * 4 + k];
	}
}

void reduce_image(Wml::GMatrixd &reduced, const Wml::GMatrixd &image)
{
	int ny = image.GetRows(), nx = image.GetColumns();
	const double filter[3][3] = {{0.0625, 0.125, 0.0625}, {0.125, 0.25, 0.125}, {0.0625, 0.125, 0.0625}};
	const int sample = 2, filter_size = 3, f_off = (filter_size - 1) / 2, snx = nx / sample, sny = ny / sample;
	int si = 0, sj = 0;
	reduced.SetSize(sny, snx);

	for(int i=0;i<sny;i++) for(int j=0;j<snx;j++) reduced[i][j]=0;

	for(int i = 1; i < ny - 1; i += 2) {
		sj = 0;
		for(int j = 1; j < nx - 1; j += 2) {
			double val = 0.0;
			for(int ii = 0; ii < filter_size; ii ++)
				for(int jj = 0; jj < filter_size; jj ++) {
					assert(i+ii-f_off>=0 && i+ii-f_off<ny);
					assert(j+jj-f_off>=0 && j+jj-f_off<nx);
					val += filter[ii][jj] * image[i + ii - f_off][j + jj - f_off];
				}
				assert(si==i/2 && sj==j/2);
				reduced[si][sj] = val;
				sj ++;
		}
		si ++;
	}

	if (nx % 2 == 0) {
		for(int i = 0; i < sny; i ++)
			reduced[i][snx - 1] = reduced[i][snx - 2];
	}

	if (ny % 2 == 0) {
		for(int j = 0; j < snx; j ++)
			reduced[sny - 1][j] = reduced[sny - 2][j];
	}
}


void reduce_image(std::vector< std::vector<RGBQUAD> > &reduced, const std::vector< std::vector<RGBQUAD> > &image)
{
	int ny = image.size(), nx = image[0].size();
	const double filter[3][3] = {{0.0625, 0.125, 0.0625}, {0.125, 0.25, 0.125}, {0.0625, 0.125, 0.0625}};
	const int sample = 2, filter_size = 3, f_off = (filter_size - 1) / 2, snx = nx / sample, sny = ny / sample;
	int si = 0, sj = 0;
	reduced = std::vector< std::vector<RGBQUAD> >(sny, std::vector<RGBQUAD>(snx));

	for(int i=0;i<sny;i++) for(int j=0;j<snx;j++) reduced[i][j].rgbBlue = reduced[i][j].rgbGreen = reduced[i][j].rgbRed = 0;

	for(int i = 1; i < ny - 1; i += 2) {
		sj = 0;
		for(int j = 1; j < nx - 1; j += 2) {
			double r = 0.0, g = 0.0, b = 0.0;
			for(int ii = 0; ii < filter_size; ii ++)
				for(int jj = 0; jj < filter_size; jj ++) {
					assert(i+ii-f_off>=0 && i+ii-f_off<ny);
					assert(j+jj-f_off>=0 && j+jj-f_off<nx);
					r += filter[ii][jj] * image[i + ii - f_off][j + jj - f_off].rgbRed;
					g += filter[ii][jj] * image[i + ii - f_off][j + jj - f_off].rgbGreen;
					b += filter[ii][jj] * image[i + ii - f_off][j + jj - f_off].rgbBlue;
				}
				assert(si==i/2 && sj==j/2);
				reduced[si][sj].rgbRed = (int)floor(r + 0.5);
				reduced[si][sj].rgbGreen = (int)floor(g + 0.5);
				reduced[si][sj].rgbBlue = (int)floor(b + 0.5);
				sj ++;
		}
		si ++;
	}

	if (nx % 2 == 0) {
		for(int i = 0; i < sny; i ++)
			reduced[i][snx - 1] = reduced[i][snx - 2];
	}

	if (ny % 2 == 0) {
		for(int j = 0; j < snx; j ++)
			reduced[sny - 1][j] = reduced[sny - 2][j];
	}
}

void reduce_image(std::vector< std::vector<int> > &reduced, const std::vector< std::vector<int> > &image)
{
	int ny = image.size(), nx = image[0].size();
	const int filter[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};
	const int sample = 2, filter_size = 3, f_off = (filter_size - 1) / 2, snx = nx / sample, sny = ny / sample;
	int si = 0, sj = 0;
	reduced = std::vector< std::vector<int> >(sny, std::vector<int>(snx, -1));

	for(int i = 1; i < ny - 1; i += 2) {
		sj = 0;
		for(int j = 1; j < nx - 1; j += 2) {
			double r = 0.0, g = 0.0, b = 0.0;
			int t[16];
			int cnt = 0;
			for(int ii = 0; ii < filter_size; ii ++)
				for(int jj = 0; jj < filter_size; jj ++) {
					for(int k = 0; k < filter[ii][jj]; k ++)
						t[cnt ++] = image[i + ii - f_off][j + jj - f_off];
				}
			std::sort(t, t + cnt);
			for(int i1 = 0, j1 = 0; i1 < cnt; i1 ++)
			{
				for(; j1 < cnt && t[j1] == t[i1]; j1 ++);
				if (j1 - i1 >= cnt / 2)
				{
					reduced[si][sj] = t[i1];
					break;
				}
			}
			sj ++;
		}
		si ++;
	}

	if (nx % 2 == 0) {
		for(int i = 0; i < sny; i ++)
			reduced[i][snx - 1] = reduced[i][snx - 2];
	}

	if (ny % 2 == 0) {
		for(int j = 0; j < snx; j ++)
			reduced[sny - 1][j] = reduced[sny - 2][j];
	}
}

void project_image(std::vector< std::vector<int> > &proj, const std::vector< std::vector<int> > &img)
{
	int ny = proj.size(), nx = proj[0].size();
	for(int i = 0; i < ny; i ++)
		for(int j = 0; j < nx; j ++) {
			proj[i][j] = -1;
		}

		int si = 0, sj = 0, snx = nx / 2, sny = ny / 2;
		for(int i = 1; i< ny; i += 2) {
			sj = 0;
			for(int j = 1; j < nx; j += 2) {
				proj[i][j] = img[si][sj];
				sj ++;
			}
			si ++;
		}

		for(int i = 1; i < ny - 1; i ++)
			for(int j = 1; j < nx - 1; j ++) {
				if (i % 2 == 0 && j % 2 == 0) {
					int t[4] = {proj[i - 1][j - 1], proj[i - 1][j + 1], proj[i + 1][j - 1], proj[i + 1][j + 1]};
					std::sort(t, t+4);
					for(int i1 = 0, j1 = 0; i1 < 4; i1 = j1) {
						for(; j1 < 4 && t[j1] == t[i1]; j1 ++);
						if (j1 - i1 >= 2)
						{
							proj[i][j] = t[i1];
							break;
						}
					}
				} else if (i % 2 == 0) {
					proj[i][j] = proj[i - 1][j];
				} else if (j % 2 == 0) {
					proj[i][j] = proj[i][j - 1];
				}
			}

			for(int i = 0; i < ny; i ++) {
				proj[i][0] = proj[i][1];
				proj[i][nx - 1] = proj[i][nx - 2];
			}
			for(int j = 0; j < nx; j ++) {
				proj[0][j] = proj[1][j];
				proj[ny - 1][j] = proj[ny - 2][j];
			}
}


inline double linear_interp(double low_value, double high_value, double position) {
	return low_value * (1.0 - position) + high_value * position;
}

void copy_image(Wml::GMatrixd &to, const Wml::GMatrixd &from)
{
	to = from;
}

void laplacian(Wml::GMatrixd &lap, const Wml::GMatrixd &image)
{
	int ny = image.GetRows(), nx = image.GetColumns();
	lap.SetSize(ny, nx);

	for(int i = 0; i < ny; i ++)
		for(int j = 0; j < nx ; j ++) {
			if (i == 0 || j == 0 || i == ny - 1 || j == nx - 1) lap[i][j] = 0.0;
			else lap[i][j] = (image[i][j] * 4.0 - image[i][j - 1] - image[i - 1][j] - image[i][j + 1] - image[i + 1][j]) / 16.0;
		}
}

void wrap_image(Wml::GMatrixd &wrap, const Wml::GMatrixd &image, const Wml::GMatrixd &u, const Wml::GMatrixd &v)
{
	int ny = image.GetRows(), nx = image.GetColumns();
	for(int i = 0; i < ny; i ++)
		for(int j = 0; j < nx; j ++) {
			double new_i = i - v[i][j], new_j = j - u[i][j];
			if (new_i < 0.0 || new_j < 0.0 || new_i > ny - 1 || new_j > nx - 1) {
				wrap[i][j] = 0.0;
				continue;
			}
			int floor_i = (int)floor(new_i), ceil_i = (int)ceil(new_i);
			int floor_j = (int)floor(new_j), ceil_j = (int)ceil(new_j);
			double remain_i = new_i - floor_i, remain_j = new_j - floor_j;

			wrap[i][j] = linear_interp(
				linear_interp(image[floor_i][floor_j], image[floor_i][ceil_j], remain_j),
				linear_interp(image[ceil_i][floor_j], image[ceil_i][ceil_j], remain_j),
				remain_i
				);
		}
}

void reduce_flow(Wml::GMatrixd &reduced, const Wml::GMatrixd &image)
{
	int ny = image.GetRows(), nx = image.GetColumns();
	const double filter[3][3] = {{0.0625, 0.125, 0.0625}, {0.125, 0.25, 0.125}, {0.0625, 0.125, 0.0625}};
	const int sample = 2, filter_size = 3, f_off = (filter_size - 1) / 2, snx = nx / sample, sny = ny / sample;
	int si = 0, sj = 0;
	reduced.SetSize(sny, snx);

	for(int i=0;i<sny;i++) for(int j=0;j<snx;j++) reduced[i][j]=0;

	for(int i = 1; i < ny - 1; i += 2) {
		sj = 0;
		for(int j = 1; j < nx - 1; j += 2) {
			double val = 0.0;
			for(int ii = 0; ii < filter_size; ii ++)
				for(int jj = 0; jj < filter_size; jj ++) {
					assert(i+ii-f_off>=0 && i+ii-f_off<ny);
					assert(j+jj-f_off>=0 && j+jj-f_off<nx);
					val += filter[ii][jj] * image[i + ii - f_off][j + jj - f_off];
				}
				assert(si==i/2 && sj==j/2);
				reduced[si][sj] = val / 2.0;
				sj ++;
		}
		si ++;
	}

	if (nx % 2 == 0) {
		for(int i = 0; i < sny; i ++)
			reduced[i][snx - 1] = reduced[i][snx - 2];
	}

	if (ny % 2 == 0) {
		for(int j = 0; j < snx; j ++)
			reduced[sny - 1][j] = reduced[sny - 2][j];
	}
}

void project_image(Wml::GMatrixd &proj, const Wml::GMatrixd &image)
{
	int ny = proj.GetRows(), nx = proj.GetColumns();
	for(int i = 0; i < ny; i ++)
		for(int j = 0; j < nx; j ++) {
			proj[i][j] = 0.0;
		}

		int si = 0, sj = 0, snx = nx / 2, sny = ny / 2;
		for(int i = 1; i< ny; i += 2) {
			sj = 0;
			for(int j = 1; j < nx; j += 2) {
				proj[i][j] = image[si][sj] * 2.0;
				sj ++;
			}
			si ++;
		}

		for(int i = 1; i < ny - 1; i ++)
			for(int j = 1; j < nx - 1; j ++) {
				if (i % 2 == 0 && j % 2 == 0) {
					proj[i][j] = (proj[i - 1][j - 1] + proj[i - 1][j + 1] + proj[i + 1][j - 1] + proj[i + 1][j + 1]) / 4.0;
				} else if (i % 2 == 0) {
					proj[i][j] = (proj[i - 1][j] + proj[i + 1][j]) / 2.0;
				} else if (j % 2 == 0) {
					proj[i][j] = (proj[i][j - 1] + proj[i][j + 1]) / 2.0;
				}
			}

			for(int i = 0; i < ny; i ++) {
				proj[i][0] = proj[i][1];
				proj[i][nx - 1] = proj[i][nx - 2];
			}
			for(int j = 0; j < nx; j ++) {
				proj[0][j] = proj[1][j];
				proj[ny - 1][j] = proj[ny - 2][j];
			}
}

