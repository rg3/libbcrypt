#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "bcrypt.h"

int main(void)
{
	clock_t before;
	clock_t after;
	double elapsed;
	char salt[BCRYPT_HASHSIZE];
	char hash[BCRYPT_HASHSIZE];
	char digest1[BCRYPT_512BITS_BASE64_SIZE];
	char digest2[BCRYPT_512BITS_BASE64_SIZE];
	int ret;

	const char pass[] = "hi,mom";
	const char hash1[] = "$2a$10$VEVmGHy4F4XQMJ3eOZJAUeb.MedU0W10pTPCuf53eHdKJPiSE8sMK";
	const char hash2[] = "$2a$10$3F0BVk5t8/aoS.3ddaB3l.fxg5qvafQ9NybxcpXLzMeAt.nVWn.NO";
	const char sha512[] = "TLbNem2eA98WFZOYRMOTshG7GNJbrb0O9rf6H2uXKL4CUh1mBr4ukRWD6JS6oxlgDUe60I03yjnA+LwpqDpRNQ==";
	const char sha3_512[] = "W9QbCtFa66fIA7HD+0ItZRiq+eZCtNuwvXs+zhqeW/uzf/HSx1HImVhlsRWttuGkg2hQ1nDY/I1KSwbjB1n2lw==";

	ret = bcrypt_gensalt(BCRYPT_DEFAULT_WORK_FACTOR, salt);
	assert(ret == 0);
	printf("Generated salt: %s\n", salt);

	before = clock();
	ret = bcrypt_hashpw("test", salt, hash);
	after = clock();
	assert(ret == 0);
	printf("Hashed password: %s\n", hash);
	elapsed = (double)(after - before) / CLOCKS_PER_SEC;
	printf("Time taken: %f seconds\n", elapsed);

	if (elapsed < 0.050)
		printf("WARNING: the default work factor (%d) "
			"may be too small for your system\n",
			BCRYPT_DEFAULT_WORK_FACTOR);
	else if (elapsed > 1.0)
		printf("WARNING: the default work factor (%d) "
			"may be too big for your system\n",
			BCRYPT_DEFAULT_WORK_FACTOR);

	ret = bcrypt_hashpw(pass, hash1, hash);
	assert(ret == 0);
	assert(strcmp(hash1, hash) == 0);

	ret = bcrypt_hashpw(pass, hash2, hash);
	assert(ret == 0);
	assert(strcmp(hash2, hash) == 0);

	ret = bcrypt_checkpw(pass, hash1);
	assert(ret == 0);

	ret = bcrypt_checkpw(pass, hash2);
	assert(ret == 0);

	ret = bcrypt_sha512(pass, digest1);
	assert(ret == 0);
	assert(strcmp(digest1, sha512) == 0);

	ret = bcrypt_sha3_512(pass, digest2);
	assert(ret == 0);
	assert(strcmp(digest2, sha3_512) == 0);

	printf("All tests passed\n");
	return 0;
}
