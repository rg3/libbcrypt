CC = gcc
CFLAGS = $(shell grep '^CFLAGS = ' crypt_blowfish/Makefile | cut -d= -f2-)
.PHONY: crypt_blowfish

all: bcrypt.a

test: libbcrypt.c crypt_blowfish
	$(CC) $(CFLAGS) -DTEST_BCRYPT -c libbcrypt.c
	$(CC) -o bcrypt_test libbcrypt.o crypt_blowfish/*.o

bcrypt.a: libbcrypt.o crypt_blowfish
	ar r bcrypt.a libbcrypt.o crypt_blowfish/*.o

libbcrypt.o: libbcrypt.c
	$(CC) $(CFLAGS) -c libbcrypt.c

crypt_blowfish:
	$(MAKE) -C crypt_blowfish

clean:
	rm -f *.o bcrypt_test bcrypt.a *~ core
	$(MAKE) -C crypt_blowfish clean
