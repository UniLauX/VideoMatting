#ifndef VIEWOBJECT_H
#define VIEWOBJECT_H

#include "stdafx.h"
#include "resource.h"

typedef struct Window_t 
{

     int x;
     int y;
     int width;
     int height;

}WINDOW;

class ViewObject
{

protected:

     WINDOW size;

public:

     virtual ~ViewObject(){};
     virtual void Draw(HDC hdc) = 0;
     virtual void Zoom(int d, float s, int x, int y) = 0;

     virtual void SetInfo(CRect &info)
     {
          size.width=info.Width();
          size.height=info.Height();
          size.x=info.left;
          size.y=info.top;
     }

     virtual void SetInfo(WINDOW &info)
     {
          size=info;
     }

     virtual void GetWinSize(WINDOW &size)
     {
         size=this->size;
     }

};


#endif