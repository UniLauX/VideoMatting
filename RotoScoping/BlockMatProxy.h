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
	int nBlockRows;		//��SSEBlockΪ��λ
	int nBlockCols;		//��SSEBlockΪ��λ
	
	int nRows;			//����ͨ�����һ��һ��Ϊ��λ
	int nCols;
	


	std::vector<int> * NonZeroRowBlockIndex;		//��¼������������ÿһ�з�0��block�������У���λΪblock��
	stdext::hash_map<int,Block_Offset> * Row_BlockCorrespondence;		//��¼ÿһ��(��λΪһ��)�����ĸ��е�block��number.��һ��ΪrowIndex,�ڶ���Ϊ��Ӧ��block��index,�Լ������block�����offset
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
