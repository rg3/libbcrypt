# Solar Designer implementation.
CRYPT_BLOWFISH_DIR = crypt_blowfish
CRYPT_BLOWFISH_LIB = $(CRYPT_BLOWFISH_DIR)/crypt_blowfish.a

# Compiler.
CC = $(shell grep '^CC = ' $(CRYPT_BLOWFISH_DIR)/Makefile | cut -d= -f2-)

# Compiler flags.
EXTRA_CFLAGS = -fPIC -fvisibility=hidden
CFLAGS = $(shell grep '^CFLAGS = ' $(CRYPT_BLOWFISH_DIR)/Makefile | cut -d= -f2-) $(EXTRA_CFLAGS)

# Library version.
BCRYPT_MAJOR = 1
BCRYPT_MINOR = 0
BCRYPT_PATCH = 0

# Library files and variables.
BCRYPT_BASENAME = bcrypt
BCRYPT_LIBNAME = lib$(BCRYPT_BASENAME)
BCRYPT_LDNAME = $(BCRYPT_LIBNAME).so
BCRYPT_SONAME = $(BCRYPT_LDNAME).$(BCRYPT_MAJOR)
BCRYPT_SOFILE = $(BCRYPT_SONAME).$(BCRYPT_MINOR).$(BCRYPT_PATCH)

BCRYPT_MANPAGE = $(BCRYPT_BASENAME).3
BCRYPT_MANPAGE_SOURCE = $(BCRYPT_MANPAGE).txt
BCRYPT_MANPAGE_LINKS = $(BCRYPT_BASENAME)_gensalt.3 $(BCRYPT_BASENAME)_hashpw.3 $(BCRYPT_BASENAME)_checkpw.3

BCRYPT_HEADER = $(BCRYPT_BASENAME).h
BCRYPT_PCFILE = $(BCRYPT_LIBNAME).pc

# Installation variables.

# This is the most common convention. It is used for LIBDIR, which can be
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
INCLUDEDIR ?= $(PREFIX)/include
LIBDIR ?= $(PREFIX)/$(LIBDIRNAME)

INCLUDESUBDIR = $(INCLUDEDIR)/$(BCRYPT_BASENAME)
MAN3DIR = $(MANDIR)/man3
PKGCONFIGDIR = $(LIBDIR)/pkgconfig

#
# Macros and rules.
#

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

.PHONY: all
all: $(BCRYPT_SOFILE) $(BCRYPT_MANPAGE) $(BCRYPT_PCFILE) $(BCRYPT_HEADER)

.PHONY: test
test: $(BCRYPT_BASENAME)_test
	@set -e ; \
	    for prog in $^; do \
	        LD_LIBRARY_PATH=.:$$LD_LIBRARY_PATH ./$$prog ; \
	    done

$(BCRYPT_BASENAME)_test: $(BCRYPT_BASENAME)_test.o $(BCRYPT_LDNAME)
	$(CC) -o $@ $< -L. -l$(BCRYPT_BASENAME)

$(BCRYPT_LDNAME) $(BCRYPT_SONAME): $(BCRYPT_SOFILE)
	$(call make_lib_links,.)

$(BCRYPT_SOFILE): $(BCRYPT_BASENAME).o keccak.o base64.o sha512.o $(CRYPT_BLOWFISH_LIB)
	$(CC) $(EXTRA_CFLAGS) -shared -Wl,-soname,$(BCRYPT_SONAME) -o $@ $^

FORCE:

$(CRYPT_BLOWFISH_LIB): FORCE
	@set -e ; \
	    $(MAKE) -q CFLAGS="$(CFLAGS)" -C $(CRYPT_BLOWFISH_DIR) || \
	    ( $(MAKE) CFLAGS="$(CFLAGS)" -C $(CRYPT_BLOWFISH_DIR) && \
	      ar Dr $@ $(CRYPT_BLOWFISH_DIR)/*.o && ranlib -D $@ )

$(BCRYPT_BASENAME)_test.c: $(BCRYPT_HEADER)

%.c: %.h

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(BCRYPT_MANPAGE): $(BCRYPT_MANPAGE_SOURCE)
	a2x --doctype manpage --format manpage $<

$(BCRYPT_PCFILE):
	@set -e ; \
	    >$@ ; \
	    echo "Name: $(BCRYPT_BASENAME)" >>$@ ; \
	    echo "Description: bcrypt password hash C library" >>$@ ; \
	    echo "Version: $(BCRYPT_MAJOR).$(BCRYPT_MINOR).$(BCRYPT_PATCH)" >>$@ ; \
	    echo "Cflags: -I$(INCLUDESUBDIR)" >>$@ ; \
	    echo "Libs: -L$(LIBDIR) -l$(BCRYPT_BASENAME)" >>$@ ; \
	    :

.PHONY: clean
clean:
	rm -f *.o $(BCRYPT_BASENAME)_test $(BCRYPT_SOFILE) $(BCRYPT_SONAME) $(BCRYPT_LDNAME) $(BCRYPT_MANPAGE) $(BCRYPT_PCFILE) *~ core
	$(MAKE) -C $(CRYPT_BLOWFISH_DIR) clean

.PHONY: install
install: all
	install -d $(DESTDIR)$(MAN3DIR)
	install -d $(DESTDIR)$(INCLUDESUBDIR)
	install -d $(DESTDIR)$(LIBDIR)
	install -d $(DESTDIR)$(PKGCONFIGDIR)
	install -m 755 $(BCRYPT_SOFILE) $(DESTDIR)$(LIBDIR)
	install -m 644 $(BCRYPT_HEADER) $(DESTDIR)$(INCLUDESUBDIR)
	install -m 644 $(BCRYPT_MANPAGE) $(DESTDIR)$(MAN3DIR)
	install -m 644 $(BCRYPT_PCFILE) $(DESTDIR)$(PKGCONFIGDIR)
	$(call make_lib_links,$(DESTDIR)$(LIBDIR))
	$(call make_man_links,$(DESTDIR)$(MAN3DIR))
