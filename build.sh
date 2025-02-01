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
declare -r binutils_directory='/tmp/binutils-2.43.1'

declare -r gcc_tarball='/tmp/gcc.tar.gz'
declare -r gcc_directory='/tmp/gcc-14.2.0'

declare -r optflags='-Os'
declare -r linkflags='-Wl,-s'

declare -r max_jobs="$(($(nproc) * 17))"

declare -r asan_libraries=(
	'libasan'
	'libhwasan'
	'liblsan'
	'libtsan'
	'libubsan'
)

declare -r plugin_libraries=(
	'libcc1plugin'
	'libcp1plugin'
)

declare -r native_tools=(
	'c++'
	'cpp'
	'g++'
	'gcc'
	'gcc-ar'
	'gcc-nm'
	'gcc-ranlib'
	'gcov'
	'gcov-dump'
	'gcov-tool'
	'lto-dump'
)

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
	wget --no-verbose 'https://ftp.gnu.org/gnu/binutils/binutils-2.43.1.tar.xz' --output-document="${binutils_tarball}"
	tar --directory="$(dirname "${binutils_directory}")" --extract --file="${binutils_tarball}"
	
	patch --directory="${binutils_directory}" --strip='1' --input="${workdir}/patches/0001-Revert-gold-Use-char16_t-char32_t-instead-of-uint16_.patch"
	patch --directory="${binutils_directory}" --strip='1' --input="${workdir}/patches/0001-Disable-annoying-linker-warnings.patch"
fi

if ! [ -f "${gcc_tarball}" ]; then
	wget --no-verbose 'https://ftp.gnu.org/gnu/gcc/gcc-14.2.0/gcc-14.2.0.tar.xz' --output-document="${gcc_tarball}"
	tar --directory="$(dirname "${gcc_directory}")" --extract --file="${gcc_tarball}"
	
	patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0001-Revert-GCC-change-about-turning-Wimplicit-function-d.patch"
	patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0001-Fix-libsanitizer-build.patch"
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
	# 'ia64-unknown-linux-gnu'
	# 'alpha-unknown-linux-gnu'
	'x86_64-unknown-linux-gnu'
	# 'i386-unknown-linux-gnu'
	# 'arm-unknown-linux-gnueabi'
	# 'arm-unknown-linux-gnueabihf'
	# 'hppa-unknown-linux-gnu'
	# 'aarch64-unknown-linux-gnu'
	# 'mips-unknown-linux-gnu'
	# 'mipsel-unknown-linux-gnu'
	# 'powerpc-unknown-linux-gnu'
	# 's390-unknown-linux-gnu'
	# 's390x-unknown-linux-gnu'
	# 'sparc-unknown-linux-gnu'
	# 'powerpc64le-unknown-linux-gnu'
	# 'mips64el-unknown-linux-gnuabi64'
)

for target in "${targets[@]}"; do
	source "${workdir}/${target}.sh"
	
	cd "$(mktemp --directory)"
	
	declare sysroot_directory="$(basename "${sysroot}" '.tar.xz')"
	declare sysroot_file="$(basename "${sysroot}")"
	
	wget \
		--no-verbose \
		--output-document="${sysroot_file}" \
		"${sysroot}"
	
	tar \
		--extract \
		--file="${sysroot_file}"
	
	mv "${sysroot_directory}" "${toolchain_directory}/${triple}"
	
	rm --force --recursive ./*
	
	if [ "${CROSS_COMPILE_TRIPLET}" = "${triple}" ]; then
		cd "${toolchain_directory}/${triple}/include"
		ln --symbolic '../../include/c++' './c++'
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
		--with-pkgversion="OBGGCC v1.1-${obggcc_revision}" \
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
		--enable-libsanitizer \
		--enable-plugin \
		--disable-libgomp \
		--disable-bootstrap \
		--disable-libstdcxx-pch \
		--disable-werror \
		--disable-multilib \
		--disable-nls \
		--without-headers \
		${extra_configure_flags} \
		CFLAGS="${optflags}" \
		CXXFLAGS="${optflags}" \
		LDFLAGS="-Wl,-rpath-link,${OBGGCC_TOOLCHAIN}/${CROSS_COMPILE_TRIPLET}/lib ${linkflags}"
	
	LD_LIBRARY_PATH="${toolchain_directory}/lib" PATH="${PATH}:${toolchain_directory}/bin" make \
		CFLAGS_FOR_TARGET="${optflags} ${linkflags}" \
		CXXFLAGS_FOR_TARGET="${optflags} ${linkflags}" \
		gcc_cv_objdump="${CROSS_COMPILE_TRIPLET}-objdump" \
		all \
		--jobs="${max_jobs}"
	make install
	
	cd "${toolchain_directory}/${triple}/bin"
	
	for name in *; do
		rm "${name}"
		ln --symbolic "../../bin/${triple}-${name}" "${name}"
	done
	
	rm --recursive "${toolchain_directory}/share"
	
	patchelf --add-rpath '$ORIGIN/../../../../lib' "${toolchain_directory}/libexec/gcc/${triple}/"*'/cc1'
	patchelf --add-rpath '$ORIGIN/../../../../lib' "${toolchain_directory}/libexec/gcc/${triple}/"*'/cc1plus'
	patchelf --add-rpath '$ORIGIN/../../../../lib' "${toolchain_directory}/libexec/gcc/${triple}/"*'/lto1'
	
	for library in "${asan_libraries[@]}"; do
		patchelf --set-rpath '$ORIGIN' "${toolchain_directory}/lib"*"/${library}.so" || true
		patchelf --set-rpath '$ORIGIN' "${toolchain_directory}/${triple}/lib"*"/${library}.so" || true
	done
	
	for library in "${plugin_libraries[@]}"; do
		patchelf --set-rpath "\$ORIGIN/../../../../../${triple}/lib64:\$ORIGIN/../../../../../${triple}/lib:\$ORIGIN/../../../../../lib64:\$ORIGIN/../../../../../lib" "${toolchain_directory}/lib/gcc/${triple}/"*"/plugin/${library}.so"
	done
	
	for name in "${native_tools[@]}"; do
		declare file="${toolchain_directory}/bin/${name}"
		[ -f "${file}" ] && unlink "${file}"
	done
done

declare cc='gcc'

if ! (( is_native )); then
	cc="${CC}"
fi

declare -r sysroot_tarball='/tmp/sysroot.tar.xz'
declare -r executable='/tmp/gcc-wrapper'

declare -r share_directory="${toolchain_directory}/usr/local/share/obggcc"

declare -r libraries=(
	'libstdc++'
	'libatomic'
	'libssp'
	'libitm'
	'libsupc++'
	'libgcc'
)

declare -r bits=(
	''
	'64'
)

while read item; do
	declare glibc_version="$(jq '.glibc_version' <<< "${item}")"
	declare triplet="$(jq --raw-output '.triplet' <<< "${item}")"
	
	if ! [ -d "${toolchain_directory}/${triplet}" ]; then
		continue
	fi
	
	"${cc}" \
		"${workdir}/tools/gcc-wrapper/filesystem.c" \
		"${workdir}/tools/gcc-wrapper/main.c" \
		"${workdir}/tools/gcc-wrapper/path.c" \
		-Os \
		-s \
		-o "${executable}"
	
	cp "${executable}" "${toolchain_directory}/bin/${triplet}${glibc_version}-gcc"
	cp "${executable}" "${toolchain_directory}/bin/${triplet}${glibc_version}-g++"
	
	wget --no-verbose "https://github.com/AmanoTeam/debian-sysroot/releases/latest/download/${triplet}${glibc_version}.tar.xz" --output-document="${sysroot_tarball}"
	tar --directory="${toolchain_directory}" --extract --file="${sysroot_tarball}"
	
	pushd "${toolchain_directory}/${triplet}${glibc_version}/lib"
	
	for library in "${libraries[@]}"; do
		for bit in "${bits[@]}"; do
			for file in "../../${triplet}/lib${bit}/${library}"*; do
				if [[ "${file}" == *'*' ]]; then
					continue
				fi
				
				ln --symbolic "${file}" './'
			done
		done
	done
	
	pushd
done <<< "$(jq --compact-output '.[]' "${workdir}/submodules/debian-sysroot/dist.json")"

mkdir --parent "${share_directory}"

cp --recursive "${workdir}/tools/dev/"* "${share_directory}"
