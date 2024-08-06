#!/bin/bash

set -eu

declare -r obggcc_revision="$(git rev-parse --short HEAD)"

declare -r workdir="${PWD}"

declare -r toolchain_directory='/tmp/obggcc'

declare -r gmp_tarball='/tmp/gmp.tar.xz'
declare -r gmp_directory='/tmp/gmp-6.3.0'

declare -r mpfr_tarball='/tmp/mpfr.tar.xz'
declare -r mpfr_directory='/tmp/mpfr-4.2.1'

declare -r mpc_tarball='/tmp/mpc.tar.gz'
declare -r mpc_directory='/tmp/mpc-1.3.1'

declare -r binutils_tarball='/tmp/binutils.tar.xz'
declare -r binutils_directory='/tmp/binutils-2.43'

declare -r gcc_tarball='/tmp/gcc.tar.gz'
declare -r gcc_directory='/tmp/gcc-14.2.0'

declare -r optflags='-Os'
declare -r linkflags='-Wl,-s'

declare -r max_jobs="$(($(nproc) * 17))"

declare build_type="${1}"

if [ -z "${build_type}" ]; then
	build_type='native'
fi

declare is_native='0'

if [ "${build_type}" == 'native' ]; then
	is_native='1'
fi

declare OBGGCC_TOOLCHAIN='/tmp/obggcc-toolchain'
declare CROSS_COMPILE_TRIPLET=''

declare cross_compile_flags=''

if ! (( is_native )); then
	source "./toolchains/${build_type}.sh"
	cross_compile_flags+="--host=${CROSS_COMPILE_TRIPLET}"
fi

if ! [ -f "${gmp_tarball}" ]; then
	wget --no-verbose 'https://ftp.gnu.org/gnu/gmp/gmp-6.3.0.tar.xz' --output-document="${gmp_tarball}"
	tar --directory="$(dirname "${gmp_directory}")" --extract --file="${gmp_tarball}"
fi

if ! [ -f "${mpfr_tarball}" ]; then
	wget --no-verbose 'https://ftp.gnu.org/gnu/mpfr/mpfr-4.2.1.tar.xz' --output-document="${mpfr_tarball}"
	tar --directory="$(dirname "${mpfr_directory}")" --extract --file="${mpfr_tarball}"
fi

if ! [ -f "${mpc_tarball}" ]; then
	wget --no-verbose 'https://ftp.gnu.org/gnu/mpc/mpc-1.3.1.tar.gz' --output-document="${mpc_tarball}"
	tar --directory="$(dirname "${mpc_directory}")" --extract --file="${mpc_tarball}"
fi

if ! [ -f "${binutils_tarball}" ]; then
	wget --no-verbose 'https://ftp.gnu.org/gnu/binutils/binutils-2.43.tar.xz' --output-document="${binutils_tarball}"
	tar --directory="$(dirname "${binutils_directory}")" --extract --file="${binutils_tarball}"
	
	patch --directory="${binutils_directory}" --strip='1' --input="${workdir}/patches/0001-Revert-gold-Use-char16_t-char32_t-instead-of-uint16_.patch"
fi

if ! [ -f "${gcc_tarball}" ]; then
	wget --no-verbose 'https://ftp.gnu.org/gnu/gcc/gcc-14.2.0/gcc-14.2.0.tar.xz' --output-document="${gcc_tarball}"
	tar --directory="$(dirname "${gcc_directory}")" --extract --file="${gcc_tarball}"
fi

[ -d "${gmp_directory}/build" ] || mkdir "${gmp_directory}/build"

cd "${gmp_directory}/build"
rm --force --recursive ./*

../configure \
	--host="${CROSS_COMPILE_TRIPLET}" \
	--prefix="${toolchain_directory}" \
	--enable-shared \
	--enable-static \
	CFLAGS="${optflags}" \
	CXXFLAGS="${optflags}" \
	LDFLAGS="${linkflags}"

make all --jobs
make install

[ -d "${mpfr_directory}/build" ] || mkdir "${mpfr_directory}/build"

cd "${mpfr_directory}/build"
rm --force --recursive ./*

../configure \
	--host="${CROSS_COMPILE_TRIPLET}" \
	--prefix="${toolchain_directory}" \
	--with-gmp="${toolchain_directory}" \
	--enable-shared \
	--enable-static \
	CFLAGS="${optflags}" \
	CXXFLAGS="${optflags}" \
	LDFLAGS="${linkflags}"

make all --jobs
make install

[ -d "${mpc_directory}/build" ] || mkdir "${mpc_directory}/build"

cd "${mpc_directory}/build"
rm --force --recursive ./*

../configure \
	--host="${CROSS_COMPILE_TRIPLET}" \
	--prefix="${toolchain_directory}" \
	--with-gmp="${toolchain_directory}" \
	--enable-shared \
	--enable-static \
	CFLAGS="${optflags} -fpermissive" \
	CXXFLAGS="${optflags}" \
	LDFLAGS="${linkflags}"

make all --jobs
make install

declare -ra targets=(
	'ia64-unknown-linux-gnu'
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

for target in "${targets[@]}"; do
	source "${workdir}/${target}.sh"
	
	cd "$(mktemp --directory)"
	
	for package in "${packages[@]}"; do
		curl --connect-timeout '10' --retry '15' --retry-all-errors --fail --silent --location --remote-name --url "${package}"
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
	
	cp --recursive './usr/include' "${toolchain_directory}/${triple}"
	cp --recursive './usr/lib' "${toolchain_directory}/${triple}"
	
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
	
	if [ "${target}" == 'alpha-unknown-linux-gnu' ] || [ "${target}" == 'ia64-unknown-linux-gnu' ]; then
		echo -e "OUTPUT_FORMAT(${output_format})\nGROUP ( ./libc.so.6.1 ./libc_nonshared.a  AS_NEEDED ( ./${ld} ) )" > './libc.so'
	else
		echo -e "OUTPUT_FORMAT(${output_format})\nGROUP ( ./libc.so.6 ./libc_nonshared.a  AS_NEEDED ( ./${ld} ) )" > './libc.so'
	fi
	
	echo -e "OUTPUT_FORMAT(${output_format})\nGROUP ( ./libpthread.so.0 ./libpthread_nonshared.a  )" > './libpthread.so'
	
	if [[ "${target}" == mips*-unknown-linux-gnu ]] || [ "${target}" == 'powerpc-unknown-linux-gnu' ] || [ "${target}" == 's390-unknown-linux-gnu' ] || [ "${target}" == 'sparc-unknown-linux-gnu' ]; then
		patch --directory="${toolchain_directory}/${triple}" --strip='1' --input="${workdir}/patches/linux_pim.patch"
	fi
	
	cd "${toolchain_directory}/${triple}/include"
	
	if ! (( is_native )); then
		CC="${triple}-gcc" python "${workdir}/tools/make_builtins.py"
		
		if [ -f './builtin_ctype.h' ]; then
			echo '#include <builtin_ctype.h>' >> './ctype.h'
		fi
		
		if [ -f './builtin_math.h' ]; then
			echo '#include <builtin_math.h>' >> './math.h'
		fi
		
		if [ -f './builtin_stdio.h' ]; then
			echo '#include <builtin_stdio.h>' >> './stdio.h'
		fi
		
		if [ -f './builtin_complex.h' ]; then
			echo '#include <builtin_complex.h>' >> './complex.h'
		fi
		
		if [ -f './builtin_stdlib.h' ]; then
			echo '#include <builtin_stdlib.h>' >> './stdlib.h'
		fi
	fi
	
	[ -d "${binutils_directory}/build" ] || mkdir "${binutils_directory}/build"
	
	cd "${binutils_directory}/build"
	rm --force --recursive ./*
	
	../configure \
		--host="${CROSS_COMPILE_TRIPLET}" \
		--target="${triple}" \
		--prefix="${toolchain_directory}" \
		--enable-gold \
		--enable-ld \
		--enable-lto \
		--disable-gprofng \
		--with-static-standard-libraries \
		--program-prefix="${triple}-" \
		--with-sysroot="${toolchain_directory}/${triple}" \
		CFLAGS="${optflags}" \
		CXXFLAGS="${optflags}" \
		LDFLAGS="${linkflags}"
	
	make all --jobs
	make install
	
	[ -d "${gcc_directory}/build" ] || mkdir "${gcc_directory}/build"
	cd "${gcc_directory}/build"
	
	rm --force --recursive ./*
	
	../configure \
		--host="${CROSS_COMPILE_TRIPLET}" \
		--target="${triple}" \
		--prefix="${toolchain_directory}" \
		--with-linker-hash-style='gnu' \
		--with-gmp="${toolchain_directory}" \
		--with-mpc="${toolchain_directory}" \
		--with-mpfr="${toolchain_directory}" \
		--with-static-standard-libraries \
		--with-bugurl='https://github.com/AmanoTeam/obggcc/issues' \
		--with-gcc-major-version-only \
		--with-pkgversion="OBGGCC v0.6-${obggcc_revision}" \
		--with-sysroot="${toolchain_directory}/${triple}" \
		--with-native-system-header-dir='/include' \
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
		--enable-shared \
		--enable-threads='posix' \
		--enable-libssp \
		--enable-languages='c,c++' \
		--enable-ld \
		--enable-gold \
		--disable-libgomp \
		--disable-bootstrap \
		--disable-libstdcxx-pch \
		--disable-werror \
		--disable-multilib \
		--disable-plugin \
		--disable-nls \
		--without-headers \
		${extra_configure_flags} \
		CFLAGS="${optflags}" \
		CXXFLAGS="${optflags}" \
		LDFLAGS="-Wl,-rpath-link,${OBGGCC_TOOLCHAIN}/${CROSS_COMPILE_TRIPLET}/lib ${linkflags}"
	
	LD_LIBRARY_PATH="${toolchain_directory}/lib" PATH="${PATH}:${toolchain_directory}/bin" make \
		CFLAGS_FOR_TARGET="${optflags} ${linkflags}" \
		CXXFLAGS_FOR_TARGET="${optflags} ${linkflags}" \
		all \
		--jobs="${max_jobs}"
	make install
	
	cd "${toolchain_directory}/${triple}/bin"
	
	for name in *; do
		rm "${name}"
		ln -s "../../bin/${triple}-${name}" "${name}"
	done
	
	rm --recursive "${toolchain_directory}/share"
	
	patchelf --add-rpath '$ORIGIN/../../../../lib' "${toolchain_directory}/libexec/gcc/${triple}/"*'/cc1'
	patchelf --add-rpath '$ORIGIN/../../../../lib' "${toolchain_directory}/libexec/gcc/${triple}/"*'/cc1plus'
	patchelf --add-rpath '$ORIGIN/../../../../lib' "${toolchain_directory}/libexec/gcc/${triple}/"*'/lto1'
done
