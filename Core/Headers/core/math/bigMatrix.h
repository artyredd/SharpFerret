#pragma once

#include "core/array.h"

struct _bigMatrix
{
	array(float) Values;
	// The Height of the matrix
	ulong Rows;
	// The Width of the matrix
	ulong Columns;
};

typedef struct _bigMatrix* BigMatrix;

extern const struct _arbitraryMatrixMethods
{
	BigMatrix(*Create)(ulong rows, ulong columns);
	// Multiplies the big matrix with the given vector and appends the results to the destinationVector
	void (*MultiplyVector)(BigMatrix, array(float) vector, array(float) destinationVector);
	void (*Resize)(BigMatrix, ulong rows, ulong columns);
	float* (*At)(BigMatrix, ulong row, ulong column);
	void (*Clear)(BigMatrix);
	void (*Dispose)(BigMatrix);
} BigMatrices;