#!/bin/bash

set -eu

declare -r revision="$(git rev-parse --short HEAD)"

declare -r workdir="${PWD}"

declare -r toolchain_directory='/tmp/obggcc'
declare -r share_directory="${toolchain_directory}/usr/local/share/obggcc"

declare -r environment="LD_LIBRARY_PATH=${toolchain_directory}/lib PATH=${PATH}:${toolchain_directory}/bin"

declare -r autotools_directory="${share_directory}/autotools"

declare -r gmp_tarball='/tmp/gmp.tar.xz'
declare -r gmp_directory='/tmp/gmp-6.3.0'

declare -r mpfr_tarball='/tmp/mpfr.tar.xz'
declare -r mpfr_directory='/tmp/mpfr-4.2.2'

declare -r mpc_tarball='/tmp/mpc.tar.gz'
declare -r mpc_directory='/tmp/mpc-1.3.1'

declare -r isl_tarball='/tmp/isl.tar.xz'
declare -r isl_directory='/tmp/isl-0.27'

declare -r binutils_tarball='/tmp/binutils.tar.xz'
declare -r binutils_directory='/tmp/binutils-with-gold-2.44'

declare -r gcc_major='15'

declare -r gcc_tarball='/tmp/gcc.tar.xz'
declare -r gcc_directory="/tmp/gcc-releases-gcc-${gcc_major}"

declare -r libsanitizer_tarball='/tmp/libsanitizer.tar.xz'
declare -r libsanitizer_directory='/tmp/libsanitizer'

declare -r gdb_tarball='/tmp/gdb.tar.xz'
declare -r gdb_directory='/tmp/gdb'

declare -r nz_tarball='/tmp/nz.tar.xz'
declare nz_directory=""

declare -r zstd_tarball='/tmp/zstd.tar.gz'
declare -r zstd_directory='/tmp/zstd-dev'

declare -r pieflags='-fPIE'
declare -r ccflags='-w -O2 -Xlinker --allow-multiple-definition'
declare -r linkflags='-Xlinker -s'

declare hccflags="-I${toolchain_directory}/include -L${toolchain_directory}/lib"
declare hlinkflags="-Xlinker -rpath-link -Xlinker ${toolchain_directory}/lib ${linkflags}"

declare -r max_jobs='40'

declare -r sysroot_tarball='/tmp/sysroot.tar.xz'
declare -r gcc_wrapper='/tmp/gcc-wrapper'

declare gdb='1'
declare nz='1'

declare -ra plugin_libraries=(
	'libcc1plugin'
	'libcp1plugin'
)

declare -ra symlink_tools=(
	'addr2line'
	'ar'
	'as'
	'c++filt'
	'cpp'
	'elfedit'
	'dwp'
	'gcc-ar'
	'gcc-nm'
	'gcc-ranlib'
	'gcov'
	'gcov-dump'
	'gcov-tool'
	'gprof'
	'ld'
	'ld.bfd'
	'ld.gold'
	'lto-dump'
	'nm'
	'objcopy'
	'objdump'
	'ranlib'
	'readelf'
	'size'
	'strings'
	'strip'
)

declare -ra libraries=(
	'libstdc++'
	'libatomic'
	'libssp'
	'libitm'
	'libsupc++'
	'libgcc'
	'libm2cor'
	'libm2iso'
	'libm2log'
	'libm2min'
	'libm2pim'
	'libobjc'
	'libgfortran'
	'libasan'
	'libhwasan'
	'liblsan'
	'libtsan'
	'libubsan'
)

declare -ra bits=(
	''
	'64'
)

declare -r languages='c,c++'

declare -ra targets=(
	'ia64-unknown-linux-gnu'
	'mips-unknown-linux-gnu'
	'mips64el-unknown-linux-gnuabi64'
	'mipsel-unknown-linux-gnu'
	'powerpc-unknown-linux-gnu'
	'powerpc64le-unknown-linux-gnu'
	's390-unknown-linux-gnu'
	's390x-unknown-linux-gnu'
	'sparc-unknown-linux-gnu'
	'x86_64-unknown-linux-gnu'
	'alpha-unknown-linux-gnu'
	'aarch64-unknown-linux-gnu'
	'arm-unknown-linux-gnueabi'
	'arm-unknown-linux-gnueabihf'
	'hppa-unknown-linux-gnu'
	'i386-unknown-linux-gnu'
)

declare build_type="${1}"

if [ -z "${build_type}" ]; then
	build_type='native'
fi

declare is_native='0'

if [ "${build_type}" = 'native' ]; then
	is_native='1'
fi

set +u

if [ -z "${CROSS_COMPILE_TRIPLET}" ]; then
	declare CROSS_COMPILE_TRIPLET=''
fi

set -u

declare -r \
	build_type \
	is_native

if ! [ -f "${gmp_tarball}" ]; then
	curl \
		--url 'https://ftp.gnu.org/gnu/gmp/gmp-6.3.0.tar.xz' \
		--retry '30' \
		--retry-all-errors \
		--retry-delay '0' \
		--retry-max-time '0' \
		--location \
		--silent \
		--output "${gmp_tarball}"
	
	tar \
		--directory="$(dirname "${gmp_directory}")" \
		--extract \
		--file="${gmp_tarball}"
	
	patch --directory="${gmp_directory}" --strip='1' --input="${workdir}/patches/0001-Remove-hardcoded-RPATH-and-versioned-SONAME-from-libgmp.patch"
fi

if ! [ -f "${mpfr_tarball}" ]; then
	curl \
		--url 'https://ftp.gnu.org/gnu/mpfr/mpfr-4.2.2.tar.xz' \
		--retry '30' \
		--retry-all-errors \
		--retry-delay '0' \
		--retry-max-time '0' \
		--location \
		--silent \
		--output "${mpfr_tarball}"
	
	tar \
		--directory="$(dirname "${mpfr_directory}")" \
		--extract \
		--file="${mpfr_tarball}"
	
	patch --directory="${mpfr_directory}" --strip='1' --input="${workdir}/patches/0001-Remove-hardcoded-RPATH-and-versioned-SONAME-from-libmpfr.patch"
fi

if ! [ -f "${mpc_tarball}" ]; then
	curl \
		--url 'https://ftp.gnu.org/gnu/mpc/mpc-1.3.1.tar.gz' \
		--retry '30' \
		--retry-all-errors \
		--retry-delay '0' \
		--retry-max-time '0' \
		--location \
		--silent \
		--output "${mpc_tarball}"
	
	tar \
		--directory="$(dirname "${mpc_directory}")" \
		--extract \
		--file="${mpc_tarball}"
	
	patch --directory="${mpc_directory}" --strip='1' --input="${workdir}/patches/0001-Remove-hardcoded-RPATH-and-versioned-SONAME-from-libmpc.patch"
fi

if ! [ -f "${isl_tarball}" ]; then
	curl \
		--url 'https://libisl.sourceforge.io/isl-0.27.tar.xz' \
		--retry '30' \
		--retry-all-errors \
		--retry-delay '0' \
		--retry-max-time '0' \
		--location \
		--silent \
		--output "${isl_tarball}"
	
	tar \
		--directory="$(dirname "${isl_directory}")" \
		--extract \
		--file="${isl_tarball}"
	
	patch --directory="${isl_directory}" --strip='1' --input="${workdir}/patches/0001-Remove-hardcoded-RPATH-and-versioned-SONAME-from-libisl.patch"
fi

if ! [ -f "${binutils_tarball}" ]; then
	curl \
		--url 'https://ftp.gnu.org/gnu/binutils/binutils-with-gold-2.44.tar.xz' \
		--retry '30' \
		--retry-all-errors \
		--retry-delay '0' \
		--retry-max-time '0' \
		--location \
		--silent \
		--output "${binutils_tarball}"
	
	tar \
		--directory="$(dirname "${binutils_directory}")" \
		--extract \
		--file="${binutils_tarball}"
	
	patch --directory="${binutils_directory}" --strip='1' --input="${workdir}/patches/0001-Revert-gold-Use-char16_t-char32_t-instead-of-uint16_.patch"
	patch --directory="${binutils_directory}" --strip='1' --input="${workdir}/patches/0001-Disable-annoying-linker-warnings.patch"
	patch --directory="${binutils_directory}" --strip='1' --input="${workdir}/patches/0001-Add-relative-RPATHs-to-binutils-host-tools.patch"
fi

if ! [ -f "${zstd_tarball}" ]; then
	curl \
		--url 'https://github.com/facebook/zstd/archive/refs/heads/dev.tar.gz' \
		--retry '30' \
		--retry-all-errors \
		--retry-delay '0' \
		--retry-max-time '0' \
		--location \
		--silent \
		--output "${zstd_tarball}"
	
	tar \
		--directory="$(dirname "${zstd_directory}")" \
		--extract \
		--file="${zstd_tarball}"
fi

if ! [ -f "${gcc_tarball}" ]; then
	curl \
		--url 'https://github.com/gcc-mirror/gcc/archive/refs/heads/releases/gcc-15.tar.gz' \
		--retry '30' \
		--retry-all-errors \
		--retry-delay '0' \
		--retry-max-time '0' \
		--location \
		--silent \
		--output "${gcc_tarball}"
	
	tar \
		--directory="$(dirname "${gcc_directory}")" \
		--extract \
		--file="${gcc_tarball}"
	
	patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0001-Revert-GCC-change-about-turning-Wimplicit-function-d.patch"
	patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0001-Fix-libsanitizer-build.patch"
	patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0001-Change-the-default-language-version-for-C-compilatio.patch"
	patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0001-Turn-Wimplicit-int-back-into-an-warning.patch"
	patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0001-Turn-Wint-conversion-back-into-an-warning.patch"
	patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0001-Fix-libgcc-build-on-arm.patch"
	patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0001-Add-relative-RPATHs-to-GCC-host-tools.patch"
fi

# Follow Debian's approach for removing hardcoded RPATH from binaries
# https://wiki.debian.org/RpathIssue
sed \
	--in-place \
	--regexp-extended \
	's/(hardcode_into_libs)=.*$/\1=no/' \
	"${isl_directory}/configure" \
	"${mpc_directory}/configure" \
	"${mpfr_directory}/configure" \
	"${gmp_directory}/configure" \
	"${gcc_directory}/libsanitizer/configure"

[ -d "${gmp_directory}/build" ] || mkdir "${gmp_directory}/build"

cd "${gmp_directory}/build"
rm --force --recursive ./*

../configure \
	--host="${CROSS_COMPILE_TRIPLET}" \
	--prefix="${toolchain_directory}" \
	--enable-shared \
	--disable-static \
	CFLAGS="${ccflags} ${hccflags}" \
	CXXFLAGS="${ccflags} ${hccflags}" \
	LDFLAGS="${linkflags} ${hlinkflags}"

make all --jobs
make install

[ -d "${mpfr_directory}/build" ] || mkdir "${mpfr_directory}/build"

cd "${mpfr_directory}/build"
rm --force --recursive ./*

../configure \
	--host="${CROSS_COMPILE_TRIPLET}" \
	--prefix="${toolchain_directory}" \
	--enable-shared \
	--disable-static \
	CFLAGS="${ccflags} ${hccflags}" \
	CXXFLAGS="${ccflags} ${hccflags}" \
	LDFLAGS="${linkflags} ${hlinkflags}"

make all --jobs
make install

[ -d "${mpc_directory}/build" ] || mkdir "${mpc_directory}/build"

cd "${mpc_directory}/build"
rm --force --recursive ./*

../configure \
	--host="${CROSS_COMPILE_TRIPLET}" \
	--prefix="${toolchain_directory}" \
	--enable-shared \
	--disable-static \
	CFLAGS="${ccflags} ${hccflags}" \
	CXXFLAGS="${ccflags} ${hccflags}" \
	LDFLAGS="${linkflags} ${hlinkflags}"

make all --jobs
make install

[ -d "${isl_directory}/build" ] || mkdir "${isl_directory}/build"

cd "${isl_directory}/build"
rm --force --recursive ./*

../configure \
	--host="${CROSS_COMPILE_TRIPLET}" \
	--prefix="${toolchain_directory}" \
	--with-gmp-prefix="${toolchain_directory}" \
	--enable-shared \
	--disable-static \
	CFLAGS="${pieflags} ${ccflags} ${hccflags}" \
	CXXFLAGS="${pieflags} ${ccflags} ${hccflags}" \
	LDFLAGS="${linkflags} ${hlinkflags}"

make all --jobs
make install

[ -d "${zstd_directory}/.build" ] || mkdir "${zstd_directory}/.build"

cd "${zstd_directory}/.build"
rm --force --recursive ./*

cmake \
	-S "${zstd_directory}/build/cmake" \
	-B "${PWD}" \
	-DCMAKE_C_FLAGS="-DZDICT_QSORT=ZDICT_QSORT_MIN ${ccflags}" \
	-DCMAKE_INSTALL_PREFIX="${toolchain_directory}" \
	-DBUILD_SHARED_LIBS=ON \
	-DZSTD_BUILD_PROGRAMS=OFF \
	-DZSTD_BUILD_TESTS=OFF \
	-DZSTD_BUILD_STATIC=OFF \
	-DCMAKE_PLATFORM_NO_VERSIONED_SONAME=ON

cmake --build "${PWD}"
cmake --install "${PWD}" --strip

# We prefer symbolic links over hard links.
cp "${workdir}/tools/ln.sh" '/tmp/ln'

export PATH="/tmp:${PATH}"

# The gold linker build incorrectly detects ffsll() as unsupported.
if [[ "${CROSS_COMPILE_TRIPLET}" == *'-android'* ]]; then
	export ac_cv_func_ffsll=yes
fi

declare cc='gcc'
declare readelf='readelf'

if ! (( is_native )); then
	cc="${CC}"
	readelf="${READELF}"
fi

"${cc}" \
	"${workdir}/tools/gcc-wrapper/"*'/'*'.c' \
	"${workdir}/tools/gcc-wrapper/"*".c" \
	-I "${workdir}/tools/gcc-wrapper" \
	${ccflags} \
	${linkflags} \
	-D OBGGCC \
	-o "${gcc_wrapper}"

for target in "${targets[@]}"; do
	declare specs='-Xlinker --disable-new-dtags'
	
	source "${workdir}/${target}.sh"
	
	cd "$(mktemp --directory)"
	
	declare sysroot_directory="$(basename "${sysroot}" '.tar.xz')"
	declare sysroot_file="$(basename "${sysroot}")"
	
	echo "- Fetching data from '${sysroot}'"
	
	curl \
		--url "${sysroot}" \
		--retry '30' \
		--retry-all-errors \
		--retry-delay '0' \
		--retry-max-time '0' \
		--location \
		--silent \
		--output "${sysroot_file}"
	
	tar \
		--extract \
		--file="${sysroot_file}"
	
	mv "${sysroot_directory}" "${toolchain_directory}/${triplet}"
	
	rm --force --recursive ./*
	
	if [ "${CROSS_COMPILE_TRIPLET}" = "${triplet}" ]; then
		cd "${toolchain_directory}/${triplet}/include"
		ln --symbolic '../../include/c++' './c++'
	fi
	
	[ -d "${binutils_directory}/build" ] || mkdir "${binutils_directory}/build"
	
	cd "${binutils_directory}/build"
	rm --force --recursive ./*
	
	../configure \
		--host="${CROSS_COMPILE_TRIPLET}" \
		--target="${triplet}" \
		--prefix="${toolchain_directory}" \
		--enable-gold \
		--enable-ld \
		--enable-lto \
		--enable-plugins \
		--disable-gprofng \
		--program-prefix="${triplet}-" \
		--with-sysroot="${toolchain_directory}/${triplet}" \
		--without-static-standard-libraries \
		--with-zstd="${toolchain_directory}" \
		CFLAGS="${ccflags} ${hccflags}" \
		CXXFLAGS="${ccflags} ${hccflags}" \
		LDFLAGS="${linkflags} ${hlinkflags}"
	
	make all --jobs
	make install
	
	if [[ "${triplet}" != 'powerpc64'* ]]; then
		specs+=' %{!fno-plt:%{!fplt:-fno-plt}}'
	fi
	
	[ -d "${gcc_directory}/build" ] || mkdir "${gcc_directory}/build"
	
	cd "${gcc_directory}/build"
	rm --force --recursive ./*
	
	../configure \
		--host="${CROSS_COMPILE_TRIPLET}" \
		--target="${triplet}" \
		--prefix="${toolchain_directory}" \
		--with-linker-hash-style='both' \
		--with-gmp="${toolchain_directory}" \
		--with-mpc="${toolchain_directory}" \
		--with-mpfr="${toolchain_directory}" \
		--with-isl="${toolchain_directory}" \
		--with-zstd="${toolchain_directory}" \
		--with-bugurl='https://github.com/AmanoTeam/obggcc/issues' \
		--with-gcc-major-version-only \
		--with-pkgversion="OBGGCC v2.7-${revision}" \
		--with-sysroot="${toolchain_directory}/${triplet}" \
		--with-native-system-header-dir='/include' \
		--with-default-libstdcxx-abi='new' \
		--enable-__cxa_atexit \
		--enable-cet='auto' \
		--enable-checking='release' \
		--enable-clocale='gnu' \
		--enable-default-pie \
		--enable-default-ssp \
		--enable-gnu-indirect-function \
		--enable-gnu-unique-object \
		--enable-libstdcxx-backtrace \
		--enable-libstdcxx-filesystem-ts \
		--enable-libstdcxx-static-eh-pool \
		--with-libstdcxx-zoneinfo='static' \
		--with-libstdcxx-lock-policy='atomic' \
		--enable-link-serialization='1' \
		--enable-linker-build-id \
		--enable-lto \
		--enable-shared \
		--enable-threads='posix' \
		--enable-libstdcxx-threads \
		--enable-libssp \
		--enable-languages="${languages}" \
		--enable-ld \
		--enable-gold \
		--enable-plugin \
		--enable-libstdcxx-time='rt' \
		--enable-cxx-flags="${linkflags} ${extra_cxx_flags}" \
		--enable-host-pie \
		--enable-host-shared \
		--with-specs="${specs}" \
		--disable-libsanitizer \
		--disable-libgomp \
		--disable-bootstrap \
		--disable-libstdcxx-pch \
		--disable-werror \
		--disable-multilib \
		--disable-nls \
		--without-headers \
		--without-static-standard-libraries \
		${extra_configure_flags} \
		CFLAGS="${ccflags} ${hccflags}" \
		CXXFLAGS="${ccflags} ${hccflags}" \
		LDFLAGS="${linkflags} ${hlinkflags}"
	
	cflags_for_target="${ccflags} ${linkflags}"
	cxxflags_for_target="${ccflags} ${linkflags}"
	ldflags_for_target="${linkflags}"
	
	declare args=''
	
	if (( is_native )); then
		args+="${environment}"
	fi
	
	env ${args} make \
		CFLAGS_FOR_TARGET="${cflags_for_target}" \
		CXXFLAGS_FOR_TARGET="${cxxflags_for_target}" \
		LDFLAGS_FOR_TARGET="${ldflags_for_target}" \
		gcc_cv_objdump="${CROSS_COMPILE_TRIPLET}-objdump" \
		all \
		--jobs="${max_jobs}"
	make install
	
	if [[ "${triplet}" = 'sparc-'* ]] || [[ "${triplet}" = 's390-'* ]] || [[ "${triplet}" = 'powerpc-'* ]] || [[ "${triplet}" = 'hppa-'* ]] || [[ "${triplet}" = 'alpha-'* ]]; then
		patch \
			--directory="${toolchain_directory}/${triplet}/include/c++/${gcc_major}/${triplet}" \
			--strip='1' \
			--input="${workdir}/patches/0001-Fix-C99-math-functions-availability.patch"
	fi
	
	cd "${toolchain_directory}/lib/bfd-plugins"
	
	if ! [ -f './liblto_plugin.so' ]; then
		ln --symbolic "../../libexec/gcc/${triplet}/${gcc_major}/liblto_plugin.so" './'
	fi
	
	if ! (( is_native )); then
		if ! [ "${triplet}" = 'mips64el-unknown-linux-gnuabi64' ]; then
			rm "${toolchain_directory}/${triplet}/lib/"*.o
		fi
	fi
	
	if [ "${CROSS_COMPILE_TRIPLET}" = "${triplet}" ]; then
		cd "${toolchain_directory}/${triplet}/lib"
			
		for library in "${libraries[@]}"; do
			for bit in "${bits[@]}"; do
				for file in "../../lib${bit}/${library}"*; do
					if [[ "${file}" == *'*' ]]; then
						continue
					fi
					
					if ! ( [[ "${file}" == *'.so'* ]] || [[ "${file}" == *'.a' ]] ); then
						continue
					fi
					
					echo "- Symlinking '${file}' to '${PWD}'"
					
					ln --symbolic "${file}" './'
				done
			done
		done
	fi
done

# Delete libtool files and other unnecessary files GCC installs
rm --force --recursive "${toolchain_directory}/share"

find \
	"${toolchain_directory}" \
	-name '*.la' -delete -o \
	-name '*.py' -delete -o \
	-name '*.json' -delete

while read triplet; do
	if ! [ -d "${toolchain_directory}/${triplet}" ]; then
		continue
	fi
	
	declare url="https://github.com/AmanoTeam/libsanitizer/releases/latest/download/${triplet}.tar.xz"
	
	echo "- Fetching data from '${url}'"
	
	curl \
		--url "${url}" \
		--retry '30' \
		--retry-all-errors \
		--retry-delay '0' \
		--retry-max-time '0' \
		--location \
		--silent \
		--output "${libsanitizer_tarball}"
	
	tar \
		--directory="$(dirname "${libsanitizer_directory}")" \
		--extract \
		--file="${libsanitizer_tarball}"
	
	cp --recursive "${libsanitizer_directory}/lib/gcc" "${toolchain_directory}/lib"
	cp --recursive "${libsanitizer_directory}/lib/lib"* "${toolchain_directory}/${triplet}/lib"
	
	rm --recursive "${libsanitizer_directory}"
done <<< "$(jq --raw-output --compact-output '.[]' "${workdir}/submodules/libsanitizer/triplets.json")"

if ! (( is_native )); then
	declare url="https://github.com/AmanoTeam/GDB-Builds/releases/latest/download/${CROSS_COMPILE_TRIPLET}.tar.xz"
	
	echo "- Fetching data from '${url}'"
	
	curl \
		--url "${url}" \
		--retry '30' \
		--retry-all-errors \
		--retry-delay '0' \
		--retry-max-time '0' \
		--location \
		--silent \
		--output "${gdb_tarball}"
	
	tar \
		--directory="$(dirname "${gdb_directory}")" \
		--extract \
		--file="${gdb_tarball}" 2>/dev/null || gdb='0'
	
	if (( gdb )); then
		cp --recursive "${gdb_directory}/bin" "${toolchain_directory}"
		rm --recursive "${gdb_directory}"
	fi
	
	declare url="https://github.com/AmanoTeam/Nouzen/releases/latest/download/${CROSS_COMPILE_TRIPLET}.tar.xz"
	declare nz_directory="/tmp/${CROSS_COMPILE_TRIPLET}"
	
	rm --force --recursive "${nz_directory}"
	
	echo "- Fetching data from '${url}'"
	
	curl \
		--url "${url}" \
		--retry '30' \
		--retry-all-errors \
		--retry-delay '0' \
		--retry-max-time '0' \
		--location \
		--silent \
		--output "${nz_tarball}"
	
	tar \
		--directory="$(dirname "${nz_directory}")" \
		--extract \
		--file="${nz_tarball}" 2>/dev/null || nz='0'
fi

# Bundle both libstdc++ and libgcc within host tools
if ! (( is_native )); then
	[ -d "${toolchain_directory}/lib" ] || mkdir "${toolchain_directory}/lib"
	
	declare name=$(realpath $("${cc}" --print-file-name='libstdc++.so'))
	declare soname=$("${readelf}" -d "${name}" | grep 'SONAME' | sed --regexp-extended 's/.+\[(.+)\]/\1/g')
	
	cp "${name}" "${toolchain_directory}/lib/${soname}"
	
	declare name=$(realpath $("${cc}" --print-file-name='libgcc_s.so.1'))
	declare soname=$("${readelf}" -d "${name}" | grep 'SONAME' | sed --regexp-extended 's/.+\[(.+)\]/\1/g')
	
	cp "${name}" "${toolchain_directory}/lib/${soname}"
fi

while read item; do
	declare glibc_version="$(jq '.glibc_version' <<< "${item}")"
	declare triplet="$(jq --raw-output '.triplet' <<< "${item}")"
	
	declare repository="$(jq --raw-output '.repository.url' <<< "${item}")"
	declare release="$(jq --raw-output '.repository.release' <<< "${item}")"
	declare resource="$(jq --raw-output '.repository.resource' <<< "${item}")"
	declare architecture="$(jq --raw-output '.repository.architecture' <<< "${item}")"
	
	if [ "${glibc_version}" = '2' ] || [ "${glibc_version}" = '2.0' ] || [ "${glibc_version}" = '2.1' ] || [ "${glibc_version}" = '2.2' ]; then
		continue
	fi
	
	declare sysroot="https://github.com/AmanoTeam/debian-sysroot/releases/latest/download/${triplet}${glibc_version}.tar.xz"
	
	if ! [ -d "${toolchain_directory}/${triplet}" ]; then
		continue
	fi
	
	cp "${gcc_wrapper}" "${toolchain_directory}/bin/${triplet}${glibc_version}-gcc"
	cp "${gcc_wrapper}" "${toolchain_directory}/bin/${triplet}${glibc_version}-g++"
	cp "${gcc_wrapper}" "${toolchain_directory}/bin/${triplet}${glibc_version}-c++"
	
	if [[ "${languages}" = *'m2'* ]]; then
		cp "${gcc_wrapper}" "${toolchain_directory}/bin/${triplet}${glibc_version}-gm2"
	fi
	
	if [[ "${languages}" = *'fortran'* ]]; then
		cp "${gcc_wrapper}" "${toolchain_directory}/bin/${triplet}${glibc_version}-gfortran"
	fi
	
	echo "- Fetching data from '${sysroot}'"
	
	curl \
		--url "${sysroot}" \
		--retry '30' \
		--retry-all-errors \
		--retry-delay '0' \
		--retry-max-time '0' \
		--location \
		--silent \
		--output "${sysroot_tarball}"
	
	tar \
		--directory="${toolchain_directory}" \
		--extract \
		--file="${sysroot_tarball}"
	
	cd "${toolchain_directory}/${triplet}${glibc_version}/lib"
	mkdir 'gcc'
	
	if (( nz )); then
		mkdir 'nouzen'
		
		if [ -d "${nz_directory}" ]; then
			cp --recursive "${nz_directory}/"* './nouzen'
			mkdir --parent './nouzen/etc/nouzen/sources.list'
			
			echo -e "repository = ${repository}\nrelease = ${release}\nresource = ${resource}\narchitecture = ${architecture}" > './nouzen/etc/nouzen/sources.list/obggcc.conf'
		fi
	fi
	
	for library in "${libraries[@]}"; do
		for bit in "${bits[@]}"; do
			for file in "../../${triplet}/lib${bit}/${library}"*; do
				if [[ "${file}" == *'*' ]]; then
					continue
				fi
				
				if ! ( [[ "${file}" == *'.so'* ]] || [[ "${file}" == *'.a' ]] ); then
					continue
				fi
				
				echo "- Symlinking '${file}' to '${PWD}'"
				
				ln --symbolic "${file}" './'
				
				cd './gcc'
				
				file="../$(basename "${file}")"
				
				echo "- Symlinking '${file}' to '${PWD}'"
				
				ln --symbolic "${file}" './'
				
				cd '../'
			done
		done
	done
	
	cd '../'
	
	if (( nz )); then
		mkdir 'bin'
		cd 'bin'
		
		ln --symbolic '../lib/nouzen/bin/'* .
	fi
	
	cd "${toolchain_directory}/bin"
	
	if (( nz )); then
		ln --symbolic "../${triplet}${glibc_version}/bin/nz" "./${triplet}${glibc_version}-nz"
		ln --symbolic "../${triplet}${glibc_version}/bin/apt" "./${triplet}${glibc_version}-apt"
		ln --symbolic "../${triplet}${glibc_version}/bin/apt-get" "./${triplet}${glibc_version}-apt-get"
	fi
	
	for name in "${symlink_tools[@]}"; do
		source="./${triplet}-${name}"
		destination="./${triplet}${glibc_version}-${name}"
		
		if ! [ -f "${source}" ]; then
			continue
		fi
		
		echo "- Symlinking '${source}' to '${destination}'"
		
		ln --symbolic "${source}" "${destination}"
	done
done <<< "$(jq --compact-output '.[]' "${workdir}/submodules/debian-sysroot/dist.json")"

for triplet in "${targets[@]}"; do
	python3 -B "${workdir}/tools/include-missing/main.py" "${toolchain_directory}" "${triplet}"
done

mkdir --parent "${share_directory}"

cp --recursive "${workdir}/tools/dev/"* "${share_directory}"
