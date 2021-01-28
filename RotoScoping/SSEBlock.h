#ifndef SSEBLOCK_H
#define SSEBLOCK_H
#include <xmmintrin.h>
#include <emmintrin.h>


class SSEBlock
{
public:
	int nRow;
	__declspec(align(16)) float* Data;	
	int fromIndex;
	int toIndex;
	void SetValue(int index,float value);
	SSEBlock(int nRow);
	~SSEBlock();
protected:
private:
};


#endif