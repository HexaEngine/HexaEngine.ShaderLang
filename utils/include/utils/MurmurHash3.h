//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

#ifndef _MURMURHASH3_H_
#define _MURMURHASH3_H_

#include "types.hpp"

void MurmurHash3_x86_32(const void* key, int len, uint32_t seed, uint32_t& out);

void MurmurHash3_x86_128(const void* key, int len, uint32_t seed, HEXA_UTILS_NAMESPACE::uint128_t& out);

void MurmurHash3_x64_128(const void* key, int len, uint32_t seed, HEXA_UTILS_NAMESPACE::uint128_t& out);

//-----------------------------------------------------------------------------

#endif // _MURMURHASH3_H_
