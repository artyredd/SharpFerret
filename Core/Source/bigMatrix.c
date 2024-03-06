#pragma once

#include "core/math/bigMatrix.h"

private BigMatrix Create(ulong rows, ulong columns);
// Multiplies the big matrix with the given vector and appends the results to the destinationVector
private void MultiplyVector(BigMatrix, array(float) vector, array(float) destinationVector);
private void Dispose(BigMatrix);
private void Resize(BigMatrix, ulong rows, ulong columns);
private void Clear(BigMatrix);
private float* At(BigMatrix, ulong row, ulong column);

const struct _arbitraryMatrixMethods BigMatrices =
{
	.Create = Create,
	.MultiplyVector = MultiplyVector,
	.Dispose = Dispose,
	.Clear = Clear,
	.Resize = Resize,
	.At = At
};

DEFINE_TYPE_ID(BigMatrix);

private BigMatrix Create(ulong rows, ulong columns)
{
	REGISTER_TYPE(BigMatrix);
	BigMatrix result = Memory.Alloc(sizeof(struct _bigMatrix), BigMatrixTypeId);

	result->Values = arrays(float).Create(rows * columns);
	result->Rows = rows;
	result->Columns = columns;

	return result;
}

// Multiplies the big matrix with the given vector and appends the results to the destinationVector
private void MultiplyVector(BigMatrix matrix, array(float) vector, array(float) destinationVector)
{
	if (vector->Count > matrix->Columns)
	{
		// dont make no sense ill tell ya hwhat
		throw(InvalidArgumentException);
	}

	for (ulong row = 0; row < matrix->Rows; row++)
	{
		float value = 0.0f;
		for (ulong i = 0; i < vector->Count; i++)
		{
			value += at(matrix->Values, (row * matrix->Columns) + i) * at(vector, i);
		}

		arrays(float).Append(destinationVector, value);
	}
}

private void Dispose(BigMatrix matrix)
{
	arrays(float).Dispose(matrix->Values);

	Memory.Free(matrix, BigMatrixTypeId);
}

private void Resize(BigMatrix matrix, ulong rows, ulong columns)
{
	matrix->Rows = rows;
	matrix->Columns = columns;
	arrays(float).Resize(matrix->Values, rows * columns);
}

private void Clear(BigMatrix matrix)
{
	arrays(float).Clear(matrix->Values);
}

private float* At(BigMatrix matrix, ulong row, ulong column)
{
	return arrays(float).At(matrix->Values, (row * matrix->Columns) + column);
}