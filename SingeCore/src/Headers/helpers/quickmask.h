#pragma once

/* forgive me */

#define SetFlag(mask,flag,value) if(value){ mask |= flag; }else{ mask ^= flag; }
#define HasFlag(mask,flag) (((mask) & (flag)) == (flag))
#define ResetFlags(mask) mask = 0

// 1 (0b_0000_0000_0000_0001)
#define FLAG_0 (1<<0)

// 2 (0b_0000_0000_0000_0010)
#define FLAG_1 (1<<1)

// 4 (0b_0000_0000_0000_0100)
#define FLAG_2 (1<<2)

// 8 (0b_0000_0000_0000_1000)
#define FLAG_3 (1<<3)

// 16 (0b_0000_0000_0001_0000)
#define FLAG_4 (1<<4)

// 32 (0b_0000_0000_0010_0000)
#define FLAG_5 (1<<5)

// 64 (0b_0000_0000_0100_0000)
#define FLAG_6 (1<<6)

// 128 (0b_0000_0000_1000_0000)
#define FLAG_7 (1<<7)

// 256 (0b_0000_0001_0000_0000)
#define FLAG_8 (1<<8)

// 512 (0b_0000_0010_0000_0000)
#define FLAG_9 (1<<9)

// 1024 (0b_0000_0100_0000_0000)
#define FLAG_10 (1<<10)

// 2048 (0b_0000_1000_0000_0000)
#define FLAG_11 (1<<11)

// 4096 (0b_0001_0000_0000_0000)
#define FLAG_12 (1<<12)

// 8192 (0b_0010_0000_0000_0000)
#define FLAG_13 (1<<13)

// 16384 (0b_0100_0000_0000_0000)
#define FLAG_14 (1<<14)

// 32768 (0b_1000_0000_0000_0000)
#define FLAG_15 (1<<15)

#define FLAG_ALL 0xFFFF