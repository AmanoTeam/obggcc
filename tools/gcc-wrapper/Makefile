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
	os/find_exe.c

GCC_SOURCES = $(SOURCES) main.c
STRIP_SOURCES = $(SOURCES) strip.c

gcc:
	$(CC) -D $(FLAVOR) -I $(SOURCE_DIRECTORY) $(CFLAGS) $(LDFLAGS) $(CPPFLAGS) $(GCC_SOURCES) -o $(PREFIX)/gcc-wrapper

strip:
	$(CC) -D $(FLAVOR) -I $(SOURCE_DIRECTORY) $(CFLAGS) $(LDFLAGS) $(CPPFLAGS) $(STRIP_SOURCES) -o $(PREFIX)/strip-wrapper

all: gcc strip
