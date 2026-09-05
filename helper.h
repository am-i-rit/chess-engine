#pragma once

#include <cstdint>
#include <cstring>


// function to return the index of a 64-bit unsigned integer's least significant bit
inline std::uint8_t lsbIndex(std::uint64_t x)
{
    if (!x) return 64;
    static const int index[64] = 
    {
        0,  1, 48,  2, 57, 49, 28,  3,
        61, 58, 50, 42, 38, 29, 17,  4,
        62, 55, 59, 36, 53, 51, 43, 22,
        45, 39, 33, 30, 24, 18, 12,  5,
        63, 47, 56, 27, 60, 41, 37, 16,
        54, 35, 52, 21, 44, 32, 23, 11,
        46, 26, 40, 15, 34, 20, 31, 10,
        25, 14, 19,  9, 13,  8,  7,  6
    };
    static const std::uint64_t debruijn = 0x03f79d71b4cb0a89;
    return index[((x & -(std::int64_t)x) * debruijn) >> 58];
}

// function to return the index of a 64-bit unsigned integer's most significant bit
inline std::uint8_t msbIndex(std::uint64_t x)
{
    if (!x) return 64;
    double d = (double)x;
    std::uint64_t bits;
    memcpy(&bits, &d, sizeof(bits));
    return ((bits >> 52) & 0x7FF) - 1023;
}

// popcount function
inline std::uint8_t popcount(uint64_t x)
{
    int count = 0;
	while (x)
	{
		x &= x - 1;
		++count;
	}
	return count;
}