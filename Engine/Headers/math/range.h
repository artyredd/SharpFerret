#pragma once

/// <summary>
/// Pointer to range
/// </summary>
typedef struct range* Range;

/// <summary>
/// a struct range object
/// </summary>
typedef struct range range;

// Represents two double values, a minimum and a maximum
struct range {
	double Minimum;
	double Maximum;
};

struct _rangeMethods {
	// Creates a new range object on the heap with the provided values
	Range(*Create)(double minimum, double maxmimum);
	// Converts the provided range to a float array
	float* (*ToArray)(Range);
	// Disposes a range object that was added to the heap
	void(*Dispose)(Range);
};

const extern struct _rangeMethods Ranges;