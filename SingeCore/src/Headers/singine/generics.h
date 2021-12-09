typedef int TypeCode;

/// <summary>
/// Structure containing the available generic types
/// </summary>
static const struct Types {
	/// <summary>
	/// 0: No Specified Type
	/// </summary>
	TypeCode none;
	/// <summary>
	/// 1: Pointer (void*)
	/// </summary>
	TypeCode Pointer;
	/// <summary>
	/// 2: size_t
	/// </summary>
	TypeCode SizeT;
	/// <summary>
	/// 4: char
	/// </summary>
	TypeCode Char;
	/// <summary>
	/// 8: unsigned char
	/// </summary>
	TypeCode UnsignedChar;
	/// <summary>
	/// 16: int
	/// </summary>
	TypeCode Int;
	/// <summary>
	/// 32: unsigned int
	/// </summary>
	TypeCode UnsignedInt;
	/// <summary>
	/// 64: long int
	/// </summary>
	TypeCode Long;
	/// <summary>
	/// 128: unsigned long int
	/// </summary>
	TypeCode UnsignedLong;
} TypeCodes = {
	0,
	1 << 0,
	1 << 1,
	1 << 2,
	1 << 3,
	1 << 4,
	1 << 5,
	1 << 6,
	1 << 7,
};

typedef union _generic* Generic;

union _generic {
	/// <summary>
	/// Retrieves the value of this generic as a generic pointer (void*)
	/// </summary>
	void* AsPointer;
	/// <summary>
	/// Retrieves the value of this generic as an unsigned char
	/// </summary>
	unsigned char AsUnsignedChar;
	/// <summary>
	/// Retrieves the value of this generic as a char
	/// </summary>
	char AsChar;
	/// <summary>
	/// Retrieves the value of this generic as an int
	/// </summary>
	int AsInt;
	/// <summary>
	/// Retrieves the value of this generic as an unsigned int
	/// </summary>
	unsigned int AsUnsignedInt;
	/// <summary>
	/// Retrieves the value of this generic as a float
	/// </summary>
	float AsFloat;
	/// <summary>
	/// Retrieves the value of this generic as a size_t
	/// </summary>
	size_t AsSizeT;
};