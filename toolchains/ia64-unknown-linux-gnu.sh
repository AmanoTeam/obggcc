#/bin/bash

set -eu

TARGET_GLIBC='2.3'
CROSS_COMPILE_TRIPLET='ia64-unknown-linux-gnu'

CC="${OBGGCC_TOOLCHAIN}/bin/${CROSS_COMPILE_TRIPLET}${TARGET_GLIBC}-gcc"
CXX="${OBGGCC_TOOLCHAIN}/bin/${CROSS_COMPILE_TRIPLET}${TARGET_GLIBC}-g++"
AR="${OBGGCC_TOOLCHAIN}/bin/${CROSS_COMPILE_TRIPLET}-ar"
AS="${OBGGCC_TOOLCHAIN}/bin/${CROSS_COMPILE_TRIPLET}-as"
LD="${OBGGCC_TOOLCHAIN}/bin/${CROSS_COMPILE_TRIPLET}-ld"
NM="${OBGGCC_TOOLCHAIN}/bin/${CROSS_COMPILE_TRIPLET}-nm"
RANLIB="${OBGGCC_TOOLCHAIN}/bin/${CROSS_COMPILE_TRIPLET}-ranlib"
STRIP="${OBGGCC_TOOLCHAIN}/bin/${CROSS_COMPILE_TRIPLET}-strip"
OBJCOPY="${OBGGCC_TOOLCHAIN}/bin/${CROSS_COMPILE_TRIPLET}-objcopy"
READELF="${OBGGCC_TOOLCHAIN}/bin/${CROSS_COMPILE_TRIPLET}-readelf"

export CROSS_COMPILE_TRIPLET \
	CC \
	CXX \
	AR \
	AS \
	LD \
	NM \
	RANLIB \
	STRIP \
	OBJCOPY \
	READELF
