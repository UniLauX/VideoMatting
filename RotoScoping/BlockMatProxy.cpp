#include "../RotoScoping/stdafx.h"

#include "../RotoScoping/BlockMatProxy.h"
#include "../RotoScoping/VisualizeMat.h"
#include "../RotoScoping/Configuration.h"


void BlockMatProxy::Create(int nBlockRows,int nBlockCols,int nRows,int nCols)
{
	this->nBlockRows = nBlockRows;
	this->nBlockCols = nBlockCols;
	this->nRows = nRows;
	this->nCols = nCols;
	NonZeroRowBlockIndex = new std::vector<int>[nCols];		
	Row_BlockCorrespondence = new stdext::hash_map<int,Block_Offset>[nCols];		


	Cols = new SSEBlock **[nCols];
	for (int c=0;c<nCols;c++)
	{
		Cols[c] = new SSEBlock*[nRows];
		memset(Cols[c],0,sizeof(SSEBlock *) * nRows);
	}
	printf("nCols = %d,nRows= %d\n",nCols,nRows);
}
BlockMatProxy::~BlockMatProxy()
{
	
	delete []NonZeroRowBlockIndex;
	delete []Row_BlockCorrespondence;
	for (int c=0;c<nBlockCols;c++)
	{
		for (int r=0;r<nBlockRows;r++)
		{
			delete Cols[c][r];
		}
		delete []Cols[c];
	}
	delete []Cols;
}

void BlockMatProxy::setValue(float value,int rowIndex,int colIndex)
{

	assert(rowIndex < nRows && colIndex < nCols);
	Block_Offset BlockInfo = Row_BlockCorrespondence[colIndex][rowIndex];
	//printf("colIndex=%d,Block=%d,Offset=%d\n",colIndex,BlockInfo.Block,BlockInfo.Offset);
	if (Cols[colIndex][BlockInfo.Block] == NULL)
	{
		printf("null!!!!!!!!!\n");
		system("pause");
	}
	Cols[colIndex][BlockInfo.Block]->SetValue(BlockInfo.Offset,value);
}

void BlockMatProxy::setValue2(double value,int BlockRowIndex,int OffsetInBlock,int colIndex)
{
	if (Cols[colIndex][BlockRowIndex] == NULL)
	{
		printf("SetValue2 null! BlockRowIndex =%d, ColIndex=%d\n",BlockRowIndex,colIndex);
		system("pause");
	}
	//printf("BlockRowIndex = %d,colIndex=%d,OffSetInBlock=%d\n",BlockRowIndex,colIndex,OffsetInBlock);
	//system("pause");
	Cols[colIndex][BlockRowIndex]->SetValue(OffsetInBlock,value);

}

void BlockMatProxy::Visualize()
{

	VisMat(Row_BlockCorrespondence,nRows,nCols,"d:\\jacRowBlockCorrespondenc.jpg");
	VisMat(NonZeroRowBlockIndex,nBlockRows,nBlockCols,"d:\\jacBlock.jpg");
	system("pause");
}


void SetFrameBlock(int StartRow,int StartRowBlock,int StartColBlock,int NumOfFrame,int NumOfControlPoint,int NumOfRowPerBlock1,int NumOfRowPerBlock2,BlockMatProxy & JacMat)
{
	// 默认一个block的列为1列
	// 这里摆一帧的一条band
	int RowStartCol = StartColBlock;
	int CurrentRowBlock = StartRowBlock;
	int CurrentRow = StartRow;
	//前面的列
	for (int ControlPoint = 0; ControlPoint < NumOfControlPoint - 2; ControlPoint++)
	{
		for (int i=0;i<8;i++)
		{
			JacMat.Cols[RowStartCol + i][CurrentRowBlock] = new SSEBlock(NumOfRowPerBlock1);
			JacMat.NonZeroRowBlockIndex[RowStartCol + i].push_back(CurrentRowBlock);

			for (int j=0;j<NumOfRowPerBlock1;j++)
			{
				(JacMat.Row_BlockCorrespondence[RowStartCol + i])[j + CurrentRow] = Block_Offset(CurrentRowBlock,j);
			}

		}
		CurrentRowBlock++;
		RowStartCol += 4;
		CurrentRow += NumOfRowPerBlock1;
	}

	for (int i=0;i<8;i++)
	{
		JacMat.Cols[RowStartCol + i][CurrentRowBlock] = new SSEBlock(NumOfRowPerBlock2);
		JacMat.NonZeroRowBlockIndex[RowStartCol + i].push_back(CurrentRowBlock);
		for (int j=0;j<NumOfRowPerBlock2;j++)
		{
			(JacMat.Row_BlockCorrespondence[RowStartCol + i])[j + CurrentRow] = Block_Offset(CurrentRowBlock,j);
		}

	}

}
void SetFrameBlockSpecialEC(int StartRow,int StartRowBlock,int StartColBlock,int NumOfFrame,int NumOfControlPoint,int NumOfRowPerBlock1,int NumOfRowPerBlock2,BlockMatProxy & JacMat)
{
	// 默认一个block的列为1列
	// 这里摆一帧的一条band
	int RowStartCol = StartColBlock;
	int CurrentRowBlock = StartRowBlock;
	int CurrentRow = StartRow;
	//前面的列
	for (int ControlPoint = 0; ControlPoint < NumOfControlPoint - 2; ControlPoint++)
	{
		for (int i=0;i<12;i++)
		{
			JacMat.Cols[RowStartCol + i][CurrentRowBlock] = new SSEBlock(NumOfRowPerBlock1);
			JacMat.NonZeroRowBlockIndex[RowStartCol + i].push_back(CurrentRowBlock);

			for (int j=0;j<NumOfRowPerBlock1;j++)
			{
				(JacMat.Row_BlockCorrespondence[RowStartCol + i])[j + CurrentRow] = Block_Offset(CurrentRowBlock,j);
			}

		}
		CurrentRowBlock++;
		RowStartCol += 4;
		CurrentRow += NumOfRowPerBlock1;
	}

	for (int i=0;i<8;i++)
	{
		JacMat.Cols[RowStartCol + i][CurrentRowBlock] = new SSEBlock(NumOfRowPerBlock2);
		JacMat.NonZeroRowBlockIndex[RowStartCol + i].push_back(CurrentRowBlock);
		for (int j=0;j<NumOfRowPerBlock2;j++)
		{
			(JacMat.Row_BlockCorrespondence[RowStartCol + i])[j + CurrentRow] = Block_Offset(CurrentRowBlock,j);
		}

	}

}
void InitializeBlockMat(int NumOfFrame,int NumOfParam,int NumOfControlPoint,LMData * adata,int xLenght,BlockMatProxy & JacBlockMat)
{
	//各个能量项所占的block的数目
	int nEVblock = 2 * (NumOfControlPoint - 1)*(NumOfFrame - 1);
	int nEGblock = (NumOfControlPoint - 1)*(NumOfFrame) ;			//比较特殊，第一帧和最后一帧是剪裁了的
	int nELblock = (NumOfControlPoint - 1)*(NumOfFrame - 1);
	int nECblock = 2 * (NumOfControlPoint - 1)*(NumOfFrame - 1);
	int nEIblock = (NumOfControlPoint - 1)*(NumOfFrame - 1);
	int NumOfVerticalBlock = nEVblock + nEGblock + nELblock + nECblock + nEIblock;

	JacBlockMat.Create(NumOfVerticalBlock,NumOfParam,xLenght,NumOfParam);


	int StartBlockRow = 0;
	int StartBlockCol = 0;
	int RowOffset = 0;
	int NumOfRowsPerFrame = (SAMPLESPERCURVE + 1) * (NumOfControlPoint - 2) + (SAMPLESPERCURVE + 2);
	//EVx1
	for (int f = 1;f <= NumOfFrame - 2; f++)
	{
		SetFrameBlock(RowOffset,StartBlockRow,StartBlockCol,NumOfFrame,NumOfControlPoint,SAMPLESPERCURVE + 1,SAMPLESPERCURVE + 2,JacBlockMat);
		StartBlockRow += NumOfControlPoint - 1;
		StartBlockCol += NumOfControlPoint * 4;
		RowOffset += NumOfRowsPerFrame;
	}

	//EVx2
	StartBlockRow = 0 + NumOfControlPoint - 1;
	StartBlockCol = 0;
	RowOffset = NumOfRowsPerFrame;	
	for (int f = 1;f <= NumOfFrame - 2; f++)
	{
		SetFrameBlock(RowOffset,StartBlockRow,StartBlockCol,NumOfFrame,NumOfControlPoint,SAMPLESPERCURVE + 1,SAMPLESPERCURVE + 2,JacBlockMat);
		StartBlockRow += NumOfControlPoint - 1;
		StartBlockCol += NumOfControlPoint * 4;
		RowOffset += NumOfRowsPerFrame;
	}

	//EVy1
	StartBlockRow = (NumOfControlPoint - 1)*(NumOfFrame - 1) ;
	StartBlockCol = 0;
	RowOffset = NumOfRowsPerFrame * (NumOfFrame -1);
	for (int f = 1;f <= NumOfFrame - 2; f++)
	{
		SetFrameBlock(RowOffset,StartBlockRow,StartBlockCol,NumOfFrame,NumOfControlPoint,SAMPLESPERCURVE + 1,SAMPLESPERCURVE + 2,JacBlockMat);
		StartBlockRow += NumOfControlPoint - 1;
		StartBlockCol += NumOfControlPoint * 4;
		RowOffset += NumOfRowsPerFrame;
	}

	//EVy2
	StartBlockRow = (NumOfControlPoint - 1)*(NumOfFrame - 1) + NumOfControlPoint - 1;
	StartBlockCol = 0;
	RowOffset = NumOfRowsPerFrame * (NumOfFrame -1) + NumOfRowsPerFrame;
	for (int f = 1;f <= NumOfFrame - 2; f++)
	{
		SetFrameBlock(RowOffset,StartBlockRow,StartBlockCol,NumOfFrame,NumOfControlPoint,SAMPLESPERCURVE + 1,SAMPLESPERCURVE + 2,JacBlockMat);
		StartBlockRow += NumOfControlPoint - 1;
		StartBlockCol += NumOfControlPoint * 4;
		RowOffset += NumOfRowsPerFrame;
	}


	//EG
	StartBlockRow = nEVblock + NumOfControlPoint - 1;
	StartBlockCol = 0;
	RowOffset = adata->nEV + NumOfRowsPerFrame;
	for (int f = 1;f <= NumOfFrame - 2; f++)
	{
		SetFrameBlock(RowOffset,StartBlockRow,StartBlockCol,NumOfFrame,NumOfControlPoint,SAMPLESPERCURVE + 1,SAMPLESPERCURVE + 2,JacBlockMat);
		StartBlockRow += NumOfControlPoint - 1;
		StartBlockCol += NumOfControlPoint * 4;
		RowOffset += NumOfRowsPerFrame;
	}


	//EL
	NumOfRowsPerFrame =  (SAMPLESPERCURVE + 1) * (NumOfControlPoint - 2) + SAMPLESPERCURVE + 1;
	StartBlockRow = nEVblock + nEGblock;
	StartBlockCol = 0;
	RowOffset = adata->nEV + adata->nEG;
	//EL 1
	for (int f = 1;f <= NumOfFrame - 2; f++)
	{
		SetFrameBlock(RowOffset,StartBlockRow,StartBlockCol,NumOfFrame,NumOfControlPoint,SAMPLESPERCURVE + 1,SAMPLESPERCURVE + 1,JacBlockMat);
		StartBlockRow += NumOfControlPoint - 1;
		StartBlockCol += NumOfControlPoint * 4;
		RowOffset += NumOfRowsPerFrame;
	}

	//EL 2
	StartBlockRow = nEVblock + nEGblock + NumOfControlPoint - 1;
	StartBlockCol = 0;
	RowOffset = adata->nEG +adata->nEV + NumOfRowsPerFrame;
	for (int f = 1;f <= NumOfFrame - 2; f++)
	{
		SetFrameBlock(RowOffset,StartBlockRow,StartBlockCol,NumOfFrame,NumOfControlPoint,SAMPLESPERCURVE + 1,SAMPLESPERCURVE + 1,JacBlockMat);
		StartBlockRow += NumOfControlPoint - 1;
		StartBlockCol += NumOfControlPoint * 4;
		RowOffset += NumOfRowsPerFrame;
	}


	//EC
	NumOfRowsPerFrame =  (SAMPLESPERCURVE + 1) * (NumOfControlPoint - 2) + SAMPLESPERCURVE;
	StartBlockRow = nEVblock + nEGblock + nELblock;
	StartBlockCol = 0;
	RowOffset = adata->nEV + adata->nEG + adata->nEL;
	//ECx1
	for (int f = 1;f <= NumOfFrame - 2; f++)
	{
		SetFrameBlockSpecialEC(RowOffset,StartBlockRow,StartBlockCol,NumOfFrame,NumOfControlPoint,SAMPLESPERCURVE + 1,SAMPLESPERCURVE,JacBlockMat);
		StartBlockRow += NumOfControlPoint - 1;
		StartBlockCol += NumOfControlPoint * 4;
		RowOffset += NumOfRowsPerFrame;
	}

	StartBlockRow = nEVblock + nEGblock + nELblock + NumOfControlPoint - 1;
	StartBlockCol = 0;
	RowOffset = adata->nEV + adata->nEG + adata->nEL + NumOfRowsPerFrame;
	//ECx2
	for (int f = 1;f <= NumOfFrame - 2; f++)
	{
		SetFrameBlockSpecialEC(RowOffset,StartBlockRow,StartBlockCol,NumOfFrame,NumOfControlPoint,SAMPLESPERCURVE + 1,SAMPLESPERCURVE,JacBlockMat);
		StartBlockRow += NumOfControlPoint - 1;
		StartBlockCol += NumOfControlPoint * 4;
		RowOffset += NumOfRowsPerFrame;
	}

	//ECy1
	StartBlockRow =  nEVblock + nEGblock + nELblock + (NumOfControlPoint - 1) * (NumOfFrame -1) ;
	StartBlockCol = 0;
	RowOffset = adata->nEV + adata->nEG + adata->nEL + NumOfRowsPerFrame * (NumOfFrame - 1);
	for (int f = 1;f <= NumOfFrame - 2; f++)
	{
		SetFrameBlockSpecialEC(RowOffset,StartBlockRow,StartBlockCol,NumOfFrame,NumOfControlPoint,SAMPLESPERCURVE + 1,SAMPLESPERCURVE,JacBlockMat);
		StartBlockRow += NumOfControlPoint - 1;
		StartBlockCol += NumOfControlPoint * 4;
		RowOffset += NumOfRowsPerFrame;
	}

	//ECy2
	StartBlockRow = nEVblock + nEGblock + nELblock + (NumOfControlPoint - 1) * (NumOfFrame -1) + NumOfControlPoint - 1;
	StartBlockCol = 0;
	RowOffset = adata->nEV + adata->nEG + adata->nEL + NumOfRowsPerFrame * NumOfFrame;
	for (int f = 1;f <= NumOfFrame - 2; f++)
	{
		SetFrameBlockSpecialEC(RowOffset,StartBlockRow,StartBlockCol,NumOfFrame,NumOfControlPoint,SAMPLESPERCURVE + 1,SAMPLESPERCURVE,JacBlockMat);
		StartBlockRow += NumOfControlPoint - 1;
		StartBlockCol += NumOfControlPoint * 4;
		RowOffset += NumOfRowsPerFrame;
	}



	//EI
	NumOfRowsPerFrame =  3*(((2*K+1)*(SAMPLESPERCURVE + 1)) * (NumOfControlPoint - 2) + (2*K+1)*(SAMPLESPERCURVE + 2));
	StartBlockRow = nEVblock + nEGblock + nECblock + nELblock;
	StartBlockCol = 0;
	RowOffset = adata->nEV + adata->nEG + adata->nEL + adata->nEC;
	//EI 1
	for (int f = 1;f <= NumOfFrame - 2; f++)
	{
		SetFrameBlock(RowOffset,StartBlockRow,StartBlockCol,NumOfFrame,NumOfControlPoint,3*(2*K+1)*(SAMPLESPERCURVE + 1),3*(2*K+1)*(SAMPLESPERCURVE + 2),JacBlockMat);
		StartBlockRow += NumOfControlPoint - 1;
		StartBlockCol += NumOfControlPoint * 4;
		RowOffset += NumOfRowsPerFrame;
	}

	//EI 2
	StartBlockRow = nEVblock + nEGblock + nECblock + nELblock + NumOfControlPoint - 1;
	StartBlockCol = 0;
	RowOffset = adata->nEV + adata->nEG + adata->nEL + adata->nEC + NumOfRowsPerFrame;
	for (int f = 1;f <= NumOfFrame - 2; f++)
	{
		SetFrameBlock(RowOffset,StartBlockRow,StartBlockCol,NumOfFrame,NumOfControlPoint,3*(2*K+1)*(SAMPLESPERCURVE + 1),3*(2*K+1)*(SAMPLESPERCURVE + 2),JacBlockMat);
		StartBlockRow += NumOfControlPoint - 1;
		StartBlockCol += NumOfControlPoint * 4;
		RowOffset += NumOfRowsPerFrame;
	}


	//JacBlockMat.Visualize();
}