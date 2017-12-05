#ifndef SHA512_H_
#define SHA512_H_

/* sum must have space for at least 512 bits. */
void sha512_calc(const void *in, unsigned long len, char *sum);

#endif
