#include "GridLayout.h"

using namespace std;

GridLayout::GridLayout()
{
     rowBound=0;
	 colBound=0;
}

GridLayout::~GridLayout()
{
	vector<ViewObject *>::iterator i;
	for(i=objects.begin();i!=objects.end();++i)
		delete (*i);
}

void GridLayout::SetGridSize(int row, int col)
{
     this->row=row;
     this->col=col;
}

void GridLayout::SetInfo(CRect &info)
{
     int index;
     int r,c;
     vector<ViewObject *>::iterator i;

     ViewObject::SetInfo(info);
     for(index=0,i=objects.begin();i!=objects.end();++i,++index)
     {
          r=index/col;
          c=index-r*col;
          (*i)->SetInfo(GetWindowInfo(r, c));
     }
}

void GridLayout::AddObject(ViewObject *object)
{
     int num;

     num=(int)objects.size();
     if(num>=row*col)
          return;

     objects.push_back(object);
}

WINDOW GridLayout::GetWindowInfo(int r, int c)
{
     WINDOW info;

     info.width=(size.width-(col+1)*colBound)/col;
     info.height=(size.height-(row+1)*rowBound)/row;
     info.x=colBound*(c+1)+info.width*c;
     info.y=rowBound*(r+1)+info.height*r;

     return info;
}

WINDOW GridLayout::GetWindowInfo(ViewObject *object)
{
     int i;
     int size;
     int r,c;

     size=(int)objects.size();
     for(i=0;i<size;++i)
     {
          if(objects[i]==object)
               break;
     }
     r=i/col;
     c=i-r*col;

     return GetWindowInfo(r, c);
}

ViewObject *GridLayout::SelectObject(int x, int y)
{
     WINDOW info;
     int r,c;
     int x0,y0;
     int index;
     vector<ViewObject *>::iterator i;

     x0=x-size.x;
     y0=y-size.y;
     for(index=0, i=objects.begin();i!=objects.end();++i,++index)
     {
          r=index/col;
          c=index-r*col;
          info=GetWindowInfo(r, c);
          if(x0>=info.x && x0<=info.x+info.width &&
               y0>=info.y && y0<=info.y+info.height)
               return *i;
     }
     return NULL;
}

void GridLayout::Draw(HDC hdc)
{
     vector<ViewObject *>::iterator i;
     for(i=objects.begin();i!=objects.end();++i)
          (*i)->Draw(hdc);
}

void GridLayout::GetWindowPos(int &r, int &c, ViewObject *object)
{
    int i;
    int size;

    size=(int)objects.size();
    for(i=0;i<size;++i)
    {
        if(objects[i]==object)
            break;
    }
    r=i/col;
    c=i-r*col;
}

void GridLayout::Zoom(int d, float s, int x, int y)
{
    int r,c;
    int a,b;
    int X,Y;
    WINDOW size;
    ViewObject *obj;
    vector<ViewObject *>::iterator iter;

    if((obj=SelectObject(x, y))==NULL)
        return;

    GetWindowPos(r, c, obj);
    size=GetWindowInfo(r, c);
    for(iter=objects.begin();iter!=objects.end();++iter)
    {
        GetWindowPos(a, b, *iter);
        X=(b-c)*size.width+x;
        Y=(a-r)*size.height+y;
        (*iter)->Zoom(d, s, X, Y);
    }
}
