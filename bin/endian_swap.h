#pragma once

#if defined(__APPLE__)

#include <libkern/OSByteOrder.h>

#define htob16(x) OSSwapHostToBigInt16(x)
#define htob32(x) OSSwapHostToBigInt32(x)
#define htob64(x) OSSwapHostToBigInt64(x)
#define btoh16(x) OSSwapBigToHostInt16(x)
#define btoh32(x) OSSwapBigToHostInt32(x)
#define btoh64(x) OSSwapBigToHostInt64(x)

#else

#include <endian.h>
#include <byteswap.h>

#ifndef htob16
#define htob16(x) bswap_16(x)
#endif

#ifndef htob32
#define htob32(x) bswap_32(x)
#endif

#ifndef htob64
#define htob64(x) bswap_64(x)
#endif

#ifndef btoh16
#define btoh16(x) bswap_16(x)
#endif

#ifndef btoh32
#define btoh32(x) bswap_32(x)
#endif

#ifndef btoh64
#define btoh64(x) bswap_64(x)
#endif

#endif
