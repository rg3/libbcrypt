CC = gcc
EXTRA_CFLAGS = -fPIC -fvisibility=hidden
CFLAGS = $(shell grep '^CFLAGS = ' crypt_blowfish/Makefile | cut -d= -f2-) $(EXTRA_CFLAGS)

BCRYPT_MAJOR = 1
BCRYPT_MINOR = 0
BCRYPT_PATCH = 0

BCRYPT_PRNAME = bcrypt
BCRYPT_LDNAME = lib$(BCRYPT_PRNAME).so
BCRYPT_SONAME = $(BCRYPT_LDNAME).$(BCRYPT_MAJOR)
BCRYPT_SOFILE = $(BCRYPT_SONAME).$(BCRYPT_MINOR).$(BCRYPT_PATCH)

all: $(BCRYPT_SONAME)

test: bcrypt_test
	@set -e ; \
	    for prog in $^; do \
		LD_LIBRARY_PATH=. ./$$prog ; \
	    done

bcrypt_test: $(BCRYPT_SONAME) bcrypt_test.o
	$(CC) -o $@ $^

$(BCRYPT_SONAME): $(BCRYPT_SOFILE)
	ln -sf $^ $@

$(BCRYPT_SOFILE): bcrypt.o crypt_blowfish
	$(CC) $(EXTRA_CFLAGS) -shared -Wl,-soname,$(BCRYPT_SONAME) -o $(BCRYPT_SOFILE) bcrypt.o crypt_blowfish/*.o

%.o: %.c bcrypt.h
	$(CC) $(CFLAGS) -c $<

.PHONY: crypt_blowfish
crypt_blowfish:
	$(MAKE) CFLAGS="$(CFLAGS)" -C crypt_blowfish

clean:
	rm -f *.o bcrypt_test $(BCRYPT_SOFILE) $(BCRYPT_SONAME) *~ core
	$(MAKE) -C crypt_blowfish clean
