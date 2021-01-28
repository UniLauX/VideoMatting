
#ifndef _INC_BDRF_H
#define _INC_BDRF_H

#include"vsfdef.h"


class _VSF_API vsf_bdrf_param
{
public:
	int		m_bhwsz,m_shwsz,m_rhwsz;
	float	m_bmin,m_bmax;

	float   m_ssig;         //sig=0.1+err*ssig, where err is the ambiguity of the pixel

	float   m_mu0,m_mus;      //mu=mu0+mus* sum_(avc-avs), where avc is the color alpha value, avs is the smooth alpha value

	float   m_msw;          //min-smooth alpha-weight, sw=msw+(1-msw)*err;

	float   m_pe;			//alpha=pow(alpha,pe);

public:
	vsf_bdrf_param();
};

/* 对二值分割的边界进行后处理，以消除微小的误分割

@foreMaskVal : @mask中用于表示前景的值，通常为255。在通过sum-filter计算像素离边界的距离时使用。

@dest => @alpha
*/
_VSF_API void vsf_refine_boundary(const uchar *img, int width, int height, int istep, const uchar *alpha, int astep, int foreMaskVal, uchar *dest, int dstep, const vsf_bdrf_param *param=NULL);


#endif


