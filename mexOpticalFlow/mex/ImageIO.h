#ifndef _ImageIO_h
#define _ImageIO_h

//#include <QVector>
//#include <QImage>
//#include <QString>
//#include <QVector.h>
#include <Qt/qvector.h>
//#include <QImage.h>
#include <Qt/qimage.h>
//#include <QString.h>
#include <Qt/qstring.h>

#ifdef _DEBUG
#pragma comment(lib, "QtCored4.lib")
#pragma comment(lib, "QtGuid4.lib")
#else if
#pragma comment(lib, "QtCore4.lib")
#pragma comment(lib, "QtGui4.lib")
#endif

#include "math.h"
//-----------------------------------------------------------------------------------------
// this class is a wrapper to use QImage to load image into image planes
//-----------------------------------------------------------------------------------------

class ImageIO
{
public:
	enum ImageType{standard, derivative, normalized};
	ImageIO(void);
	~ImageIO(void);
public:
	template <class T>
	static void loadImage(const QImage& image,T*& pImagePlane,int& width,int& height,int& nchannels);
	template <class T>
	static bool loadImage(const QString& filename,T*& pImagePlane,int& width,int& height,int& nchannels);

	template <class T>
	static unsigned char convertPixel(const T& value,bool IsFloat,ImageType type,T& _Max,T& _Min);

	template <class T>
	static bool writeImage(const QString& filename, const T*& pImagePlane,int width,int height,int nchannels,ImageType type=standard,int quality=-1);
};

template <class T>
void ImageIO::loadImage(const QImage& image, T*& pImagePlane,int& width,int& height,int& nchannels)
{
	// get the image information
	width=image.width();
	height=image.height();
	nchannels=3;
	pImagePlane=new T[width*height*nchannels];

	// check whether the type is float point
	bool IsFloat=false;
	if(typeid(T)==typeid(double) || typeid(T)==typeid(float) || typeid(T)==typeid(long double))
		IsFloat=true;

	const unsigned char* plinebuffer;
	for(int i=0;i<height;i++)
	{
		plinebuffer=image.scanLine(i);
		for(int j=0;j<width;j++)
		{
			if(IsFloat)
			{
				pImagePlane[(i*width+j)*3]=(T)plinebuffer[j*4]/255;
				pImagePlane[(i*width+j)*3+1]=(T)plinebuffer[j*4+1]/255;
				pImagePlane[(i*width+j)*3+2]=(T)plinebuffer[j*4+2]/255;
			}
			else
			{
				pImagePlane[(i*width+j)*3]=plinebuffer[j*4];
				pImagePlane[(i*width+j)*3+1]=plinebuffer[j*4+1];
				pImagePlane[(i*width+j)*3+2]=plinebuffer[j*4+2];
			}
		}
	}
}

template <class T>
bool ImageIO::loadImage(const QString&filename, T*& pImagePlane,int& width,int& height,int& nchannels)
{
	QImage image;
	if(image.load(filename)==false)
		return false;
	if(image.format()!=QImage::Format_RGB32)
	{
		QImage temp=image.convertToFormat(QImage::Format_RGB32);
		image=temp;
	}
	loadImage(image,pImagePlane,width,height,nchannels);
	return true;
}

template <class T>
bool ImageIO::writeImage(const QString& filename, const T*& pImagePlane,int width,int height,int nchannels,ImageType type,int quality)
{
	int nPixels=width*height,nElements;
	nElements=nPixels*nchannels;
	unsigned char* pTempBuffer;
	pTempBuffer=new unsigned char[nPixels*4];
	memset(pTempBuffer,0,nPixels*4);

	// check whether the type is float point
	bool IsFloat=false;
	if(typeid(T)==typeid(double) || typeid(T)==typeid(float) || typeid(T)==typeid(long double))
		IsFloat=true;

	T _Max=0,_Min=0;
	switch(type){
		case standard:
			break;
		case derivative:
			_Max=0;
			for(int i=0;i<nPixels;i++)
			{
				if(IsFloat)
					_Max=__max(_Max,fabs((double)pImagePlane[i]));
				else
					_Max=__max(_Max,abs(pImagePlane[i]));
			}
			break;
		case normalized:
			_Min=_Max=pImagePlane[0];
			for(int i=1;i<nElements;i++)
			{
				_Min=__min(_Min,pImagePlane[i]);
				_Max=__max(_Max,pImagePlane[i]);
			}
			break;
	}

	for(int i=0;i<nPixels;i++)
	{
		if(nchannels>=3)
		{
			pTempBuffer[i*4]=convertPixel(pImagePlane[i*nchannels],IsFloat,type,_Max,_Min);
			pTempBuffer[i*4+1]=convertPixel(pImagePlane[i*nchannels+1],IsFloat,type,_Max,_Min);
			pTempBuffer[i*4+2]=convertPixel(pImagePlane[i*nchannels+2],IsFloat,type,_Max,_Min);
		}
		else 
			for (int j=0;j<3;j++)
				pTempBuffer[i*4+j]=convertPixel(pImagePlane[i*nchannels],IsFloat,type,_Max,_Min);
		pTempBuffer[i*4+3]=255;
	}
	QImage *pQImage=new QImage(pTempBuffer,width,height,QImage::Format_RGB32);
	bool result= pQImage->save(filename,0,quality);
	delete pQImage;
	delete pTempBuffer;
	return result;
}

template <class T>
unsigned char ImageIO::convertPixel(const T& value,bool IsFloat,ImageType type,T& _Max,T& _Min)
{
	switch(type){
		case standard:
			if(IsFloat)
				return __max(__min(value*255,255),0);
			else
				return __max(__min(value,255),0);
			break;
		case derivative:
			return (double)((double)value/_Max+1)/2*255;
			break;
		case normalized:
			return (double)(value-_Min)/(_Max-_Min)*255;
			break;
	}
	return 0;
}

#endif