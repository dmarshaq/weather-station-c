#ifndef CORE_H
#define CORE_H

#define TODO(message)                           printf_err("%s:%d TODO: %s\n", __FILE__, __LINE__, message); exit(1)

/**
 * Integer typedefs.
 */

#include <stdint.h>

typedef int8_t      s8;
typedef uint8_t	    u8;
typedef int16_t     s16;
typedef uint16_t    u16;
typedef int32_t     s32;
typedef uint32_t    u32;
typedef int64_t     s64;
typedef uint64_t    u64;


#endif
