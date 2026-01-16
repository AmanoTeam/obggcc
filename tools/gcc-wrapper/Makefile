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
	fs/absrel.c \
	fs/basename.c \
	fs/cp.c \
	fs/dirname.c \
	fs/exists.c \
	fs/splitext.c \
	fs/getexec.c \
	fs/parentpath.c \
	fs/realpath.c \
	fs/stripsep.c \
	fs/rm.c \
	fs/cwd.c \
	os/find_exe.c \
	os/execve.c

GCC_SOURCES = $(SOURCES) gcc.c
BINUTILS_LLVM_SOURCES = $(SOURCES) binutils.c
BINUTILS_GNU_SOURCES = $(SOURCES) binutils-gnu.c

gcc:
	$(CC) -Wno-discarded-qualifiers -D $(FLAVOR) -I $(SOURCE_DIRECTORY) $(CFLAGS) $(LDFLAGS) $(CPPFLAGS) $(GCC_SOURCES) -o $(PREFIX)/gcc-wrapper

binutils:
	$(CC) -Wno-discarded-qualifiers -D $(FLAVOR) -I $(SOURCE_DIRECTORY) $(CFLAGS) $(LDFLAGS) $(CPPFLAGS) $(BINUTILS_LLVM_SOURCES) -o $(PREFIX)/binutils-wrapper

binutils-gnu:
	$(CC) -Wno-discarded-qualifiers -D $(FLAVOR) -I $(SOURCE_DIRECTORY) $(CFLAGS) $(LDFLAGS) $(CPPFLAGS) $(BINUTILS_GNU_SOURCES) -o $(PREFIX)/binutils-gnu-wrapper

all: gcc binutils
