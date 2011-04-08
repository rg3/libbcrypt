CC = gcc
CFLAGS = -W -Wall -O2 -fomit-frame-pointer -funroll-loops

all: bcrypt.a

test: bcrypt.c crypt_blowfish/crypt_blowfish.a
	$(CC) $(CFLAGS) -D _TEST_BCRYPT_ -c bcrypt.c
	$(CC) -o bcrypt_test bcrypt.o crypt_blowfish/crypt_blowfish.a

bcrypt.a: bcrypt.o crypt_blowfish/crypt_blowfish.a
	ar r bcrypt.a bcrypt.o crypt_blowfish/crypt_blowfish.a

bcrypt.o: bcrypt.c
	$(CC) $(CFLAGS) -c bcrypt.c

crypt_blowfish/crypt_blowfish.a:
	$(MAKE) -C crypt_blowfish crypt_blowfish.a

clean:
	rm -f *.o bcrypt_test bcrypt.a *~ core
	$(MAKE) -C crypt_blowfish clean
