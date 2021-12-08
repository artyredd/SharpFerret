#pragma once

typedef const int Exception;

//.NET Foundation licenses this [value] to you under the MIT license. MS Exception values used for interop
// Failed to allocate enough memory for a new pointer
Exception OutOfMemoryException = 0x8007000E;
/// One or more arguments were null or invalid
Exception InvalidArgumentException = 0x80070057;
/// The range provided was out of bounds of the array or linked list
Exception IndexOutOfRangeException = 0x80131508;
// Some feature is not implemented
Exception NotImplementedException = 0x80004001;
// The provided file was not found
Exception FileNotFoundException = 0x80070002;
// end license

// custom exceptions
// end custom exceptions