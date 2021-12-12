#pragma once

#include "csharp.h"
#include "singine/file.h"

/// <summary>
/// The error stream that gaurd clauses should output to
/// </summary>
#define GaurdClauseOutputstream stderr

// throwing guard clauses

// ensures the variable != 0 and if it's zero throws an exception
#define EnsureNotNull(variableName) if(variableName is null){\
	fprintf(GaurdClauseOutputstream,#variableName" can not be null (NULL)");\
	throw(InvalidArgumentException);}
#define EnsureNotZero(variableName) if(variableName != 0){\
	fprintf(GaurdClauseOutputstream,#variableName" can not be zero (0)");\
	throw(InvalidArgumentException);}
#define Ensure(variableName) if(variableName != true){\
	fprintf(GaurdClauseOutputstream,#variableName" must be true (1)");\
	throw(InvalidArgumentException);}

// bool ensures (non-throwing)

// ensures the variable != 0 and if it's zero returns false
#define BoolEnsureNotZero(variableName) if(variableName is 0) { return false; }
// ensures the variable != NULL and it's NULL returns false
#define BoolEnsureNotNull(variableName) if(variableName is null) {return false; }
// ensures the variable is true otherwise returns false
#define BoolEnsure(variableName) if(variableName is false) { return false; }