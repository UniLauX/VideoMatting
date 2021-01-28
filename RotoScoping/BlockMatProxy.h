#ifndef BLOCKMATPROXY_H
#define BLOCKMATPROXY_H
#include <assert.h>
#include <vector>
#include <hash_map>
#include "../RotoScoping/SSEBlock.h"
#include "../RotoScoping/LMData.h"
using stdext::hash_map;

struct Block_Offset{
	int Block;
	int Offset;
	Block_Offset():Block(0),Offset(0){}
	Block_Offset(int iBlock,int iOffset):Block(iBlock),Offset(iOffset){}
};
class BlockMatProxy
{
public:
	int nBlockRows;		//以SSEBlock为单位
	int nBlockCols;		//以SSEBlock为单位
	
	int nRows;			//以普通矩阵的一行一列为单位
	int nCols;
	


	std::vector<int> * NonZeroRowBlockIndex;		//记录这个块矩阵里面每一列非0的block所处的行（单位为block）
	stdext::hash_map<int,Block_Offset> * Row_BlockCorrespondence;		//记录每一行(单位为一行)所属的该列的block的number.第一个为rowIndex,第二个为对应的block的index,以及在这个block里面的offset
	SSEBlock *** Cols;



	void Create(int nBlockRows,int nBlockCols,int nRows,int nCols);
	~BlockMatProxy();
	void setValue(float value,int rowIndex,int colIndex);
	void setValue2(double value,int BlockRowIndex,int OffsetInBlock,int colIndex);
	void Visualize();

private:


};

void SetFrameBlock(int StartRow,int StartRowBlock,int StartColBlock,int NumOfFrame,int NumOfControlPoint,int NumOfRowPerBlock1,int NumOfRowPerBlock2,BlockMatProxy & JacMat);
void InitializeBlockMat(int NumOfFrame,int NumOfParam,int NumOfControlPoint,LMData * adata,int xLenght,BlockMatProxy & JacBlockMat);

#endif
