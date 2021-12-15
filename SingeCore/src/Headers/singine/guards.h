#pragma once

#include "csharp.h"
#include "singine/file.h"

/// <summary>
/// The error stream that gaurd clauses should output to
/// </summary>
#define GaurdClauseOutputstream stderr

// throwing guard clauses

#define ThrowGuard(variableName, message) fprintf(GaurdClauseOutputstream,#variableName message); throw(InvalidArgumentException);

// ensures the variable != 0 and if it's zero throws an exception
#define GuardNotNull(variableName) if(variableName is null){ ThrowGuard(variableName," can not be null"); }
#define GuardNotZero(variableName) if(variableName != 0){ ThrowGuard(variableName," can not be zero"); }
#define Guard(variableName) if(variableName != true){ ThrowGuard(variableName," can not be false"); }

// bool ensures (non-throwing)

// ensures the variable != 0 and if it's zero returns false
#define BoolGuardNotZero(variableName) if(variableName is 0) { return false; }
// ensures the variable != NULL and it's NULL returns false
#define BoolGuardNotNull(variableName) if(variableName is null) {return false; }
// ensures the variable is true otherwise returns false
#define BoolGuard(variableName) if(variableName is false) { return false; }