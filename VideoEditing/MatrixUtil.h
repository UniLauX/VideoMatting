#pragma once

#include "SparseMatrix.h"
#include "SparseMatrix_ListType.h"

void PrintSparseMatrix(CSparseMatrix* pMtx);

void PrintSparseMatrix(CSparseMatrix_ListType* pMtx);

void ScaleSparseMatrix(CSparseMatrix* pMtx, double scale);