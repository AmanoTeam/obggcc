CC ?= cc
CFLAGS ?= -O2
LDFLAGS ?= -Xlinker -s
CPPFLAGS ?= 

FLAVOR ?= OBGGCC

SOURCE_DIRECTORY = $(dir $(realpath $(firstword $(MAKEFILE_LIST))))

PREFIX ?= $(SOURCE_DIRECTORY)

SOURCES = biggestint.c \
	errors.c \
	fstream.c \
	hex.c \
	obggcc.c \
	query.c \
	strsplit.c \
	urldecode.c \
	urlencode.c \
	walkdir.c \
	kargv.c \
	fs/absoluteness.c \
	fs/basename.c \
	fs/cp.c \
	fs/dirname.c \
	fs/exists.c \
	fs/ext.c \
	fs/getexec.c \
	fs/parentpath.c \
	fs/realpath.c \
	fs/stripsep.c \
	fs/rm.c \
	fs/cwd.c \
	os/find_exe.c

GCC_SOURCES = $(SOURCES) gcc.c
STRIP_SOURCES = $(SOURCES) binutils.c

gcc:
	$(CC) -Wno-discarded-qualifiers -D $(FLAVOR) -I $(SOURCE_DIRECTORY) $(CFLAGS) $(LDFLAGS) $(CPPFLAGS) $(GCC_SOURCES) -o $(PREFIX)/gcc-wrapper

binutils:
	$(CC) -Wno-discarded-qualifiers -D $(FLAVOR) -I $(SOURCE_DIRECTORY) $(CFLAGS) $(LDFLAGS) $(CPPFLAGS) $(STRIP_SOURCES) -o $(PREFIX)/binutils-wrapper

all: gcc binutils
