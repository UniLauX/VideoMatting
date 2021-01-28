#ifndef VISUALIZEMAT_H
#define VISUALIZEMAT_H

#include <vector>
#include <hash_map>
#include "../RotoScoping/BlockMatProxy.h"


void VisMat(std::vector<int> *NonZeroRowIndex,int NumOfRow,int NumOfCol,char * FileName);
void VisMat(stdext::hash_map<int,Block_Offset> *NonZeroRowIndex,int NumOfRow,int NumOfCol,char * FileName);
#endif