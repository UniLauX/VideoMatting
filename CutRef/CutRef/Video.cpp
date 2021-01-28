#include "Video.h"
#include "filenamedetail.h"
#include <iostream>
#include <fstream>

using namespace std;

Seq_t::Seq_t(string name, int start, int end, int _optStart, int _optEnd)
{
    SetInfo(name, start, end, _optStart, _optEnd);
}

void Seq_t::SetInfo(string name, int start, int end, int _optStart, int _optEnd)
{
    this->name=name;
    this->start=start;
    this->end=end;
	this->optStart = _optStart;
	this->optEnd = _optEnd;
    
    CxImage t;
    if(LoadImage(start, t))
    {
        width=t.GetWidth();
        height=t.GetHeight();
    }
    else
        width=height=0;
}

bool Seq_t::LoadImage(int num, CxImage &image)
{
    string t;
    char name[128];

    if(num<start)
        return false;

    t=FileName::increase_filename_number(this->name, num);//-start);
    sprintf_s(name, "%s", t.c_str());
	GetFileTitle(name);
    if(image.Load(name))
        return true;
    else
        return false;
}

void Seq_t::GetFileTitle(string _name)
{
	//int i = _name.length()-1; 
	//while (_name[i]!='.')
	//	i--;
	//int index = _name.rfind('/');
	//fileTitle = _name.substr(index+1, i-1-index);
	fileTitle = FileName::find_filename_name_without_ext(_name);
}