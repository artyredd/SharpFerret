#pragma once

#include <stdint.h>

typedef uint16_t intMask;

#define QUICKMASK_TYPE (intMask)

#define SetFlag(mask,flag) ((mask) |= (flag))
#define ClearFlag(mask,flag) ((mask) ^= (flag))
#define AssignFlag(mask,flag,value) if(value){ SetFlag(mask,flag); }else{ ClearFlag(mask,flag); }
#define HasFlag(mask,flag) (((mask) & (flag)) == (flag))
#define ResetFlags(mask) (mask = 0)

// 1 (0b_0000_0000_0000_0001)
#define FLAG_0 QUICKMASK_TYPE(1<<0)

// 2 (0b_0000_0000_0000_0010)
#define FLAG_1 QUICKMASK_TYPE(1<<1)

// 4 (0b_0000_0000_0000_0100)
#define FLAG_2 QUICKMASK_TYPE(1<<2)

// 8 (0b_0000_0000_0000_1000)
#define FLAG_3 QUICKMASK_TYPE(1<<3)

// 16 (0b_0000_0000_0001_0000)
#define FLAG_4 QUICKMASK_TYPE(1<<4)

// 32 (0b_0000_0000_0010_0000)
#define FLAG_5 QUICKMASK_TYPE(1<<5)

// 64 (0b_0000_0000_0100_0000)
#define FLAG_6 QUICKMASK_TYPE(1<<6)

// 128 (0b_0000_0000_1000_0000)
#define FLAG_7 QUICKMASK_TYPE(1<<7)

// 256 (0b_0000_0001_0000_0000)
#define FLAG_8 QUICKMASK_TYPE(1<<8)

// 512 (0b_0000_0010_0000_0000)
#define FLAG_9 QUICKMASK_TYPE(1<<9)

// 1024 (0b_0000_0100_0000_0000)
#define FLAG_10 QUICKMASK_TYPE(1<<10)

// 2048 (0b_0000_1000_0000_0000)
#define FLAG_11 QUICKMASK_TYPE(1<<11)

// 4096 (0b_0001_0000_0000_0000)
#define FLAG_12 QUICKMASK_TYPE(1<<12)

// 8192 (0b_0010_0000_0000_0000)
#define FLAG_13 QUICKMASK_TYPE(1<<13)

// 16384 (0b_0100_0000_0000_0000)
#define FLAG_14 QUICKMASK_TYPE(1<<14)

// 32768 (0b_1000_0000_0000_0000)
#define FLAG_15 QUICKMASK_TYPE(1<<15)

#define FLAG_ALL (intMask)0xFFFF

#define FlagN(n) QUICKMASK_TYPE(1<<n)
