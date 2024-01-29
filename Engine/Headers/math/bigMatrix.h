#pragma once

#include "singine/array.h"

struct _bigMatrix
{
	ARRAY(float) Values;
	// The Height of the matrix
	size_t Rows;
	// The Width of the matrix
	size_t Columns;
};

typedef struct _bigMatrix* BigMatrix;

extern const struct _arbitraryMatrixMethods
{
	BigMatrix(*Create)(size_t rows, size_t columns);
	// Multiplies the big matrix with the given vector and appends the results to the destinationVector
	void (*MultiplyVector)(BigMatrix, ARRAY(float) vector, ARRAY(float) destinationVector);
	void (*Resize)(BigMatrix, size_t rows, size_t columns);
	float* (*At)(BigMatrix, size_t row, size_t column);
	void (*Clear)(BigMatrix);
	void (*Dispose)(BigMatrix);
} BigMatrices;