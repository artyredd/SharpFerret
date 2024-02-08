#pragma once

#include "core/math/bigMatrix.h"

private BigMatrix Create(size_t rows, size_t columns);
// Multiplies the big matrix with the given vector and appends the results to the destinationVector
private void MultiplyVector(BigMatrix, array(float) vector, array(float) destinationVector);
private void Dispose(BigMatrix);
private void Resize(BigMatrix, size_t rows, size_t columns);
private void Clear(BigMatrix);
private float* At(BigMatrix, size_t row, size_t column);

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

private BigMatrix Create(size_t rows, size_t columns)
{
	REGISTER_TYPE(BigMatrix);
	BigMatrix result = Memory.Alloc(sizeof(struct _bigMatrix), BigMatrixTypeId);

	result->Values = Arrays(float).Create(rows * columns);
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

	for (size_t row = 0; row < matrix->Rows; row++)
	{
		float value = 0.0f;
		for (size_t i = 0; i < vector->Count; i++)
		{
			value += matrix->Values->Values[(row * matrix->Columns) + i] * vector->Values[i];
		}

		Arrays(float).Append(destinationVector, value);
	}
}

private void Dispose(BigMatrix matrix)
{
	Arrays(float).Dispose(matrix->Values);

	Memory.Free(matrix, BigMatrixTypeId);
}

private void Resize(BigMatrix matrix, size_t rows, size_t columns)
{
	matrix->Rows = rows;
	matrix->Columns = columns;
	Arrays(float).Resize(matrix->Values, rows * columns);
}

private void Clear(BigMatrix matrix)
{
	Arrays(float).Clear(matrix->Values);
}

private float* At(BigMatrix matrix, size_t row, size_t column)
{
	return Arrays(float).At(matrix->Values, (row * matrix->Columns) + column);
}