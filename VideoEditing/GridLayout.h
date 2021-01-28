#ifndef GRIDLAYOUT_H
#define GRIDLAYOUT_H

#include "ViewObject.h"
#include <vector>

class GridLayout : public ViewObject
{

     int row;
     int col;
     int rowBound;
     int colBound;
     
     std::vector<ViewObject *> objects;

public:

     GridLayout();
     ~GridLayout();

     void SetInfo(CRect &info);
     void SetGridSize(int row, int col);
     void AddObject(ViewObject *object);

     WINDOW GetWindowInfo(int row, int col);
     WINDOW GetWindowInfo(ViewObject *object);

     ViewObject * SelectObject(int x, int y);
     virtual void Draw(HDC hdc);

     void GetWindowPos(int &r, int &c, ViewObject *object);
     virtual void Zoom(int d, float s, int x, int y);
};


#endif