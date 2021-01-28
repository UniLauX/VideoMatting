#pragma once

#include <malloc.h>
namespace SIFT{

template<typename T>
class ImageArray//: private Wml::GMatrix<T>
{
protected:
	int m_nWidth;
	int m_nHeight;

	int m_nWidthActual;

	static __forceinline int expandAlign(int w)
	{
		return (w*sizeof(T)+3)/4*4/sizeof(T);

		//return w + 3 - (w - 1) % 4;
	}

public:
	T* m_afData;
	T** m_aafEntry;

	typedef T* PointerType;
	typedef T** EntryType;

	void SetSize(int height, int width)
	{
		if(m_afData)
			delete[] m_afData;
		if(m_aafEntry)
			delete[] m_aafEntry;


		m_nWidth = width;
		m_nHeight = height;
		m_nWidthActual = expandAlign(width);

		m_afData = new T[m_nWidthActual * m_nHeight];
// 		m_afData = (T*)_aligned_malloc(sizeof(T) * m_nWidthActual * m_nHeight, 16);

		m_aafEntry = new PointerType[m_nHeight];

		T* ptr = m_afData;
		for(int i=0;i<m_nHeight;i++)
		{
			m_aafEntry[i] = ptr;
			ptr += m_nWidthActual;
		}
	}

public:

	ImageArray(): m_afData(0),m_aafEntry(0)
	{
		SetSize(0, 0);
	}

	//ImageArray(int width, int height):Wml::GMatrix<T>(height, width), m_nWidth(m_iCols), m_nHeight(m_iRows), m_pImageIntegral(NULL){}
	ImageArray(int width, int height): m_afData(0),m_aafEntry(0)
	{
		SetSize(height, width);
	}

	ImageArray(const ImageArray& that): m_afData(0),m_aafEntry(0)
	{
		SetSize(that.Height(), that.Width());
		memcpy(m_afData, that.m_afData, sizeof(T) * that.m_nWidthActual * that.m_nHeight);
	}

	~ImageArray()
	{
		if(m_afData)
		{
			//_aligned_free(m_afData);
			delete []m_afData;
		}

		if(m_aafEntry)
			delete []m_aafEntry;
	}

	ImageArray* Clone()
	{
		return new ImageArray(*this);
	}

	void CreateDataArray(int width, int height)
	{
		m_nWidthActual = expandAlign(width);
		SetSize(height, m_nWidthActual);
		m_nWidth = width;
		m_nHeight = height;
	}

	__forceinline T& At(int x, int y)
	{
		XASSERT(m_afData);
		XASSERT(x >= 0 && x < m_nWidth && y >= 0 && y < m_nHeight);
		return m_aafEntry[y][x];
	}

	__forceinline const int Width() const {return m_nWidth;}
	__forceinline const int Height() const {return m_nHeight;}

	T MaxElement();

	ImageArray* ScaleHalf();
	ImageArray* ScaleDouble();

	bool Normalize();

	void fillShiftedImage(int shift, ImageArray& dst);

	ImageArray* operator *(ImageArray& that);
	ImageArray* operator +(ImageArray& that);
	ImageArray* operator -(ImageArray& that);
	operator T*(){return m_afData;}
	T* operator[](int idx){return m_aafEntry[idx];}

	T boxSum(int x, int y, int cx, int cy, bool useIntegralImaage = true);
};

#include "ImageArray.inl"

typedef ImageArray<float> ImageArrayf;
typedef ImageArray<double> ImageArrayd;

} // namespace SIFT