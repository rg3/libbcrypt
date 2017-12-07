#ifndef KECCAK_H_
#define KECCAK_H_

typedef unsigned char u8;
typedef unsigned long long int u64;
typedef unsigned int ui;

/* out must have room for at least 512 bits. */
void bcrypt_sha3_512_calc(const u8 *in, u64 inLen, u8 *out);

#endif
