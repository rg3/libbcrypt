# Solar Designer implementation.
CRYPT_BLOWFISH_DIR = crypt_blowfish
CRYPT_BLOWFISH_SRC = $(sort $(wildcard $(CRYPT_BLOWFISH_DIR)/*.c $(CRYPT_BLOWFISH_DIR)/*.S))
CRYPT_BLOWFISH_OBJ = $(patsubst %.c,%.o,$(patsubst %.S,%.o,$(CRYPT_BLOWFISH_SRC)))

# Compiler.
CC = $(shell grep '^CC = ' $(CRYPT_BLOWFISH_DIR)/Makefile | cut -d= -f2-)
EXTRA_CFLAGS = -fPIC -fvisibility=hidden
CFLAGS = $(shell grep '^CFLAGS = ' $(CRYPT_BLOWFISH_DIR)/Makefile | cut -d= -f2-) $(EXTRA_CFLAGS)

BCRYPT_MAJOR = 1
BCRYPT_MINOR = 0
BCRYPT_PATCH = 0

BCRYPT_PRNAME = bcrypt
BCRYPT_LDNAME = lib$(BCRYPT_PRNAME).so
BCRYPT_SONAME = $(BCRYPT_LDNAME).$(BCRYPT_MAJOR)
BCRYPT_SOFILE = $(BCRYPT_SONAME).$(BCRYPT_MINOR).$(BCRYPT_PATCH)

BCRYPT_MANPAGE = bcrypt.3
BCRYPT_MANPAGE_SOURCE = $(BCRYPT_MANPAGE).txt
BCRYPT_MANPAGE_LINKS = bcrypt_gensalt.3 bcrypt_hashpw.3 bcrypt_checkpw.3

# Installation variables.

# This is the most common convention. This is used for LIBDIR, which can be
# overridden completely below.
MACHINE = $(shell uname -m)
ifeq ($(MACHINE),x86_64)
    LIBDIRNAME = lib64
else
    LIBDIRNAME = lib
endif

DESTDIR ?=
PREFIX ?= /usr/local
MANDIR ?= $(PREFIX)/share/man
MAN3DIR = $(MANDIR)/man3
INCLUDEDIR ?= $(PREFIX)/include/bcrypt
LIBDIR ?= $(PREFIX)/$(LIBDIRNAME)

define make_lib_links
set -e ; \
    ln -sf $(BCRYPT_SOFILE) $(1)/$(BCRYPT_SONAME) ; \
    ln -sf $(BCRYPT_SONAME) $(1)/$(BCRYPT_LDNAME)
endef

define make_man_links
set -e ; \
    for f in $(BCRYPT_MANPAGE_LINKS); do \
        ln -sf $(BCRYPT_MANPAGE) $(1)/$$f ; \
    done
endef

all: $(BCRYPT_SOFILE)

test: bcrypt_test
	@set -e ; \
	    for prog in $^; do \
	        LD_LIBRARY_PATH=.:$$LD_LIBRARY_PATH ./$$prog ; \
	    done

bcrypt_test: bcrypt_test.o $(BCRYPT_LDNAME)
	$(CC) -o $@ $< -L. -l$(BCRYPT_PRNAME)

$(BCRYPT_LDNAME) $(BCRYPT_SONAME): $(BCRYPT_SOFILE)
	$(call make_lib_links,.)

$(BCRYPT_SOFILE): bcrypt.o $(CRYPT_BLOWFISH_OBJ)
	$(CC) $(EXTRA_CFLAGS) -shared -Wl,-soname,$(BCRYPT_SONAME) -o $(BCRYPT_SOFILE) bcrypt.o $(CRYPT_BLOWFISH_OBJ)

%.o: %.c bcrypt.h
	$(CC) $(CFLAGS) -c $<

$(CRYPT_BLOWFISH_OBJ):
	$(MAKE) CFLAGS="$(CFLAGS)" -C $(CRYPT_BLOWFISH_DIR)

$(BCRYPT_MANPAGE): $(BCRYPT_MANPAGE_SOURCE)
	a2x --doctype manpage --format manpage $<

clean:
	rm -f *.o bcrypt_test $(BCRYPT_SOFILE) $(BCRYPT_SONAME) $(BCRYPT_LDNAME) $(BCRYPT_MANPAGE) *~ core
	$(MAKE) -C $(CRYPT_BLOWFISH_DIR) clean

install: $(BCRYPT_SOFILE) $(BCRYPT_MANPAGE) bcrypt.h
	install -d $(DESTDIR)$(MAN3DIR)
	install -d $(DESTDIR)$(INCLUDEDIR)
	install -d $(DESTDIR)$(LIBDIR)
	install -m 755 $(BCRYPT_SOFILE) $(DESTDIR)$(LIBDIR)
	install -m 644 bcrypt.h $(DESTDIR)$(INCLUDEDIR)
	install -m 644 $(BCRYPT_MANPAGE) $(DESTDIR)$(MAN3DIR)
	$(call make_lib_links,$(DESTDIR)$(LIBDIR))
	$(call make_man_links,$(DESTDIR)$(MAN3DIR))
