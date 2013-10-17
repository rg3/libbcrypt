#ifndef BCRYPT_H_
#define BCRYPT_H_

#define BCRYPT_HASHSIZE	(64)

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This function expects a work factor between 4 and 31 and a char array to
 * store the resulting generated salt. The char array should typically have
 * BCRYPT_HASHSIZE bytes at least. If the provided work factor is not in the
 * previous range, it will default to 12.
 *
 * The return value is zero if the salt could be correctly generated and
 * nonzero otherwise.
 *
 */
int bcrypt_gensalt(int workfactor, char salt[BCRYPT_HASHSIZE]);

/*
 * This function expects a password to be hashed, a salt to hash the password
 * with and a char array to leave the result. It can also be used to verify a
 * hashed password. In that case, provide the expected hash in the salt
 * parameter and verify the output hash is the same as the input hash. Both the
 * salt and the hash parameters should have room for BCRYPT_HASHSIZE characters
 * at least.
 *
 * The return value is zero if the password could be hashed and nonzero
 * otherwise.
 */
int bcrypt_hashpw(const char *passwd, const char salt[BCRYPT_HASHSIZE],
		  char hash[BCRYPT_HASHSIZE]);

/*
 * Brief Example
 * -------------
 *
 * Hashing a password:
 *
 *	char salt[BCRYPT_HASHSIZE];
 *	char hash[BCRYPT_HASHSIZE];
 *
 *	assert(bcrypt_gensalt(12, salt) == 0);
 *	assert(bcrypt_hashpw("thepassword", salt, hash) == 0);
 *
 *
 * Verifying a password:
 *
 *	char outhash[BCRYPT_HASHSIZE];
 *
 *	assert(bcrypt_hashpw("thepassword", "expectedhash", outhash) == 0);
 *
 *	if (strcmp("expectedhash", outhash) == 0) {
 *		printf("The password matches\n");
 *	} else {
 *		printf("The password does NOT match\n");
 *	}
 *
 * IMPORTANT NOTE: using strcmp or memcmp like in this simple example may make
 * your code vulnerable to timing attacks[1]. If possible, use a function that
 * always compares all the characters in the string before returning.
 *
 * [1] https://en.wikipedia.org/wiki/Timing_attack
 *
 */

#ifdef __cplusplus
}
#endif

#endif
