#!/bin/bash

declare PKG_CONFIG_PATH=''
declare PKG_CONFIG_LIBDIR=''
declare PKG_CONFIG_SYSROOT_DIR=''

declare -r TRIPLET="${CROSS_COMPILE_TRIPLET/-unknown/}"

declare -r SYSROOT="${CROSS_COMPILE_SYSROOT}"
declare -r NZ_SYSROOT="${SYSROOT}/lib/nouzen/sysroot"

if [ "${OBGGCC_NZ}" = '1' ] || [ "${OBGGCC_NZ}" = 'true' ]; then
	PKG_CONFIG_PATH+="${NZ_SYSROOT}/usr/lib/${TRIPLET}/pkgconfig:"
	PKG_CONFIG_PATH+="${NZ_SYSROOT}/usr/lib/pkgconfig:"
	PKG_CONFIG_PATH+="${NZ_SYSROOT}/usr/share/pkgconfig:"
	
	PKG_CONFIG_LIBDIR="${PKG_CONFIG_PATH}"
	
	PKG_CONFIG_SYSROOT_DIR="${NZ_SYSROOT}"
fi

declare -r \
	PKG_CONFIG_PATH \
	PKG_CONFIG_LIBDIR \
	PKG_CONFIG_SYSROOT_DIR

export \
	PKG_CONFIG_PATH \
	PKG_CONFIG_LIBDIR \
	PKG_CONFIG_SYSROOT_DIR

pkg-config ${@}
