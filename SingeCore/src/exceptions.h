#pragma once

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

// A possible memory leak was detected
static Exception MemoryLeakException = 0x70000001;
// end custom exceptions