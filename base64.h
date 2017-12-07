#ifndef BASE64_H_
#define BASE64_H_

#include <stddef.h>

/*
 * The output buffer size must be enough to fit the base64 representation of
 * the input buffer. Their lengths are related. For an input buffer of N bytes,
 * the output buffer needs at least (N+2)/3*4 + 1 bytes. We're supposing
 * integer division, and the extra byte accounts for the string terminator.
 */
void bcrypt_base64_calc(const unsigned char *in, size_t len, char *out);

#endif
