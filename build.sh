#!/bin/bash

set -e
set -u

declare -r obggcc_revision="$(git rev-parse --short HEAD)"

declare -r current_source_directory="${PWD}"

declare -r toolchain_directory='/tmp/unknown-unknown-linux'
declare -r toolchain_tarball="${current_source_directory}/linux-cross.tar.xz"

declare -r gmp_tarball='/tmp/gmp.tar.xz'
declare -r gmp_directory='/tmp/gmp-6.2.1'

declare -r mpfr_tarball='/tmp/mpfr.tar.xz'
declare -r mpfr_directory='/tmp/mpfr-4.2.0'

declare -r mpc_tarball='/tmp/mpc.tar.gz'
declare -r mpc_directory='/tmp/mpc-1.3.1'

declare -r binutils_tarball='/tmp/binutils.tar.xz'
declare -r binutils_directory='/tmp/binutils-2.40'

declare -r gcc_tarball='/tmp/gcc.tar.xz'
declare -r gcc_directory='/tmp/gcc-12.2.0'

if ! [ -f "${gmp_tarball}" ]; then
	wget --no-verbose 'https://mirrors.kernel.org/gnu/gmp/gmp-6.2.1.tar.xz' --output-document="${gmp_tarball}"
	tar --directory="$(dirname "${gmp_directory}")" --extract --file="${gmp_tarball}"
fi

if ! [ -f "${mpfr_tarball}" ]; then
	wget --no-verbose 'https://mirrors.kernel.org/gnu/mpfr/mpfr-4.2.0.tar.xz' --output-document="${mpfr_tarball}"
	tar --directory="$(dirname "${mpfr_directory}")" --extract --file="${mpfr_tarball}"
fi

if ! [ -f "${mpc_tarball}" ]; then
	wget --no-verbose 'https://mirrors.kernel.org/gnu/mpc/mpc-1.3.1.tar.gz' --output-document="${mpc_tarball}"
	tar --directory="$(dirname "${mpc_directory}")" --extract --file="${mpc_tarball}"
fi

if ! [ -f "${binutils_tarball}" ]; then
	wget --no-verbose 'https://mirrors.kernel.org/gnu/binutils/binutils-2.40.tar.xz' --output-document="${binutils_tarball}"
	tar --directory="$(dirname "${binutils_directory}")" --extract --file="${binutils_tarball}"
fi

if ! [ -f "${gcc_tarball}" ]; then
	wget --no-verbose 'https://mirrors.kernel.org/gnu/gcc/gcc-12.2.0/gcc-12.2.0.tar.xz' --output-document="${gcc_tarball}"
	tar --directory="$(dirname "${gcc_directory}")" --extract --file="${gcc_tarball}"
fi

while read file; do
	sed -i 's/-O2/-Os -s -DNDEBUG/g' "${file}"
done <<< "$(find '/tmp' -type 'f' -wholename '*configure')"

[ -d "${gmp_directory}/build" ] || mkdir "${gmp_directory}/build"

cd "${gmp_directory}/build"

../configure \
	--prefix="${toolchain_directory}" \
	--enable-shared \
	--enable-static

make all --jobs="$(nproc)"
make install

[ -d "${mpfr_directory}/build" ] || mkdir "${mpfr_directory}/build"

cd "${mpfr_directory}/build"

../configure \
	--prefix="${toolchain_directory}" \
	--with-gmp="${toolchain_directory}" \
	--enable-shared \
	--enable-static

make all --jobs="$(nproc)"
make install

[ -d "${mpc_directory}/build" ] || mkdir "${mpc_directory}/build"

cd "${mpc_directory}/build"

../configure \
	--prefix="${toolchain_directory}" \
	--with-gmp="${toolchain_directory}" \
	--enable-shared \
	--enable-static

make all --jobs="$(nproc)"
make install

declare -ra targets=(
	'alpha-unknown-linux-gnu'
	'x86_64-unknown-linux-gnu'
	'i486-unknown-linux-gnu'
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

for target in "${targets[@]}"; do
	source "${current_source_directory}/${target}.sh"
	
	cd "$(mktemp --directory)"
	
	for package in "${packages[@]}"; do
		wget --no-verbose "${package}"
	done
	
	for file in *.deb; do
		ar x "${file}"
		
		if [ -f './data.tar.gz' ]; then
			declare filename='./data.tar.gz'
		else
			declare filename='./data.tar.xz'
		fi
		
		tar --extract --file="${filename}"
		
		rm "${filename}"
	done
	
	[ -d "${toolchain_directory}/${triple}" ] || mkdir --parent "${toolchain_directory}/${triple}"
	
	mv './usr/include' "${toolchain_directory}/${triple}"
	mv './usr/lib' "${toolchain_directory}/${triple}"
	
	if (( debian_release_major > 6 )); then
		mv "./lib/${host}/"* "${toolchain_directory}/${triple}/lib"
	else
		mv "./lib/"* "${toolchain_directory}/${triple}/lib"
	fi
	
	if (( debian_release_major > 6 )); then
		mv "${toolchain_directory}/${triple}/lib/${host}/"* "${toolchain_directory}/${triple}/lib"
		cp --recursive "${toolchain_directory}/${triple}/include/${host}/"* "${toolchain_directory}/${triple}/include"
		
		rm --recursive "${toolchain_directory}/${triple}/lib/${host}"
		rm --recursive "${toolchain_directory}/${triple}/include/${host}"
	fi
	
	cd "${toolchain_directory}/${triple}/lib"
	
	find . -type l | xargs ls -l | grep '/lib/' | awk '{print "unlink "$9" && ln -s ./$(basename "$11") ./$(basename "$9")"}' | bash
	
	if [ "${target}" == 'alpha-unknown-linux-gnu' ]; then
		echo -e "OUTPUT_FORMAT(${output_format})\nGROUP ( ./libc.so.6.1 ./libc_nonshared.a  AS_NEEDED ( ./${ld} ) )" > './libc.so'
	else
		echo -e "OUTPUT_FORMAT(${output_format})\nGROUP ( ./libc.so.6 ./libc_nonshared.a  AS_NEEDED ( ./${ld} ) )" > './libc.so'
	fi
	
	echo -e "OUTPUT_FORMAT(${output_format})\nGROUP ( ./libpthread.so.0 ./libpthread_nonshared.a  )" > './libpthread.so'
	
	if [[ "${target}" == mips*-unknown-linux-gnu ]] || [ "${target}" == 'powerpc-unknown-linux-gnu' ] || [ "${target}" == 's390-unknown-linux-gnu' ] || [ "${target}" == 'sparc-unknown-linux-gnu' ]; then
		patch --directory="${toolchain_directory}/${triple}" --strip='1' --input="${current_source_directory}/patches/linux_pim.patch"
	fi
	
	[ -d "${binutils_directory}/build" ] || mkdir "${binutils_directory}/build"
	
	cd "${binutils_directory}/build"
	rm --force --recursive ./*
	
	../configure \
		--target="${triple}" \
		--prefix="${toolchain_directory}" \
		--enable-gold \
		--enable-ld
	
	make all --jobs="$(nproc)"
	make install
	
	[ -d "${gcc_directory}/build" ] || mkdir "${gcc_directory}/build"
	cd "${gcc_directory}/build"
	
	rm --force --recursive ./*
	
	../configure \
		--target="${triple}" \
		--prefix="${toolchain_directory}" \
		--with-linker-hash-style='gnu' \
		--with-gmp="${toolchain_directory}" \
		--with-mpc="${toolchain_directory}" \
		--with-mpfr="${toolchain_directory}" \
		--with-system-zlib \
		--with-bugurl='https://github.com/AmanoTeam/obggcc/issues' \
		--enable-__cxa_atexit \
		--enable-cet='auto' \
		--enable-checking='release' \
		--enable-clocale='gnu' \
		--enable-default-ssp \
		--enable-gnu-indirect-function \
		--enable-gnu-unique-object \
		--enable-libstdcxx-backtrace \
		--enable-link-serialization='1' \
		--enable-linker-build-id \
		--enable-lto \
		--disable-multilib \
		--enable-plugin \
		--enable-shared \
		--enable-threads='posix' \
		--enable-libssp \
		--disable-libstdcxx-pch \
		--disable-werror \
		--enable-languages='c,c++' \
		--disable-libgomp \
		--disable-bootstrap \
		--without-headers \
		--enable-ld \
		--enable-gold \
		--with-pic \
		--with-gcc-major-version-only \
		--with-pkgversion="OBGGCC v0.1-${obggcc_revision}" \
		--with-sysroot="${toolchain_directory}/${triple}" \
		--with-native-system-header-dir='/include' \
		${extra_configure_flags}
	
	LD_LIBRARY_PATH="${toolchain_directory}/lib" PATH="${PATH}:${toolchain_directory}/bin" make CFLAGS_FOR_TARGET='-fno-stack-protector' CXXFLAGS_FOR_TARGET='-fno-stack-protector' all --jobs="$(nproc)"
	make install
done

tar --directory="$(dirname "${toolchain_directory}")" --create --file=- "$(basename "${toolchain_directory}")" |  xz --threads=0 --compress -9 > "${toolchain_tarball}"