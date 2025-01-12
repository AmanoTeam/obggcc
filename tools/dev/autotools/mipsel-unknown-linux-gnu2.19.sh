#/bin/bash

set -e

if [ -z "${OBGGCC_HOME}" ]; then
	OBGGCC_HOME="$(realpath "$(dirname "${0}")")/../.."
fi

set -u

CROSS_COMPILE_TRIPLET='mipsel-unknown-linux-gnu'
CROSS_COMPILE_SYSTEM='linux'
CROSS_COMPILE_ARCHITECTURE='mipsel'
CROSS_COMPILE_GLIBCVER='2.19'
CROSS_COMPILE_SYSROOT="${OBGGCC_HOME}/${CROSS_COMPILE_TRIPLET}${CROSS_COMPILE_GLIBCVER}"

CC="${OBGGCC_HOME}/bin/${CROSS_COMPILE_TRIPLET}${CROSS_COMPILE_GLIBCVER}-gcc"
CXX="${OBGGCC_HOME}/bin/${CROSS_COMPILE_TRIPLET}${CROSS_COMPILE_GLIBCVER}-g++"
AR="${OBGGCC_HOME}/bin/${CROSS_COMPILE_TRIPLET}-ar"
AS="${OBGGCC_HOME}/bin/${CROSS_COMPILE_TRIPLET}-as"
LD="${OBGGCC_HOME}/bin/${CROSS_COMPILE_TRIPLET}-ld"
NM="${OBGGCC_HOME}/bin/${CROSS_COMPILE_TRIPLET}-nm"
RANLIB="${OBGGCC_HOME}/bin/${CROSS_COMPILE_TRIPLET}-ranlib"
STRIP="${OBGGCC_HOME}/bin/${CROSS_COMPILE_TRIPLET}-strip"
OBJCOPY="${OBGGCC_HOME}/bin/${CROSS_COMPILE_TRIPLET}-objcopy"
READELF="${OBGGCC_HOME}/bin/${CROSS_COMPILE_TRIPLET}-readelf"

export \
	CROSS_COMPILE_TRIPLET \
	CROSS_COMPILE_SYSTEM \
	CROSS_COMPILE_ARCHITECTURE \
	CROSS_COMPILE_GLIBCVER \
	CROSS_COMPILE_SYSROOT \
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

set +eu
