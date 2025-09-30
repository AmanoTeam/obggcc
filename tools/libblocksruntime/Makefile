CC ?= cc
AR ?= ar
CFLAGS ?= -O2 -g
LDFLAGS ?= -Xlinker -s
CPPFLAGS ?= 

SOURCE_DIRECTORY := $(patsubst %/,%,$(dir $(realpath $(firstword $(MAKEFILE_LIST)))))
BLOCKSRUNTIME_DIRECTORY = $(SOURCE_DIRECTORY)/../../submodules/llvm-project/compiler-rt/lib/BlocksRuntime

PREFIX ?= $(SOURCE_DIRECTORY)

SOURCES = $(BLOCKSRUNTIME_DIRECTORY)/data.c \
	$(BLOCKSRUNTIME_DIRECTORY)/runtime.c

OBJECTS = $(SOURCE_DIRECTORY)/data.o \
	$(SOURCE_DIRECTORY)/runtime.o

HEADERS = $(BLOCKSRUNTIME_DIRECTORY)/Block.h \
	$(BLOCKSRUNTIME_DIRECTORY)/Block_private.h

all: libblocksruntime-static install

libblocksruntime:
	$(CC) -c -I $(SOURCE_DIRECTORY) -I $(BLOCKSRUNTIME_DIRECTORY) $(CFLAGS) $(LDFLAGS) $(CPPFLAGS) $(SOURCES)

libblocksruntime-static: libblocksruntime
	$(AR) rcs $(SOURCE_DIRECTORY)/libBlocksRuntime.a $(OBJECTS)

install:
	install -m 0644 $(HEADERS) $(PREFIX)/include
	install -m 0644 $(SOURCE_DIRECTORY)/libBlocksRuntime.a $(PREFIX)/lib
