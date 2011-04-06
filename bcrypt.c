#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "bcrypt.h"
#include "crypt_blowfish/ow-crypt.h"

#define RANDBYTES (16)

static int try_close(int fd)
{
	int ret;
	for (;;) {
		errno = 0;
		ret = close(fd);
		if (ret == -1 && errno == EINTR)
			continue;
		break;
	}
	return ret;
}

static int try_read(int fd, void *out, size_t count)
{
	size_t total;
	ssize_t partial;

	total = 0;
	do {
		for (;;) {
			errno = 0;
			partial = read(fd, out + total, count - total);
			if (partial == -1 && errno == EINTR)
				continue;
			break;
		}

		if (partial < 1)
			return partial;

		total += partial;
	} while (total < count);
	return 0;
}

int bcrypt_gensalt(int factor, char salt[BCRYPT_HASHSIZE])
{
	int fd;
	char input[RANDBYTES];
	int workf;
	char *aux;

	fd = open("/dev/urandom", O_RDONLY);
	if (fd == -1)
		return 1;

	if (try_read(fd, salt, RANDBYTES) != 0) {
		if (try_close(fd) != 0)
			return 4;
		return 2;
	}

	if (try_close(fd) != 0)
		return 3;

	/* Generate salt. */
	workf = (factor < 4 || factor > 31)?12:factor;
	aux = crypt_gensalt_rn("$2a$", workf, input, RANDBYTES,
			       salt, BCRYPT_HASHSIZE);
	return (aux == NULL)?5:0;
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
