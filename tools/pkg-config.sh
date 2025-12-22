#!/bin/bash

declare PKG_CONFIG_PATH=''
declare PKG_CONFIG_LIBDIR=''
declare PKG_CONFIG_SYSROOT_DIR=''

declare -r EXECUTABLE_PATH="$([ -n "${BASH_SOURCE}" ] && realpath "${BASH_SOURCE[0]}" || realpath "${0}")"
declare -r EXECUTABLE_NAME="$(basename "${EXECUTABLE_PATH}")"

declare TRIPLET="${EXECUTABLE_NAME/-pkg-config/}"

declare -r SYSROOT="$(realpath "$(dirname "${EXECUTABLE_PATH}")/../${TRIPLET}")"
declare -r NZ_SYSROOT="${SYSROOT}/lib/nouzen/sysroot"

declare LIBC_VERSION=''

if [[ "${TRIPLET}" = *'linux-gnu'* ]]; then
	LIBC_VERSION="$(awk -F  'linux-gnu' '{print $2}' <<< "${TRIPLET}")"
elif [[ "${TRIPLET}" = *'linux-android'* ]]; then
	LIBC_VERSION="$(awk -F  'linux-android' '{print $2}' <<< "${TRIPLET}")"
else
	echo "unknown target: ${TRIPLET}" >&2
	exit '1'
fi

TRIPLET="${TRIPLET/${LIBC_VERSION}/}"
TRIPLET="${TRIPLET/-unknown/}"

declare -r LIBC_VERSION
declare -r TRIPLET

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

exec pkg-config "${@}"
