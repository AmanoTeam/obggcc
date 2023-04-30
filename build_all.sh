#!/bin/bash

set -eu

declare -ra targets=(
	'alpha-unknown-linux-gnu'
	'x86_64-unknown-linux-gnu'
	'i386-unknown-linux-gnu'
	'arm-unknown-linux-gnueabi'
	'arm-unknown-linux-gnueabihf'
	'hppa-unknown-linux-gnu'
	'aarch64-unknown-linux-gnu'
	'mips-unknown-linux-gnu'
	'mipsel-unknown-linux-gnu'
	'powerpc-unknown-linux-gnu'
	's390-unknown-linux-gnu'
	's390x-unknown-linux-gnu'
	'sparc-unknown-linux-gnu'
	'powerpc64le-unknown-linux-gnu'
	'mips64el-unknown-linux-gnuabi64'
)

declare -r tarballs_directory="${PWD}/obggcc-tarballs"

[ -d "${tarballs_directory}" ] || mkdir "${tarballs_directory}"

source './tools/setup_toolchain.sh'

for target in "${targets[@]}"; do
	bash './build.sh' "${target}"
	
	declare tarball_filename="${tarballs_directory}/${target}.tar.xz"
	
	tar --directory='/tmp' --create --file=- 'obggcc' |  xz --threads=0 --compress -9 > "${tarball_filename}"
	sha256sum "${tarball_filename}" > "${tarball_filename}.sha256"
	
	rm --recursive '/tmp/obggcc'
done
