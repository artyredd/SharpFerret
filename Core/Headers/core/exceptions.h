#pragma once

/*
				EXCEPTION FILE
	KEY:
		0x00000000 NORMAL OPERATION

	ERROR TIERS:
		0x5.......	OPEN_GL SPECIFIC
		0x6.......	GLFW SPECIFIC
		0x7.......	APPLICATION SPECIFIC
		0x8.......	MICROSOFT STANDARD
		0xDEADADD.  CRITICAL; DEAD ADDRESS; EITHER PROGRAMMER OR SECURITY THREAT
		0xDEADBEEF  RESERVED
*/

#ifndef _exceptions_h_
#define _exceptions_h_
#endif // !_exceptions_h_

typedef const int Exception;

//.NET Foundation licenses this [value] to you under the MIT license. MS Exception values used for interop
// Failed to allocate enough memory for a new pointer
static Exception OutOfMemoryException = 0x8007000E;
/// One or more arguments were null or invalid
static Exception InvalidArgumentException = 0x80070057;
/// The range provided was out of bounds of the array or linked list
static Exception IndexOutOfRangeException = 0x80131508;
// Some feature is not implemented
static Exception NotImplementedException = 0x80004001;
// The provided file was not found
static Exception FileNotFoundException = 0x80070002;
// end license

// custom exceptions

// CRITICAL: A NULL reference was encountered
// this is a critical error that should never be encountered
// most notably thrown by allocation, memory, or disposal methods
static Exception NullReferenceException = 0xDEADADD0;

// A possible memory leak was detected
static Exception MemoryLeakException = 0x70000001;
static Exception FailedToReadFileException = 0x70000002;
static Exception FailedToCloseFileException = 0x70000003;
static Exception FailedToOpenFileException = 0x70000004;
static Exception FailedToWriteToStreamException = 0x70000005;
static Exception FailedToImportModelException = 0x70000006;
static Exception MissingCharacterException = 0x70000007;
static Exception FailedToSerializeException = 0x70000008;
static Exception NoAvailableObjectInPoolException = 0x70000009;
static Exception ItemNotFoundInCollectionException = 0x7000000A;
// Attempted to free, modify or change a stack variable inaparopriately
static Exception StackObjectModifiedException = 0x7000000B;
// Attempted to perform an operation with two objects that are incompatible sizes in bytes
static Exception TypeMismatchException = 0x7000000C;
static Exception FailedToMoveMemoryException = 0x7000000D;

// The logic path or branching that lead to this error needs to be evaluated or tested more thoroughly.
// This error denotes and exception that may or may not be dangerous but is unexpected and clearly unintended.
static Exception InvalidLogicException = 0x70000008;
// A texture failed to load from a texture definition file
static Exception FailedToLoadTextureException = 0x70000009;
// The outcome where this error is thrown should never occur even by chance, either an edge case was missed or something else weird happend
static Exception UnexpectedOutcomeException = 0x7FFFFFFF;
// end custom exceptions

// GLFW Exceptions

static Exception FailedToInitializeExceptionGLFW = 0x60000001;
static Exception NotIntializedExceptionGLFW = 0x60000002;
static Exception FailedToCreateWindowExceptionGLFW = 0x60000003;
static Exception FailedToGetWindowMonitorException = 0x60000004;
static Exception FailedToGetVideoModeException = 0x60000005;
static Exception NoActiveWindowException = 0x60000006;
// end GLFW Exceptions

// GL Exceptions
static Exception FailedToCompileShaderException = 0x50000001;
static Exception FailedToLocationMVPUniformException = 0x50000002;
static Exception FailedToBindMeshException = 0x50000003;

// Runtime Exceptions

// Programmer failed to set the scene before attempting to draw primitives using Drawing.Draw
static Exception FailedToSetSceneException = 0x40000001;

// thrown when the resolution of one object should match another object's but they do not
static Exception ResolutionMismatchException = 0x50000004;
//

