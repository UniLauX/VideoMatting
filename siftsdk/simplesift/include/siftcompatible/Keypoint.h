// Keypoint.h: interface for the CKeypoint class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KEYPOINT_H__2ECFE70D_1CF4_4A6C_85AC_B15A751F7077__INCLUDED_)
#define AFX_KEYPOINT_H__2ECFE70D_1CF4_4A6C_85AC_B15A751F7077__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ImageArray.h"
#include <vector>

namespace SIFT{
//typedef std::vector<double> DoubleVector;
typedef std::vector<float>	FloatVector;

// The interface to be implemented by all data elements within the
// kd-tree. As every element is represented in domain space by a
// multidimensional vector, the interface provides readonly methods to
// access into this vector and to get its dimension.
class IKDTreeDomain
{
public:
	virtual int DimensionCount() const
	{
		return -1;
	}
	
	virtual float GetDimensionElement (int dim) const { return -1; }	
};

class CKeypoint : public IKDTreeDomain
{
public:
	// Keypoint constructor.
	//
	// image: The smoothed gaussian image the keypoint was located in.
	// x, y: The subpixel level coordinates of the keypoint.
	// imgScale: The scale of the gaussian image, with 1.0 being the original
	//    detail scale (source image), and doubling at each octave.
	// kpScale: The scale of the keypoint.
	// orientation: Orientation degree in the range of [-M_PI ; M_PI] of the
	//    keypoint.
	//
	// First add a keypoint, then use 'MakeDescriptor' to generate the local
	CKeypoint(){
		image = NULL;
		x = y = 0.0;
		z = 1.0;
		imgScale = scale = 1.0;
		orientation = 0.0;
		m_iReserved = 0;
		m_iStatus = 0;
	}
	CKeypoint(ImageArrayf * image, float x, float y, float imgScale, float kpScale, float orientation);

	virtual ~CKeypoint();

    ImageArrayf * Image()
	{
		return image;
    }
    
	inline float & X(){return x;}
	inline float & Y(){return y;}
	inline float & Z(){return z;}

	inline float & ImgScale(){return imgScale;}
	inline float & Scale(){return scale;}
	inline float & Orientation(){return orientation;}
    
    inline int FVLinearDim()
	{
	    return m_iDimension;
	}

	inline float FVLinearGet(int idx)
	{
		return m_descriptor[idx];
	}

	inline void FVLinearSet(int idx, float value)
	{
		m_descriptor[idx] = value;
	}

    void CreateVector (int xD, int yD, int oD)
	{
		m_iDimension = yD * xD * oD;

		m_descriptor.resize(m_iDimension, 0);
	}

	void CreateVector (int iDimension)
	{
		m_iDimension = iDimension;

		m_descriptor.resize(iDimension, 0);
	}
	
	CKeypoint* Clone()
	{
		CKeypoint* pClone = new CKeypoint(image, x,y,imgScale,scale,orientation);
		*pClone = *this;

		return pClone;
	}
    
	virtual int DimensionCount() const
	{
		return m_iDimension;
	}

	float* ToANN()
	{
		return &m_descriptor[0];
	}

	static CKeypoint* FromANN(float* pt)
	{
		return (CKeypoint*)(pt);
	}

	// Temp Data Section
public:
	FloatVector m_descriptor;

	ImageArrayf * image;

	float x, y, z;
	float imgScale;// The scale of the image the keypoint was found in

	float scale;
	float orientation;

	int	m_iDimension;

public:
	int ptno;
	float dValue;

	int m_iReserved;

	//Version101: add this variable
	int m_iStatus;

	// pixel color of the point. [0, 255]
	unsigned char m_red;
	unsigned char m_green;
	unsigned char m_blue;
};

} // namespace SIFT

#endif // !defined(AFX_KEYPOINT_H__2ECFE70D_1CF4_4A6C_85AC_B15A751F7077__INCLUDED_)
