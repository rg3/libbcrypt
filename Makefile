# Solar Designer implementation.
CRYPT_BLOWFISH_DIR = crypt_blowfish
CRYPT_BLOWFISH_LIB = $(CRYPT_BLOWFISH_DIR)/$(CRYPT_BLOWFISH_DIR).a

# Compiler. Use the one from crypt_blowfish (gcc) if available, falling back to
# cc in systems without it, like most BSDs.
CRYPT_BLOWFISH_CC := $(strip $(shell grep '^CC = ' $(CRYPT_BLOWFISH_DIR)/Makefile | cut -d= -f2-))
CC := $(if $(shell command -v $(CRYPT_BLOWFISH_CC) 2>/dev/null),$(CRYPT_BLOWFISH_CC),cc)

# Compiler flags.
CFLAGS = $(shell grep '^CFLAGS = ' $(CRYPT_BLOWFISH_DIR)/Makefile | cut -d= -f2-)
override CFLAGS += -fPIC -fvisibility=hidden

# crypt_blowfish objects. x86.S only has code for 32-bit x86, and it fails to
# assemble on some other architectures, like 32-bit ARM, so it is only built
# when targeting 32-bit x86. The object list is passed to crypt_blowfish's
# Makefile, which is used without modifications.
CRYPT_BLOWFISH_I386 := $(strip $(shell echo __i386__ | $(CC) $(CFLAGS) -x c -E -P - 2>/dev/null))
CRYPT_BLOWFISH_BF_OBJS = crypt_blowfish.o $(if $(filter 1,$(CRYPT_BLOWFISH_I386)),x86.o)
CRYPT_BLOWFISH_OBJ = $(addprefix $(CRYPT_BLOWFISH_DIR)/,$(CRYPT_BLOWFISH_BF_OBJS) crypt_gensalt.o wrapper.o)
CRYPT_BLOWFISH_MAKE = $(MAKE) CC="$(CC)" CFLAGS="$(CFLAGS)" BLOWFISH_OBJS="$(CRYPT_BLOWFISH_BF_OBJS)" -C $(CRYPT_BLOWFISH_DIR)

# Linker flags, empty by default.
LDFLAGS =

# Other tools. Old versions of ar may not support the D (deterministic) flag.
AR = ar
ARFLAGS = rcsD
OBJCOPY = objcopy

# Library version.
BCRYPT_MAJOR = 1
BCRYPT_MINOR = 1
BCRYPT_PATCH = 0

# Library files and variables.
BCRYPT_BASENAME = bcrypt
BCRYPT_LIBNAME = lib$(BCRYPT_BASENAME)
BCRYPT_LDNAME = $(BCRYPT_LIBNAME).so
BCRYPT_LAFILE = $(BCRYPT_LIBNAME).a
BCRYPT_SONAME = $(BCRYPT_LDNAME).$(BCRYPT_MAJOR)
BCRYPT_SOFILE = $(BCRYPT_SONAME).$(BCRYPT_MINOR).$(BCRYPT_PATCH)

BCRYPT_MANPAGE = $(BCRYPT_BASENAME).3
BCRYPT_MANPAGE_SOURCE = $(BCRYPT_MANPAGE).txt
BCRYPT_MANPAGE_LINKS = $(BCRYPT_BASENAME)_gensalt.3 $(BCRYPT_BASENAME)_hashpw.3 $(BCRYPT_BASENAME)_checkpw.3 $(BCRYPT_BASENAME)_sha512.3 $(BCRYPT_BASENAME)_sha3_512.3 $(BCRYPT_BASENAME)_hashpw_sha512.3 $(BCRYPT_BASENAME)_checkpw_sha512.3 $(BCRYPT_BASENAME)_hashpw_sha3_512.3 $(BCRYPT_BASENAME)_checkpw_sha3_512.3

BCRYPT_HEADER = $(BCRYPT_BASENAME).h
BCRYPT_PCFILE = $(BCRYPT_LIBNAME).pc
BCRYPT_OBJS = $(BCRYPT_BASENAME).o keccak.o base64.o sha512.o

# All library code (ours and crypt_blowfish) is merged into this object file.
# Every symbol except the public ones is made local, so the static library does
# not expose crypt_blowfish functions like crypt() that would clash with the
# system ones. Keep this list in sync with bcrypt.h.
BCRYPT_ALL_OBJ = $(BCRYPT_LIBNAME)_all.o
BCRYPT_EXPORTS = bcrypt_gensalt bcrypt_hashpw bcrypt_checkpw bcrypt_sha512 bcrypt_sha3_512 bcrypt_hashpw_sha512 bcrypt_checkpw_sha512 bcrypt_hashpw_sha3_512 bcrypt_checkpw_sha3_512

# Installation variables.

# These may be set by the user on the command line.
DESTDIR =
PREFIX = /usr/local
MANDIR = $(PREFIX)/share/man
INCLUDEDIR = $(PREFIX)/include

# Use lib64 if the system C library is in a lib64 directory (e.g. Fedora and
# derivatives), and lib otherwise (e.g. Debian and the BSDs). Checking for an
# existing lib64 directory is not enough, as Debian has /usr/lib64 only for
# the dynamic loader.
SYSTEM_LIBC_DIR := $(patsubst %/,%,$(dir $(realpath $(shell $(CC) $(CFLAGS) -print-file-name=libc.so 2>/dev/null))))
LIBDIR = $(PREFIX)/$(if $(filter lib64,$(notdir $(SYSTEM_LIBC_DIR))),lib64,lib)

# FreeBSD expects pkg-config files in libdata instead of the library directory.
ifeq ($(shell uname -s),FreeBSD)
    PKGCONFIGDIR = $(PREFIX)/libdata/pkgconfig
else
    PKGCONFIGDIR = $(LIBDIR)/pkgconfig
endif

# Derived from the previous ones.
INCLUDESUBDIR = $(INCLUDEDIR)/$(BCRYPT_BASENAME)
MAN3DIR = $(MANDIR)/man3

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
all: $(BCRYPT_SOFILE) $(BCRYPT_LAFILE) $(BCRYPT_MANPAGE) $(BCRYPT_PCFILE) $(BCRYPT_HEADER)

.PHONY: test
test: $(BCRYPT_BASENAME)_test
	@set -e ; \
	    $(CRYPT_BLOWFISH_MAKE) check check_threads ; \
	    for prog in $^; do \
	        ./$$prog ; \
	    done

$(BCRYPT_BASENAME)_test: $(BCRYPT_BASENAME)_test.o $(BCRYPT_LAFILE)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BCRYPT_LDNAME) $(BCRYPT_SONAME): $(BCRYPT_SOFILE)
	$(call make_lib_links,.)

$(BCRYPT_SOFILE): $(BCRYPT_ALL_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -Wl,-soname,$(BCRYPT_SONAME) -o $@ $^

$(BCRYPT_LAFILE): $(BCRYPT_ALL_OBJ)
	rm -f $@ && $(AR) $(ARFLAGS) $@ $^

$(BCRYPT_ALL_OBJ): $(BCRYPT_OBJS) $(CRYPT_BLOWFISH_LIB)
	$(CC) -nostdlib -r -o $@ $(BCRYPT_OBJS) $(CRYPT_BLOWFISH_OBJ)
	$(OBJCOPY) $(addprefix --keep-global-symbol=,$(BCRYPT_EXPORTS)) $@

FORCE:

# The archive is only used as a timestamp to know if crypt_blowfish changed.
$(CRYPT_BLOWFISH_LIB): FORCE
	@set -e ; \
	    $(CRYPT_BLOWFISH_MAKE) -q || \
	    ( $(CRYPT_BLOWFISH_MAKE) && \
	      rm -f $@ && $(AR) $(ARFLAGS) $@ $(CRYPT_BLOWFISH_OBJ) )

# Header dependencies.
$(BCRYPT_BASENAME).o: $(BCRYPT_HEADER) sha512.h keccak.h base64.h $(CRYPT_BLOWFISH_DIR)/ow-crypt.h
$(BCRYPT_BASENAME)_test.o: $(BCRYPT_HEADER)
keccak.o: keccak.h
base64.o: base64.h
sha512.o: sha512.h

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(BCRYPT_MANPAGE): $(BCRYPT_MANPAGE_SOURCE)
	a2x --doctype manpage --format manpage $<

# Always regenerated, so it reflects the current installation paths, but only
# replaced when its contents change.
$(BCRYPT_PCFILE): FORCE
	@set -e ; \
	    { echo "Name: $(BCRYPT_BASENAME)" ; \
	      echo "Description: bcrypt password hash C library" ; \
	      echo "Version: $(BCRYPT_MAJOR).$(BCRYPT_MINOR).$(BCRYPT_PATCH)" ; \
	      echo "Cflags: -I$(INCLUDESUBDIR)" ; \
	      echo "Libs: -L$(LIBDIR) -l$(BCRYPT_BASENAME)" ; \
	    } >$@.tmp ; \
	    if [ -f $@ ] && [ "$$(cat $@)" = "$$(cat $@.tmp)" ] ; then rm -f $@.tmp ; else mv -f $@.tmp $@ ; fi

.PHONY: clean
clean:
	rm -f *.o $(BCRYPT_BASENAME)_test $(BCRYPT_SOFILE) $(BCRYPT_LAFILE) $(BCRYPT_SONAME) $(BCRYPT_LDNAME) $(BCRYPT_MANPAGE) $(BCRYPT_PCFILE) $(BCRYPT_PCFILE).tmp $(CRYPT_BLOWFISH_LIB) *~ core
	$(MAKE) -C $(CRYPT_BLOWFISH_DIR) clean

.PHONY: install
install: all
	install -d $(DESTDIR)$(MAN3DIR)
	install -d $(DESTDIR)$(INCLUDESUBDIR)
	install -d $(DESTDIR)$(LIBDIR)
	install -d $(DESTDIR)$(PKGCONFIGDIR)
	install -m 755 $(BCRYPT_SOFILE) $(DESTDIR)$(LIBDIR)
	install -m 644 $(BCRYPT_LAFILE) $(DESTDIR)$(LIBDIR)
	install -m 644 $(BCRYPT_HEADER) $(DESTDIR)$(INCLUDESUBDIR)
	install -m 644 $(BCRYPT_MANPAGE) $(DESTDIR)$(MAN3DIR)
	install -m 644 $(BCRYPT_PCFILE) $(DESTDIR)$(PKGCONFIGDIR)
	$(call make_lib_links,$(DESTDIR)$(LIBDIR))
	$(call make_man_links,$(DESTDIR)$(MAN3DIR))

# Removes the files installed by the install target. The shared directories
# are kept, except for the header subdirectory if it ends up empty.
.PHONY: uninstall
uninstall:
	rm -f $(addprefix $(DESTDIR)$(LIBDIR)/,$(BCRYPT_SOFILE) $(BCRYPT_SONAME) $(BCRYPT_LDNAME) $(BCRYPT_LAFILE))
	rm -f $(DESTDIR)$(INCLUDESUBDIR)/$(BCRYPT_HEADER)
	rm -f $(addprefix $(DESTDIR)$(MAN3DIR)/,$(BCRYPT_MANPAGE) $(BCRYPT_MANPAGE_LINKS))
	rm -f $(DESTDIR)$(PKGCONFIGDIR)/$(BCRYPT_PCFILE)
	rmdir $(DESTDIR)$(INCLUDESUBDIR) 2>/dev/null || true
