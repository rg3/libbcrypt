#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "bcrypt.h"
#include "crypt_blowfish/ow-crypt.h"

#define RANDBYTES (16)

int bcrypt_gensalt(int factor, char salt[BCRYPT_HASHSIZE])
{
	int retval;
	int fd;
	int total;
	ssize_t numbytes;
	char input[RANDBYTES];
	char *aux;

	fd = open("/dev/urandom", O_RDONLY);
	if (fd == -1)
		return 1;

	total = 0;
	do {
		/* Read. */
		for (;;) {
			errno = 0;
			numbytes = read(fd, input + total, RANDBYTES - total);
			if (numbytes == -1 && errno == EINTR)
				continue;
			break;
		}

		/* Check. */
		if (numbytes < 1) {
			for (;;) {
				errno = 0;
				if (close(fd) == -1 && errno == EINTR)
					continue;
				break;
			}
			return 2;
		}

		total += numbytes;
	} while (total < RANDBYTES);

	/* Generate salt. */
	aux = crypt_gensalt_rn("$2a$",
			       (factor < 4 || factor > 31)?12:factor,
			       input, RANDBYTES, salt, BCRYPT_HASHSIZE);

	/* Check and close. */
	retval = (aux == NULL)?3:0;
	for (;;) {
		errno = 0;
		if (close(fd) == -1 && errno == EINTR)
			continue;
		break;
	}

	return retval;
}

int bcrypt_hashpw(const char *passwd, const char salt[BCRYPT_HASHSIZE], char hash[BCRYPT_HASHSIZE])
{
	char *aux;

	aux = crypt_rn(passwd, salt, hash, BCRYPT_HASHSIZE);
	return (aux == NULL)?1:0;
}

#ifdef _TEST_BCRYPT_
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int main()
{
	clock_t before;
	clock_t after;
	char salt[BCRYPT_HASHSIZE];
	char hash[BCRYPT_HASHSIZE];

	const char pass[] = "hi,mom";
	const char hash1[] = "$2a$10$VEVmGHy4F4XQMJ3eOZJAUeb.MedU0W10pTPCuf53eHdKJPiSE8sMK";
	const char hash2[] = "$2a$10$3F0BVk5t8/aoS.3ddaB3l.fxg5qvafQ9NybxcpXLzMeAt.nVWn.NO";

	assert(bcrypt_gensalt(12, salt) == 0);
	printf("Generated salt: %s\n", salt);
	before = clock();
	assert(bcrypt_hashpw("testtesttest", salt, hash) == 0);
	after = clock();
	printf("Hashed password: %s\n", hash);
	printf("Time taken: %f seconds\n",
	       (float)(after - before) / CLOCKS_PER_SEC);

	assert(bcrypt_hashpw(pass, hash1, hash) == 0);
	printf("First hash check: %s\n", (strcmp(hash1, hash) == 0)?"OK":"FAIL");
	assert(bcrypt_hashpw(pass, hash2, hash) == 0);
	printf("Second hash check: %s\n", (strcmp(hash2, hash) == 0)?"OK":"FAIL");

	return 0;
}
#endif
