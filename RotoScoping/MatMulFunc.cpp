#include "../RotoScoping/stdafx.h"
#include "../RotoScoping/MatMulFunc.h"
#include <xmmintrin.h>
#include <emmintrin.h>


float SSEVecMul(SSEBlock * AVec, SSEBlock * BVec)
{
	assert(AVec->nRow == BVec->nRow);
	
	
	float sum = 0.0;
	__m128 *Op1 = (__m128*)AVec->Data;
	__m128 *Op2 = (__m128*)BVec->Data;				//SSE的两个操作数
	__m128 ProductSum;


	int nSSEIte = AVec->nRow/4;						//需要进行SSE迭代的次数
	


	ProductSum = _mm_setr_ps(0.0f,0.0f,0.0f,0.0f);
	for (int SSEIte = 0; SSEIte < nSSEIte; SSEIte++)
	{
		ProductSum = _mm_add_ps( _mm_mul_ps(*Op1,*Op2), ProductSum);
		Op1++;
		Op2++;
	}
	sum += ProductSum.m128_f32[0] + ProductSum.m128_f32[1] + ProductSum.m128_f32[2] + ProductSum.m128_f32[3];


	//计算被4除余下的部分
	ProductSum = _mm_setr_ps(0.0f,0.0f,0.0f,0.0f);
	ProductSum = _mm_add_ps( _mm_mul_ps(*Op1,*Op2), ProductSum);
	for (int i=0 ; i<AVec->nRow%4 ; i++)
	{
		sum += ProductSum.m128_f32[i];
	}
	
	return sum;
}

//SSE版本
void JTJJacColSSE(
	BlockMatProxy& JacBlockMat,
	std::vector<int> *JacColNonZeroBlockRowIndex,
	int NumOfFrame, 
	int NumOfControlPoint,
	int NumOfParam,
	double * JTJ)			//深度优化
{
	int RH = NumOfParam;
	int RW = RH;

	printf("Begin JTJJacColSSE.\n");

	int NumOfParamPerFrame = 4 * NumOfControlPoint;
	double sum;
	SSEBlock ** ColR;
	SSEBlock ** ColC;

	//这里一切的block和strip都是相对于水平而言的
	// for the blocks before the last block
	for (int b = 0;b<NumOfFrame-3;b++)  
	{
		int ThisBlockOffset = NumOfParamPerFrame * b;
		int NextBlockOffset = ThisBlockOffset + NumOfParamPerFrame;

		// for all the strip before the last strip inside this block
		for (int s = 0 ; s < NumOfControlPoint - 1 ; s++) 
		{
			int ThisStripOffset = ThisBlockOffset + 4 * s;
			int NextStripOffset = NextBlockOffset + 4 * s;
			int RangeA = 12;				//一般来说，一列只需要与所在的strip中的12个列相乘，因为在EC那里是12个列内多有重叠的部分的。但是在最后一个曲线区间里面的点就没有12个列那么多了。
			int UnitLeftRange = -8;
			int UnitRightRange = 12 ;
			if (s == NumOfControlPoint - 2)
			{
				RangeA = 8;
				UnitRightRange = 8;
			}

			if (s == 0)
			{
				UnitLeftRange = 0;
			}
			else if (s == 1)
			{
				UnitLeftRange = -4;
			}

			for (int i = 0 ; i<4;i++)  // the columns of a strip
			{
				int r = ThisStripOffset + i;	//Jac的第r列，就是JTJ的第r行
				ColR = JacBlockMat.Cols[r];
				// the target column of the i th column, the next Range-i target within this unit
				for (int j = i;j<RangeA;j++)  
				{
					int c = ThisStripOffset + j;	// Jac的第c列,就是JTJ的第r行的第c个元素
					ColC = JacBlockMat.Cols[c];
					sum = 0.0;

					// row r of Jact mul col c of Jac , that is , col r of Jac mul col c of Jac
					for (int NonZeroRow = 0; NonZeroRow < JacColNonZeroBlockRowIndex[c].size(); NonZeroRow ++)
					{
						int RowIndex = JacColNonZeroBlockRowIndex[c][NonZeroRow];
						if (ColR[RowIndex] != NULL)
						{
							sum += SSEVecMul(ColR[RowIndex],ColC[RowIndex]);
						}
					}

					JTJ[r*RW + c] = sum;
					JTJ[c*RW + r] = sum;
				}		


				// Its target column, the unit inside next block 
				for (int j = UnitLeftRange; j<UnitRightRange ; j++)
				{
					int c = NextStripOffset + j;
					ColC = JacBlockMat.Cols[c];
					sum = 0.0;

					// row r of Jact mul col c of Jac , that is , col r of Jac mul col c of Jac
					for (int NonZeroRow = 0; NonZeroRow < JacColNonZeroBlockRowIndex[c].size(); NonZeroRow ++)
					{
						int RowIndex = JacColNonZeroBlockRowIndex[c][NonZeroRow];
						if (ColR[RowIndex] != NULL)
						{
							sum += SSEVecMul(ColR[RowIndex],ColC[RowIndex]);
						}
					}

					JTJ[r*RW + c] = sum;
					JTJ[c*RW + r] = sum;
				}



			}
		}

		// for the last strip inside this block
		int ThisStripOffset = ThisBlockOffset + 4 * (NumOfControlPoint - 1);
		int NextStripOffset = NextBlockOffset + 4 * (NumOfControlPoint - 1);			
		for (int i = 0 ; i<4;i++)  // the column of a strip
		{
			int r = ThisStripOffset + i;
			ColR = JacBlockMat.Cols[r];

			// its target column, the next 4-i target within this unit
			for (int j = i;j<4;j++)  
			{
				int c = ThisStripOffset + j;
				ColC = JacBlockMat.Cols[c];
				sum = 0.0;

				// row r of Jact mul col c of Jac , that is , col r of Jac mul col c of Jac
				for (int NonZeroRow = 0; NonZeroRow < JacColNonZeroBlockRowIndex[c].size(); NonZeroRow ++)
				{
					int RowIndex = JacColNonZeroBlockRowIndex[c][NonZeroRow];
					if (ColR[RowIndex] != NULL)
					{
						sum += SSEVecMul(ColR[RowIndex],ColC[RowIndex]);
					}
				}

				JTJ[r*RW + c] = sum;
				JTJ[c*RW + r] = sum;
			}		


			// Its target column, the unit inside next block
			for (int j = -8; j<4 ; j++)
			{
				int c = NextStripOffset + j;
				sum = 0.0;
				ColC = JacBlockMat.Cols[c];

				// row r of Jact mul col c of Jac , that is , col r of Jac mul col c of Jac
				for (int NonZeroRow = 0; NonZeroRow < JacColNonZeroBlockRowIndex[c].size(); NonZeroRow ++)
				{
					int RowIndex = JacColNonZeroBlockRowIndex[c][NonZeroRow];
					if (ColR[RowIndex] != NULL)
					{
						sum += SSEVecMul(ColR[RowIndex],ColC[RowIndex]);
					}
				}

				JTJ[r*RW + c] = sum;
				JTJ[c*RW + r] = sum;
			}
		}


	}




	// for the last block
	int ThisBlockOffset = NumOfParamPerFrame * (NumOfFrame - 3);
	// for all the strip before the last strip inside this block
	for (int s = 0;s<NumOfControlPoint - 1; s++) 
	{
		int ThisStripOffset = ThisBlockOffset + 4 * s;
		int RangeA = 12;
		if (s == NumOfControlPoint-2)
		{
			RangeA = 8;
		}
		for (int i = 0 ; i<4;i++)  // the column of a strip
		{
			int r = ThisStripOffset + i;
			ColR = JacBlockMat.Cols[r];

			// its target column, the next Range-i target within this unit
			for (int j = i;j<RangeA;j++)  
			{
				int c = ThisStripOffset + j;
				sum = 0.0;
				ColC = JacBlockMat.Cols[c];

				// row r of Jact mul col c of Jac , that is , col r of Jac mul col c of Jac
				for (int NonZeroRow = 0; NonZeroRow < JacColNonZeroBlockRowIndex[c].size(); NonZeroRow ++)
				{
					int RowIndex = JacColNonZeroBlockRowIndex[c][NonZeroRow];
					if (ColR[RowIndex] != NULL)
					{
						sum += SSEVecMul(ColR[RowIndex],ColC[RowIndex]);
					}
				}

				JTJ[r*RW + c] = sum;
				JTJ[c*RW + r] = sum;
			}		


			// There is no next block
		}
	}

	// for the last strip inside this block
	int ThisStripOffset = ThisBlockOffset + 4 * (NumOfControlPoint - 1);
	for (int i = 0 ; i<4;i++)  // the column of a strip
	{
		int r = ThisStripOffset + i;
		ColR = JacBlockMat.Cols[r];

		// its target column, the next 4-i target within this unit
		for (int j = i;j<4;j++)  
		{
			int c = ThisStripOffset + j;
			sum = 0.0;
			ColC = JacBlockMat.Cols[c];

			// row r of Jact mul col c of Jac , that is , col r of Jac mul col c of Jac
			for (int NonZeroRow = 0; NonZeroRow < JacColNonZeroBlockRowIndex[c].size(); NonZeroRow ++)
			{
				int RowIndex = JacColNonZeroBlockRowIndex[c][NonZeroRow];
				if (ColR[RowIndex] != NULL)
				{
					sum += SSEVecMul(ColR[RowIndex],ColC[RowIndex]);
				}
			}

			JTJ[r*RW + c] = sum;
			JTJ[c*RW + r] = sum;
		}		


		// There is no next block

	}

	printf("End JTJJacColSSE.\n");
}

