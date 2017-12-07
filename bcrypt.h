#ifndef BCRYPT_H_
#define BCRYPT_H_
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

#define BCRYPT_HASHSIZE	(64)
#define BCRYPT_DEFAULT_WORK_FACTOR (12)

/*
 * Length of a null-terminated string containing a base64 representantion of
 * 512 bits. Rounded up to a pretty number.
 */
#define BCRYPT_512BITS_BASE64_SIZE (90)


#ifdef __cplusplus
extern "C" {
#endif

/*
 * This function expects a work factor between 4 and 31 and a char array to
 * store the resulting generated salt. The char array should typically have
 * BCRYPT_HASHSIZE bytes at least. If the provided work factor is not in the
 * previous range, it will default to BCRYPT_DEFAULT_WORK_FACTOR.
 *
 * The return value is zero if the salt could be correctly generated and
 * nonzero otherwise.
 *
 */
int bcrypt_gensalt(int workfactor, char salt[BCRYPT_HASHSIZE]);

/*
 * This function expects a password to be hashed, a salt to hash the password
 * with and a char array to leave the result. Both the salt and the hash
 * parameters should have room for BCRYPT_HASHSIZE characters at least.
 *
 * It can also be used to verify a hashed password. In that case, provide the
 * expected hash in the salt parameter and verify the output hash is the same
 * as the input hash. However, to avoid timing attacks, it's better to use
 * bcrypt_checkpw when verifying a password.
 *
 * The return value is zero if the password could be hashed and nonzero
 * otherwise.
 *
 * Please note the generated hash is a null-terminated string that can be
 * printed or stored somewhere and already includes the salt information.
 */
int bcrypt_hashpw(const char *passwd, const char salt[BCRYPT_HASHSIZE],
		  char hash[BCRYPT_HASHSIZE]);

/*
 * This function expects a password and a hash to verify the password against.
 * The internal implementation is tuned to avoid timing attacks.
 *
 * The return value will be -1 in case of errors, zero if the provided password
 * matches the given hash and greater than zero if no errors are found and the
 * passwords don't match.
 *
 */
int bcrypt_checkpw(const char *passwd, const char hash[BCRYPT_HASHSIZE]);

/*
 * This function expects a password and a char array to leave the result, with
 * at least BCRYPT_512BITS_BASE64_SIZE capacity. It will calculate the SHA-512
 * hash of the given password and output the result as the base64
 * representation of the digest.
 *
 * It will return zero if no problems were encountered and nonzero otherwise.
 */
int bcrypt_sha512(const char *passwd,
		  char digest[BCRYPT_512BITS_BASE64_SIZE]);

/*
 * This function is identical to the one above, but it uses SHA3-512 instead of
 * SHA-512.
 */
int bcrypt_sha3_512(const char *passwd,
		    char digest[BCRYPT_512BITS_BASE64_SIZE]);

/*
 * Brief Examples
 * --------------
 *
 * Hashing a password
 *
 *     char salt[BCRYPT_HASHSIZE];
 *     char hash[BCRYPT_HASHSIZE];
 *     int ret;
 *
 *     ret = bcrypt_gensalt(BCRYPT_DEFAULT_WORK_FACTOR, salt);
 *     if (ret != 0)
 *         // Handle the error here.
 *
 *     ret = bcrypt_hashpw("thepassword", salt, hash);
 *     if (ret != 0)
 *         // Handle another error here.
 *
 *     printf("Generated hash: %s\n", hash);
 *
 *
 * Verifying a password
 *
 *     char hash[BCRYPT_HASHSIZE];
 *     int ret;
 *
 *     // Read the hash somewhere here and store it in the `hash' buffer.
 *
 *     ret = bcrypt_checkpw("thepassword", hash);
 *     if (ret == -1)
 *         // Handle the error here.
 *
 *     if (ret == 0)
 *         printf("The password matches\n");
 *     else
 *         printf("The password does NOT match\n");
 *
 *
 * Hashing a password using SHA-512 as a pre-hash
 *
 *     char salt[BCRYPT_HASHSIZE];
 *     char hash[BCRYPT_HASHSIZE];
 *     char digest[BCRYPT_512BITS_BASE64_SIZE];
 *     int ret;
 *
 *     ret = bcrypt_sha512("thepassword", digest);
 *     if (ret != 0)
 *         // Handle the error here.
 *
 *     ret = bcrypt_gensalt(BCRYPT_DEFAULT_WORK_FACTOR, salt);
 *     if (ret != 0)
 *         // Handle another error here.
 *
 *     ret = bcrypt_hashpw(digest, salt, hash);
 *     if (ret != 0)
 *         // Handle another error here.
 *
 *     printf("Generated hash: %s\n", hash);
 *
 *
 * Verifying a password using SHA-512 as a pre-hash
 *
 *     char hash[BCRYPT_HASHSIZE];
 *     char digest[BCRYPT_512BITS_BASE64_SIZE];
 *     int ret;
 *
 *     // Read the hash somewhere here and store it in the `hash' buffer.
 *
 *     ret = bcrypt_sha512("thepassword", digest);
 *     if (ret != 0)
 *         // Handle the error here.
 *
 *     ret = bcrypt_checkpw(digest, hash);
 *     if (ret == -1)
 *         // Handle another error here.
 *
 *     if (ret == 0)
 *         printf("The password matches\n");
 *     else
 *         printf("The password does NOT match\n");
 *
 *
 */

#ifdef __cplusplus
}
#endif

#endif
