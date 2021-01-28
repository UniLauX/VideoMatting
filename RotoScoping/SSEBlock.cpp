#include "../RotoScoping/stdafx.h"
#include "../RotoScoping/SSEBlock.h"
#include <assert.h>



void SSEBlock::SetValue(int index,float value)
{
	assert(index < nRow);
	Data[index] = value;
}
SSEBlock::SSEBlock(int nRow)
{
	this->nRow = nRow;
	Data = (float *)_aligned_malloc(nRow * sizeof(float), 16);
}
SSEBlock::~SSEBlock()
{
	_aligned_free(Data);
}