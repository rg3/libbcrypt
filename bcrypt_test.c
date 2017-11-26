#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "bcrypt.h"

int main(void)
{
	clock_t before;
	clock_t after;
	char salt[BCRYPT_HASHSIZE];
	char hash[BCRYPT_HASHSIZE];
	int ret;

	const char pass[] = "hi,mom";
	const char hash1[] = "$2a$10$VEVmGHy4F4XQMJ3eOZJAUeb.MedU0W10pTPCuf53eHdKJPiSE8sMK";
	const char hash2[] = "$2a$10$3F0BVk5t8/aoS.3ddaB3l.fxg5qvafQ9NybxcpXLzMeAt.nVWn.NO";

	ret = bcrypt_gensalt(12, salt);
	assert(ret == 0);
	printf("Generated salt: %s\n", salt);
	before = clock();
	ret = bcrypt_hashpw("test", salt, hash);
	after = clock();
	assert(ret == 0);
	printf("Hashed password: %s\n", hash);
	printf("Time taken: %f seconds\n",
	       (double)(after - before) / CLOCKS_PER_SEC);

	ret = bcrypt_hashpw(pass, hash1, hash);
	assert(ret == 0);
	printf("First hash check: %s\n", (strcmp(hash1, hash) == 0)?"OK":"FAIL");
	ret = bcrypt_hashpw(pass, hash2, hash);
	assert(ret == 0);
	printf("Second hash check: %s\n", (strcmp(hash2, hash) == 0)?"OK":"FAIL");

	before = clock();
	ret = (bcrypt_checkpw(pass, hash1) == 0);
	after = clock();
	printf("First hash check with bcrypt_checkpw: %s\n", ret?"OK":"FAIL");
	printf("Time taken: %f seconds\n",
	       (double)(after - before) / CLOCKS_PER_SEC);

	before = clock();
	ret = (bcrypt_checkpw(pass, hash2) == 0);
	after = clock();
	printf("Second hash check with bcrypt_checkpw: %s\n", ret?"OK":"FAIL");
	printf("Time taken: %f seconds\n",
	       (double)(after - before) / CLOCKS_PER_SEC);

	return 0;
}
