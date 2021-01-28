#include "stdafx.h"
#include "ImageBuffer.h"
#include <math.h>


ImageBuffer::ImageBuffer()
{
	name="Data";
     extName="jpg";
     currentFrame=0;
     empty=true;
	 flag = false;
}

ImageBuffer::~ImageBuffer()
{

}

void ImageBuffer::ClearBuffer()
{
     frame.Destroy();
}

bool ImageBuffer::AddImage(int framePos)
{
	CString str;
	CString temp="";
	temp.AppendFormat("%d",framePos);
	if (temp.GetLength()>=namelength)
	{
		str.Format(_T("%s%s.%s"),(LPCTSTR)name,(LPCTSTR)temp,(LPCTSTR)extName);
	}
	else
	{
		CString temps="%.";
		temps.AppendFormat("%d%s",namelength,"d");
		CString tempname;
		tempname.Format(temps,framePos);
		str.Format(_T("%s%s.%s"),(LPCTSTR)name,(LPCTSTR)tempname,(LPCTSTR)extName);
	}
	//printf("%s", name);
	//printf("%s", str);
	if(frame.Load((LPCTSTR)str))
		return true;
	else
		return false;
}

bool ImageBuffer::Forward()
{
     if(AddImage(currentFrame+1))
     {
          ++currentFrame;
          return true;
     }
     else
          return false;
}

bool ImageBuffer::Back()
{
     if(AddImage(currentFrame-1))
     {
          --currentFrame;
          return true;
     }
     else
          return false;
}

CxImage *ImageBuffer::GetImage()
{
	if(empty){
		
          return NULL;
	}
	else{
          return &frame;
	}
}

bool ImageBuffer::OpenVideo(int framePos)
{
     if(AddImage(framePos))
     {
          currentFrame=framePos;
          empty=false;
          return true;
     }
     else
          return false;
}

void ImageBuffer::CloseVideo()
{
     empty=true;
}

bool ImageBuffer::OpenImage(const char *filename)
{
     if(frame.Load(filename))
     {
          empty=false;
          return true;
     }
     else
          return false;
}

bool ImageBuffer::OpenImage(const CxImage &image)
{
     frame.Copy(image);
     empty=false;
     return true;
}

bool ImageBuffer::Goto(int framePos)
{
     if(AddImage(framePos))
     {
          currentFrame=framePos;
          return true;
     }
     else
          return false;
}

void ImageBuffer::GetImageSize(int &width, int &height)
{
     if(empty)
     {
          width=0;
          height=0;
     }
     else
     {
          width=frame.GetWidth();
          height=frame.GetHeight();
     }
}

CString ImageBuffer::GetFrameName()
{
	CString str;
	CString temp="";
	temp.AppendFormat("%d",currentFrame);
	if (temp.GetLength()>=namelength)
	{
		str.Format(_T("%s%s.%s"),(LPCTSTR)name,(LPCTSTR)temp,(LPCTSTR)extName);
	}
	else
	{
		CString temps="%.";
		temps.AppendFormat("%d%s",namelength,"d");
		CString tempname;
		tempname.Format(temps,currentFrame);
		str.Format(_T("%s%s.%s"),(LPCTSTR)name,(LPCTSTR)tempname,(LPCTSTR)extName);
	}
	//str.Format(_T("%s/%d.%s"), (LPCTSTR)name, currentFrame, (LPCTSTR)extName);
	return str;
}

CString ImageBuffer::GetFrameName(int frameno)
{
	CString str;
	CString temp="";
	temp.AppendFormat("%d",frameno);
	if (temp.GetLength()>=namelength)
	{
		str.Format(_T("%s%s.%s"),(LPCTSTR)name,(LPCTSTR)temp,(LPCTSTR)extName);
	}
	else
	{
		CString temps="%.";
		temps.AppendFormat("%d%s",namelength,"d");
		CString tempname;
		tempname.Format(temps,frameno);
		str.Format(_T("%s%s.%s"),(LPCTSTR)name,(LPCTSTR)tempname,(LPCTSTR)extName);
	}
	return str;
}

CString ImageBuffer::GetPreviousFrameName()
{
	CString str;
	CString temp="";
	temp.AppendFormat("%d",currentFrame-1);
	if (temp.GetLength()>=namelength)
	{
		str.Format(_T("%s%s.%s"),(LPCTSTR)name,(LPCTSTR)temp,(LPCTSTR)extName);
	}
	else
	{
		CString temps="%.";
		temps.AppendFormat("%d%s",namelength,"d");
		CString tempname;
		tempname.Format(temps,currentFrame-1);
		str.Format(_T("%s%s.%s"),(LPCTSTR)name,(LPCTSTR)tempname,(LPCTSTR)extName);
	}
	return str;
}
CString ImageBuffer::GetFileName()
{
	CString name=this->GetFrameName();
	int index=name.ReverseFind('\\');

	name.Delete(0,index+1);
	return name;
}

int ImageBuffer::GetFramePos()
{
     return currentFrame;
}

void ImageBuffer::SetName(CString &name, CString &ext)
{
	//printf("in set name two\n");
     this->name=name;
     this->extName=ext;
}

//void ImageBuffer::SetName(CString &name)
//{
//	printf("in set name one\n");
//     this->name=name;
//     this->extName=_T("jpg");
//}

CString ImageBuffer::GetName()
{
	return this->name;
}

CString ImageBuffer::GetExt()
{
	return this->extName;
}

void ImageBuffer::SetPos(int pos)
{
     currentFrame=pos;
}

void ImageBuffer::SetBoolOption(bool b)
{
	empty=b;
}
