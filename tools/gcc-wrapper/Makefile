CC ?= cc
CFLAGS ?= -O2
LDFLAGS ?= -Xlinker -s
CPPFLAGS ?= 
CROSS_COMPILE_TRIPLET ?= 

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
	os/execv.c \
	os/posix_spawn.c

GCC_SOURCES = $(SOURCES) gcc.c
BINUTILS_LLVM_SOURCES = $(SOURCES) binutils-llvm.c
BINUTILS_GNU_SOURCES = $(SOURCES) binutils-gnu.c

ifneq (,$(filter %-mingw32,$(CROSS_COMPILE_TRIPLET)))
	override CFLAGS += -D_WIN32_WINNT=0x600
	override LDFLAGS += -lkernel32
endif

gcc:
	$(CC) -Wno-discarded-qualifiers -D $(FLAVOR) -I $(SOURCE_DIRECTORY) $(CFLAGS) $(LDFLAGS) $(CPPFLAGS) $(GCC_SOURCES) -o $(PREFIX)/gcc-wrapper

binutils-llvm:
	$(CC) -Wno-discarded-qualifiers -D $(FLAVOR) -I $(SOURCE_DIRECTORY) $(CFLAGS) $(LDFLAGS) $(CPPFLAGS) $(BINUTILS_LLVM_SOURCES) -o $(PREFIX)/binutils-llvm-wrapper

binutils-gnu:
	$(CC) -Wno-discarded-qualifiers -D $(FLAVOR) -I $(SOURCE_DIRECTORY) $(CFLAGS) $(LDFLAGS) $(CPPFLAGS) $(BINUTILS_GNU_SOURCES) -o $(PREFIX)/binutils-gnu-wrapper

all: gcc binutils-llvm binutils-gnu
