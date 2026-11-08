/*
 * bcrypt wrapper library
 *
 * Written in 2011, 2013, 2014, 2015, 2017 by Ricardo Garcia <r@rg3.name>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication along
 * with this software. If not, see
 * <http://creativecommons.org/publicdomain/zero/1.0/>.
 */
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#include "bcrypt.h"
#include "crypt_blowfish/ow-crypt.h"
#include "sha512.h"
#include "keccak.h"
#include "base64.h"

#define RANDBYTES (16)
#define BYTES_IN_512BITS (64)
#define BCRYPT_API __attribute__ ((visibility ("default")))

/*
 * Domain separation tags for the salted pre-hash functions. Changing them
 * would make existing hashes impossible to verify.
 */
#define PREHASH_SHA512_TAG "libbcrypt-sha512-v1:"
#define PREHASH_SHA3_512_TAG "libbcrypt-sha3-512-v1:"

/* Position of the salt characters in a bcrypt salt or hash ("$2b$NN$..."). */
#define SALT_OFFSET (7)
#define SALT_LENGTH (22)

enum prehash_alg {
	PREHASH_SHA512,
	PREHASH_SHA3_512
};

/*
 * This is a best effort implementation. Nothing prevents a compiler from
 * optimizing this function and making it vulnerable to timing attacks, but
 * this method is commonly used in crypto libraries like NaCl.
 *
 * Return value is zero if both strings are equal and nonzero otherwise.
*/
static int timing_safe_strcmp(const char *str1, const char *str2)
{
	const unsigned char *a;
	const unsigned char *b;
	int ret;
	int i;

	int len1 = strlen(str1);
	int len2 = strlen(str2);

	/* In our context both strings should always have the same length
	 * because they will be hashed passwords. */
	if (len1 != len2)
		return 1;

	/* Force unsigned for bitwise operations. */
	a = (const unsigned char *)str1;
	b = (const unsigned char *)str2;

	ret = 0;
	for (i = 0; i < len1; ++i)
		ret |= (a[i] ^ b[i]);

	return ret;
}

BCRYPT_API int bcrypt_gensalt(int factor, char salt[BCRYPT_HASHSIZE])
{
	int ret;
	char input[RANDBYTES];
	int workf;
	char *aux;

	/* Get random bytes. */
	ret = getentropy(input, sizeof(input));
	if (ret != 0)
	    return 1;

	/* Generate salt. */
	workf = (factor < 4 || factor > 31)?BCRYPT_DEFAULT_WORK_FACTOR:factor;
	aux = crypt_gensalt_rn("$2b$", workf, input, RANDBYTES,
			       salt, BCRYPT_HASHSIZE);
	return (aux == NULL)?2:0;
}

BCRYPT_API int bcrypt_hashpw(const char *passwd, const char *salt, char hash[BCRYPT_HASHSIZE])
{
	char *aux;
	aux = crypt_rn(passwd, salt, hash, BCRYPT_HASHSIZE);
	return (aux == NULL)?1:0;
}

BCRYPT_API int bcrypt_checkpw(const char *passwd, const char *hash)
{
	int ret;
	char outhash[BCRYPT_HASHSIZE];

	ret = bcrypt_hashpw(passwd, hash, outhash);
	if (ret != 0)
		return -1;

	return timing_safe_strcmp(hash, outhash);
}

/*
 * Calculates the base64 representation of the SHA-512 or SHA3-512 digest of
 * the given data. Returns zero on success and nonzero otherwise.
 */
static int prehash(enum prehash_alg alg, const void *in, size_t len,
		   char digest[BCRYPT_512BITS_BASE64_SIZE])
{
	unsigned char bindigest[BYTES_IN_512BITS];

	if (alg == PREHASH_SHA512) {
#if SIZE_MAX > ULONG_MAX
		if (len > ULONG_MAX)
			return 1;
#endif
		bcrypt_sha512_calc(in, (unsigned long)len, (char *)bindigest);
	} else {
#if SIZE_MAX > ULLONG_MAX
		if (len > ULLONG_MAX)
			return 1;
#endif
		bcrypt_sha3_512_calc((const u8 *)in, (u64)len, bindigest);
	}

	bcrypt_base64_calc(bindigest, sizeof(bindigest), digest);
	explicit_bzero(bindigest, sizeof(bindigest));
	return 0;
}

/*
 * Pre-hashes the password together with a domain separation tag and the salt
 * characters, and hashes the base64 digest with bcrypt using the same salt.
 * Returns zero on success and nonzero otherwise.
 */
static int hashpw_prehash(enum prehash_alg alg, const char *passwd,
			  const char *salt, char hash[BCRYPT_HASHSIZE])
{
	const char *tag;
	size_t taglen;
	size_t pwlen;
	size_t total;
	char *buf;
	char digest[BCRYPT_512BITS_BASE64_SIZE];
	int ret;

	if (strlen(salt) < SALT_OFFSET + SALT_LENGTH || salt[0] != '$' ||
	    salt[1] != '2' || salt[3] != '$' || salt[6] != '$')
		return 1;

	tag = (alg == PREHASH_SHA512)?PREHASH_SHA512_TAG:PREHASH_SHA3_512_TAG;
	taglen = strlen(tag);
	pwlen = strlen(passwd);
	if (pwlen > SIZE_MAX - taglen - SALT_LENGTH)
		return 1;

	total = taglen + SALT_LENGTH + pwlen;
	buf = malloc(total);
	if (buf == NULL)
		return 1;

	memcpy(buf, tag, taglen);
	memcpy(buf + taglen, salt + SALT_OFFSET, SALT_LENGTH);
	memcpy(buf + taglen + SALT_LENGTH, passwd, pwlen);
	ret = prehash(alg, buf, total, digest);
	explicit_bzero(buf, total);
	free(buf);

	if (ret == 0)
		ret = bcrypt_hashpw(digest, salt, hash);
	explicit_bzero(digest, sizeof(digest));
	return ret;
}

static int checkpw_prehash(enum prehash_alg alg, const char *passwd,
			   const char *hash)
{
	char outhash[BCRYPT_HASHSIZE];

	if (hashpw_prehash(alg, passwd, hash, outhash) != 0)
		return -1;

	return timing_safe_strcmp(hash, outhash);
}

BCRYPT_API int bcrypt_hashpw_sha512(const char *passwd, const char *salt, char hash[BCRYPT_HASHSIZE])
{
	return hashpw_prehash(PREHASH_SHA512, passwd, salt, hash);
}

BCRYPT_API int bcrypt_checkpw_sha512(const char *passwd, const char *hash)
{
	return checkpw_prehash(PREHASH_SHA512, passwd, hash);
}

BCRYPT_API int bcrypt_hashpw_sha3_512(const char *passwd, const char *salt, char hash[BCRYPT_HASHSIZE])
{
	return hashpw_prehash(PREHASH_SHA3_512, passwd, salt, hash);
}

BCRYPT_API int bcrypt_checkpw_sha3_512(const char *passwd, const char *hash)
{
	return checkpw_prehash(PREHASH_SHA3_512, passwd, hash);
}

/* Deprecated. */
BCRYPT_API int bcrypt_sha512(const char *in, char digest[BCRYPT_512BITS_BASE64_SIZE])
{
	return prehash(PREHASH_SHA512, in, strlen(in), digest);
}

/* Deprecated. */
BCRYPT_API int bcrypt_sha3_512(const char *in, char digest[BCRYPT_512BITS_BASE64_SIZE])
{
	return prehash(PREHASH_SHA3_512, in, strlen(in), digest);
}
