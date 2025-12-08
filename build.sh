#!/bin/bash

declare -r workdir="${PWD}"

declare -r build="$("${workdir}/tools/config.guess")"

if [ -z "${CROSS_COMPILE_TRIPLET}" ]; then
	declare -r host="${build}"
	declare -r native='1'
else
	declare -r host="${CROSS_COMPILE_TRIPLET}"
	declare -r native='0'
fi

if [ -z "${OBGGCC_BUILD_PARALLEL_LEVEL}" ]; then
	declare -r max_jobs="$(nproc)"
else
	declare -r max_jobs="${OBGGCC_BUILD_PARALLEL_LEVEL}"
fi

if [ -z "${OBGGCC_BUILD_DIRECTORY}" ]; then
	declare -r build_directory='/var/tmp/obggcc-build'
else
	declare -r build_directory="${OBGGCC_BUILD_DIRECTORY}"
fi

set -eu

declare -r revision="$(git rev-parse --short HEAD)"

declare -r toolchain_directory="${build_directory}/obggcc"
declare -r share_directory="${toolchain_directory}/usr/local/share/obggcc"

declare -r environment="LD_LIBRARY_PATH=${toolchain_directory}/lib PATH=${PATH}:${toolchain_directory}/bin"

declare -r autotools_directory="${share_directory}/autotools"

declare -r gmp_tarball="${build_directory}/gmp.tar.xz"
declare -r gmp_directory="${build_directory}/gmp-6.3.0"

declare -r mpfr_tarball="${build_directory}/mpfr.tar.xz"
declare -r mpfr_directory="${build_directory}/mpfr-4.2.2"

declare -r mpc_tarball="${build_directory}/mpc.tar.gz"
declare -r mpc_directory="${build_directory}/mpc-1.3.1"

declare -r isl_tarball="${build_directory}/isl.tar.xz"
declare -r isl_directory="${build_directory}/isl-0.27"

declare -r binutils_tarball="${build_directory}/binutils.tar.xz"
declare -r binutils_directory="${build_directory}/binutils"

declare -r gcc_major='15'

declare gcc_url='https://github.com/gcc-mirror/gcc/archive/master.tar.gz'
declare -r gcc_tarball="${build_directory}/gcc.tar.xz"
declare gcc_directory="${build_directory}/gcc-master"

declare -r libsanitizer_tarball="${build_directory}/libsanitizer.tar.xz"
declare -r libsanitizer_directory="${build_directory}/libsanitizer"

declare -r gdb_tarball="${build_directory}/gdb.tar.xz"
declare -r gdb_directory="${build_directory}/gdb"

declare -r nz_tarball="${build_directory}/nz.tar.xz"
declare nz_directory=""

declare -r zstd_tarball="${build_directory}/zstd.tar.gz"
declare -r zstd_directory="${build_directory}/zstd-dev"

declare -r zlib_tarball="${build_directory}/zlib.tar.gz"
declare -r zlib_directory="${build_directory}/zlib-develop"

declare -r cmake_directory="${workdir}/submodules/cmake"

declare -r curl_directory="${workdir}/submodules/curl"

declare -r ccflags='-w -O2'
declare -r linkflags='-Xlinker -s'

declare -r sysroot_tarball="${build_directory}/sysroot.tar.xz"
declare -r gcc_wrapper="${build_directory}/gcc-wrapper"
declare -r clang_wrapper="${build_directory}/clang-wrapper"

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
	'libquadmath'
)

declare -ra bits=(
	''
	'64'
)

declare -r languages='c,c++'

declare -ra deprecated_targets=(
	'ia64-unknown-linux-gnu'
	'mips-unknown-linux-gnu'
	'mips64el-unknown-linux-gnuabi64'
	'mipsel-unknown-linux-gnu'
	'powerpc-unknown-linux-gnu'
	'powerpc64le-unknown-linux-gnu'
	's390-unknown-linux-gnu'
	's390x-unknown-linux-gnu'
	'sparc-unknown-linux-gnu'
	'alpha-unknown-linux-gnu'
	'hppa-unknown-linux-gnu'
)

declare -ra targets=(
	'x86_64-unknown-linux-gnu'
	'aarch64-unknown-linux-gnu'
	'arm-unknown-linux-gnueabi'
	'arm-unknown-linux-gnueabihf'
	'i386-unknown-linux-gnu'
)

declare -r PKG_CONFIG_PATH="${toolchain_directory}/lib/pkgconfig"
declare -r PKG_CONFIG_LIBDIR="${PKG_CONFIG_PATH}"
declare -r PKG_CONFIG_SYSROOT_DIR="${toolchain_directory}"

declare -r pkg_cv_ZSTD_CFLAGS="-I${toolchain_directory}/include"
declare -r pkg_cv_ZSTD_LIBS="-L${toolchain_directory}/lib -lzstd"
declare -r ZSTD_CFLAGS="-I${toolchain_directory}/include"
declare -r ZSTD_LIBS="-L${toolchain_directory}/lib -lzstd"

export \
	PKG_CONFIG_PATH \
	PKG_CONFIG_LIBDIR \
	PKG_CONFIG_SYSROOT_DIR \
	pkg_cv_ZSTD_CFLAGS \
	pkg_cv_ZSTD_LIBS \
	ZSTD_CFLAGS \
	ZSTD_LIBS

rm --force --recursive "${toolchain_directory}"
mkdir --parent "${build_directory}"

export PATH="${build_directory}:${build_directory}/bin:${PATH}"

cd "${cmake_directory}"

CC= CXX= \
	./bootstrap \
	--prefix="${build_directory}" \
	--parallel="${max_jobs}"

make all --jobs="${max_jobs}"
make install

cmake --version

CC= CXX= \
	cmake \
	-S "${curl_directory}" \
	-B "${curl_directory}/build" \
	-D CMAKE_INSTALL_PREFIX="${build_directory}" \
	-D CURL_USE_LIBPSL=OFF \
	-D CURL_ENABLE_SSL=ON \
	-D CURL_USE_OPENSSL=ON \
	-D BUILD_SHARED_LIBS=OFF \
	-D BUILD_STATIC_LIBS=ON

make \
	-C "${curl_directory}/build" \
	--jobs="${max_jobs}" \
	install

curl --version

if ! [ -f "${gmp_tarball}" ]; then
	curl \
		--url 'https://mirrors.kernel.org/gnu/gmp/gmp-6.3.0.tar.xz' \
		--retry '30' \
		--retry-all-errors \
		--retry-delay '0' \
		--retry-max-time '0' \
		--show-error \
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
		--url 'https://mirrors.kernel.org/gnu/mpfr/mpfr-4.2.2.tar.xz' \
		--retry '30' \
		--retry-all-errors \
		--retry-delay '0' \
		--retry-max-time '0' \
		--show-error \
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
		--url 'https://mirrors.kernel.org/gnu/mpc/mpc-1.3.1.tar.gz' \
		--retry '30' \
		--retry-all-errors \
		--retry-delay '0' \
		--retry-max-time '0' \
		--show-error \
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
		--url 'https://deb.debian.org/debian/pool/main/i/isl/isl_0.27.orig.tar.xz' \
		--retry '30' \
		--retry-all-errors \
		--retry-delay '0' \
		--retry-max-time '0' \
		--show-error \
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
		--url 'https://github.com/AmanoTeam/binutils-snapshots/releases/latest/download/binutils.tar.xz' \
		--retry '30' \
		--retry-all-errors \
		--retry-delay '0' \
		--retry-max-time '0' \
		--show-error \
		--location \
		--silent \
		--output "${binutils_tarball}"
	
	tar \
		--directory="$(dirname "${binutils_directory}")" \
		--extract \
		--file="${binutils_tarball}"
	
	if [[ "${host}" = *'-darwin'* ]]; then
		sed \
			--in-place \
			's/$$ORIGIN/@loader_path/g' \
			"${workdir}/patches/0001-Add-relative-RPATHs-to-binutils-host-tools.patch"
	fi
	
	patch --directory="${binutils_directory}" --strip='1' --input="${workdir}/patches/0001-Add-relative-RPATHs-to-binutils-host-tools.patch"
	patch --directory="${binutils_directory}" --strip='1' --input="${workdir}/patches/0001-Don-t-warn-about-local-symbols-within-the-globals.patch"
fi

if ! [ -f "${zlib_tarball}" ]; then
	curl \
		--url 'https://github.com/madler/zlib/archive/refs/heads/develop.tar.gz' \
		--retry '30' \
		--retry-all-errors \
		--retry-delay '0' \
		--retry-max-time '0' \
		--show-error \
		--location \
		--silent \
		--output "${zlib_tarball}"
	
	tar \
		--directory="$(dirname "${zlib_directory}")" \
		--extract \
		--file="${zlib_tarball}"
	
	patch --directory="${zlib_directory}" --strip='1' --input="${workdir}/patches/0001-Remove-versioned-SONAME-from-libz.patch"
fi

if ! [ -f "${zstd_tarball}" ]; then
	curl \
		--url 'https://github.com/facebook/zstd/archive/refs/heads/dev.tar.gz' \
		--retry '30' \
		--retry-all-errors \
		--retry-delay '0' \
		--retry-max-time '0' \
		--show-error \
		--location \
		--silent \
		--output "${zstd_tarball}"
	
	tar \
		--directory="$(dirname "${zstd_directory}")" \
		--extract \
		--file="${zstd_tarball}"
fi

if ! [ -f "${gcc_tarball}" ]; then
	if [ "${gcc_major}" != '16' ]; then
		gcc_url='https://github.com/gcc-mirror/gcc/archive/releases/gcc-15.tar.gz'
		gcc_directory="${build_directory}/gcc-releases-gcc-${gcc_major}"
	fi
	
	curl \
		--url "${gcc_url}" \
		--retry '30' \
		--retry-all-errors \
		--retry-delay '0' \
		--retry-max-time '0' \
		--show-error \
		--location \
		--silent \
		--output "${gcc_tarball}"
	
	tar \
		--directory="$(dirname "${gcc_directory}")" \
		--extract \
		--file="${gcc_tarball}"
	
	if [[ "${host}" = *'-darwin'* ]]; then
		sed \
			--in-place \
			's/$$ORIGIN/@loader_path/g' \
			"${workdir}/patches/0007-Add-relative-RPATHs-to-GCC-host-tools.patch"
	fi
	
	patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0001-Turn-Wimplicit-function-declaration-back-into-an-warning.patch"
	patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0002-Fix-libsanitizer-build-on-older-platforms.patch"
	patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0003-Change-the-default-language-version-for-C-compilation-from-std-gnu23-to-std-gnu17.patch"
	
	if [ "${gcc_major}" = '16' ]; then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0004-Turn-Wimplicit-int-back-into-an-warning.patch"
	else
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-15/0004-Turn-Wimplicit-int-back-into-an-warning.patch"
	fi
	
	patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0005-Turn-Wint-conversion-back-into-an-warning.patch"
	
	if [ "${gcc_major}" = '16' ]; then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0006-Turn-Wincompatible-pointer-types-back-into-an-warning.patch"
	else
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-15/0006-Turn-Wincompatible-pointer-types-back-into-an-warning.patch"
	fi
	
	patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0007-Add-relative-RPATHs-to-GCC-host-tools.patch"
	patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0008-Add-ARM-and-ARM64-drivers-to-OpenBSD-host-tools.patch"
	patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0009-Fix-missing-stdint.h-include-when-compiling-host-tools-on-OpenBSD.patch"
	patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0010-Prefer-DT_RPATH-over-DT_RUNPATH.patch"
	
	if [ "${gcc_major}" = '16' ]; then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0011-Revert-configure-Always-add-pre-installed-header-directories-to-search-path.patch"
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0001-Revert-x86-Fixes-for-AMD-znver5-enablement.patch"
	fi
	
	if [ "${gcc_major}" = '15' ]; then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-15/0001-Enable-automatic-linking-of-libatomic.patch"
	fi
	
	patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0001-AArch64-enable-libquadmath.patch"
fi

# Follow Debian's approach to remove hardcoded RPATHs from binaries
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

# Avoid using absolute hardcoded install_name values on macOS
sed \
	--in-place \
	's|-install_name \\$rpath/\\$soname|-install_name @rpath/\\$soname|g' \
	"${isl_directory}/configure" \
	"${mpc_directory}/configure" \
	"${mpfr_directory}/configure" \
	"${gmp_directory}/configure"

# Fix Autotools mistakenly detecting shared libraries as not supported on OpenBSD
while read file; do
	sed \
		--in-place \
		--regexp-extended \
		's|test -f /usr/libexec/ld.so|true|g' \
		"${file}"
done <<< "$(find "${build_directory}" -type 'f' -name 'configure')"

# Force GCC and binutils to prefix host tools with the target triplet even in native builds
sed \
	--in-place \
	's/test "$host_noncanonical" = "$target_noncanonical"/false/' \
	"${gcc_directory}/configure" \
	"${binutils_directory}/configure"

declare disable_assembly='--disable-assembly'

if [[ "${host}" != 'mips64el-'* ]]; then
	disable_assembly=''
fi

[ -d "${gmp_directory}/build" ] || mkdir "${gmp_directory}/build"

cd "${gmp_directory}/build"
rm --force --recursive ./*

../configure \
	--build="${build}" \
	--host="${host}" \
	--prefix="${toolchain_directory}" \
	--enable-shared \
	--disable-static \
	${disable_assembly} \
	CFLAGS="${ccflags}" \
	CXXFLAGS="${ccflags}" \
	LDFLAGS="${linkflags}"

make all --jobs
make install

[ -d "${mpfr_directory}/build" ] || mkdir "${mpfr_directory}/build"

cd "${mpfr_directory}/build"
rm --force --recursive ./*

../configure \
	--build="${build}" \
	--host="${host}" \
	--prefix="${toolchain_directory}" \
	--with-gmp="${toolchain_directory}" \
	--enable-shared \
	--disable-static \
	CFLAGS="${ccflags}" \
	CXXFLAGS="${ccflags}" \
	LDFLAGS="${linkflags}"

make all --jobs
make install

[ -d "${mpc_directory}/build" ] || mkdir "${mpc_directory}/build"

cd "${mpc_directory}/build"
rm --force --recursive ./*

../configure \
	--build="${build}" \
	--host="${host}" \
	--prefix="${toolchain_directory}" \
	--with-gmp="${toolchain_directory}" \
	--enable-shared \
	--disable-static \
	CFLAGS="${ccflags}" \
	CXXFLAGS="${ccflags}" \
	LDFLAGS="${linkflags}"

make all --jobs
make install

[ -d "${isl_directory}/build" ] || mkdir "${isl_directory}/build"

cd "${isl_directory}/build"
rm --force --recursive ./*

declare isl_extra_ldflags=''

if [[ "${host}" != *'-darwin'* ]]; then
	isl_extra_ldflags+=" -Xlinker -rpath-link -Xlinker ${toolchain_directory}/lib"
fi

../configure \
	--build="${build}" \
	--host="${host}" \
	--prefix="${toolchain_directory}" \
	--with-gmp-prefix="${toolchain_directory}" \
	--enable-shared \
	--disable-static \
	--with-pic \
	CFLAGS="${ccflags}" \
	CXXFLAGS="${ccflags}" \
	LDFLAGS="${linkflags} ${isl_extra_ldflags}"

make all --jobs
make install

[ -d "${zlib_directory}/build" ] || mkdir "${zlib_directory}/build"

cd "${zlib_directory}/build"
rm --force --recursive ./*

../configure \
	--build="${build}" \
	--host="${host}" \
	--prefix="${toolchain_directory}" \
	CFLAGS="${ccflags}" \
	CXXFLAGS="${ccflags}" \
	LDFLAGS="${linkflags}"

make all --jobs
make install

unlink "${toolchain_directory}/lib/libz.a"

[ -d "${zstd_directory}/.build" ] || mkdir "${zstd_directory}/.build"

cd "${zstd_directory}/.build"
rm --force --recursive ./*

declare cmake_flags=''

if [[ "${host}" = *'-darwin'* ]]; then
	cmake_flags+=' -DCMAKE_SYSTEM_NAME=Darwin'
fi

cmake \
	-S "${zstd_directory}/build/cmake" \
	-B "${PWD}" \
	${cmake_flags} \
	-DCMAKE_C_FLAGS="-DZDICT_QSORT=ZDICT_QSORT_MIN ${ccflags}" \
	-DCMAKE_INSTALL_PREFIX="${toolchain_directory}" \
	-DBUILD_SHARED_LIBS=ON \
	-DZSTD_BUILD_PROGRAMS=OFF \
	-DZSTD_BUILD_TESTS=OFF \
	-DZSTD_BUILD_STATIC=OFF \
	-DCMAKE_PLATFORM_NO_VERSIONED_SONAME=ON

cmake --build "${PWD}" -- --jobs
cmake --install "${PWD}" --strip

# We prefer symbolic links over hard links.
cp "${workdir}/tools/ln.sh" "${build_directory}/ln"

if [[ "${host}" = 'arm'*'-android'* ]] || [[ "${host}" = 'i686-'*'-android'* ]] || [[ "${host}" = 'mipsel-'*'-android'* ]]; then
	export \
		ac_cv_func_fseeko='no' \
		ac_cv_func_ftello='no'
fi

if [[ "${host}" = 'armv5'*'-android'* ]]; then
	export PINO_ARM_MODE='true'
fi

if [[ "${host}" = *'-haiku' ]]; then
	export ac_cv_c_bigendian='no'
fi

declare cc='gcc'
declare readelf='readelf'

if ! (( native )); then
	cc="${CC}"
	readelf="${READELF}"
fi

sed \
	--in-place \
	--regexp-extended \
	"s/(GCC_MAJOR_VERSION\[\] = )\"[0-9]+\"/\1\"${gcc_major}\"/g" \
	"${workdir}/tools/gcc-wrapper/gcc.c" \

make \
	-C "${workdir}/tools/gcc-wrapper" \
	PREFIX="$(dirname "${gcc_wrapper}")" \
	CFLAGS="-D WCLANG ${ccflags}" \
	CXXFLAGS="${ccflags}" \
	LDFLAGS="${linkflags}"  \
	gcc

cp "${gcc_wrapper}" "${clang_wrapper}"

make \
	-C "${workdir}/tools/gcc-wrapper" \
	PREFIX="$(dirname "${gcc_wrapper}")" \
	CFLAGS="${ccflags}" \
	CXXFLAGS="${ccflags}" \
	LDFLAGS="${linkflags}"

for target in "${targets[@]}"; do
	declare specs='%{!Qy: -Qn}'
	declare hash_style='both'
	
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
	
	rm --force --recursive "${PWD}"
	
	[ -d "${binutils_directory}/build" ] || mkdir "${binutils_directory}/build"
	
	cd "${binutils_directory}/build"
	
	../configure \
		--build="${build}" \
		--host="${host}" \
		--target="${triplet}" \
		--prefix="${toolchain_directory}" \
		--enable-ld \
		--enable-lto \
		--enable-separate-code \
		--enable-rosegment \
		--enable-relro \
		--enable-compressed-debug-sections='all' \
		--enable-default-compressed-debug-sections-algorithm='zstd' \
		--disable-gprofng \
		--disable-gold \
		--disable-default-execstack \
		--with-sysroot="${toolchain_directory}/${triplet}" \
		--without-static-standard-libraries \
		--with-zstd="${toolchain_directory}" \
		--with-system-zlib \
		CFLAGS="-I${toolchain_directory}/include ${ccflags}" \
		CXXFLAGS="-I${toolchain_directory}/include ${ccflags}" \
		LDFLAGS="-L${toolchain_directory}/lib ${linkflags}"
	
	make all --jobs
	make install
	
	rm --force --recursive "${PWD}" &
	
	if [ "${triplet}" = 'x86_64-unknown-linux-gnu' ] || [ "${triplet}" = 'i386-unknown-linux-gnu' ]; then
		specs+=' %{!fno-plt: %{!fplt: -fno-plt}}'
	fi
	
	if [[ "${triplet}" = 'mips'* ]]; then
		hash_style='sysv'
	fi
	
	if ! (( native )); then
		extra_configure_flags+=" --with-cross-host=${host}"
		extra_configure_flags+=" --with-toolexeclibdir=${toolchain_directory}/${triplet}/lib/"
	fi
	
	if [[ "${host}" != *'-darwin'* ]]; then
		extra_configure_flags+=' --enable-host-bind-now'
	fi
	
	[ -d "${gcc_directory}/build" ] || mkdir "${gcc_directory}/build"
	
	cd "${gcc_directory}/build"
	
	../configure \
		--build="${build/unknown-/}" \
		--host="${host}" \
		--target="${triplet}" \
		--prefix="${toolchain_directory}" \
		--with-linker-hash-style="${hash_style}" \
		--with-gmp="${toolchain_directory}" \
		--with-mpc="${toolchain_directory}" \
		--with-mpfr="${toolchain_directory}" \
		--with-isl="${toolchain_directory}" \
		--with-zstd="${toolchain_directory}" \
		--with-system-zlib \
		--with-gcc-major-version-only \
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
		--enable-plugin \
		--enable-libstdcxx-time='rt' \
		--enable-cxx-flags="${linkflags} ${extra_cxx_flags}" \
		--enable-host-pie \
		--enable-host-shared \
		--enable-libgomp \
		--with-specs="${specs}" \
		--with-pic \
		--disable-libsanitizer \
		--disable-bootstrap \
		--disable-libstdcxx-pch \
		--disable-werror \
		--disable-multilib \
		--disable-nls \
		--without-headers \
		--without-static-standard-libraries \
		${extra_configure_flags} \
		CFLAGS="${ccflags}" \
		CXXFLAGS="${ccflags}" \
		LDFLAGS="-L${toolchain_directory}/lib ${linkflags}"
	
	cflags_for_target="${ccflags} ${linkflags}"
	cxxflags_for_target="${ccflags} ${linkflags}"
	ldflags_for_target="${linkflags}"
	
	declare args=''
	
	if (( native )); then
		args+="${environment}"
	fi
	
	env ${args} make \
		CFLAGS_FOR_TARGET="${cflags_for_target}" \
		CXXFLAGS_FOR_TARGET="${cxxflags_for_target}" \
		LDFLAGS_FOR_TARGET="${ldflags_for_target}" \
		gcc_cv_objdump="${host}-objdump" \
		all \
		--jobs="${max_jobs}"
	make install
	
	rm --force --recursive "${PWD}"
	
	cd "${toolchain_directory}/${triplet}/lib64" 2>/dev/null || cd "${toolchain_directory}/${triplet}/lib"
	
	if [[ "$(basename "${PWD}")" = 'lib64' ]]; then
		mv './'* '../lib' || true
		rmdir "${PWD}"
		cd '../lib'
	fi
	
	env ${args} make \
		-C "${workdir}/tools/libblocksruntime" \
		PREFIX="${toolchain_directory}/${triplet}" \
		CC="${triplet}-gcc" \
		CFLAGS="${ccflags}" \
		CXXFLAGS="${ccflags}" \
		LDFLAGS="${linkflags}"
	
	ln \
		--symbolic \
		--relative \
		"${toolchain_directory}/lib/gcc/${triplet}/${gcc_major}/"*'.'{a,o} \
		'./'
	
	declare gcc_include_dir="${toolchain_directory}/lib/gcc/${triplet}/${gcc_major}/include"
	declare clang_include_dir="${gcc_include_dir}/clang"
	
	mkdir "${clang_include_dir}"
	
	ln \
		--symbolic \
		--relative \
		"${gcc_include_dir}/"*'.h' \
		"${clang_include_dir}"
	
	rm --force \
		"${clang_include_dir}/"*'intrin'*'.h' \
		"${clang_include_dir}/arm"*'.h' \
		"${clang_include_dir}/stdatomic.h"
	
	[ -f './libiberty.a' ] && unlink './libiberty.a'
	
	unlink './libgcc_s.so' && echo 'GROUP ( libgcc_s.so.1 -lgcc )' > './libgcc_s.so'
	
	cp "${workdir}/tools/pkg-config.sh" "${toolchain_directory}/bin/${triplet}-pkg-config"
	
	rm "${toolchain_directory}/bin/${triplet}-${triplet}-"* || true
	
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

if ! (( native )); then
	declare url="https://github.com/AmanoTeam/GDB-Builds/releases/latest/download/${host}.tar.xz"
	
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
	
	declare url="https://github.com/AmanoTeam/Nouzen/releases/latest/download/${host}.tar.xz"
	declare nz_directory="${build_directory}/${host}"
	
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
	
	if (( nz )); then
		mv "${nz_directory}/lib" "${toolchain_directory}/lib/nouzen"
	fi
fi

# Bundle both libstdc++ and libgcc within host tools
if ! (( native )) && [[ "${host}" != *'-darwin'* ]]; then
	[ -d "${toolchain_directory}/lib" ] || mkdir "${toolchain_directory}/lib"
	
	# libstdc++
	declare name=$(realpath $("${cc}" --print-file-name='libstdc++.so'))
	
	# libestdc++
	if ! [ -f "${name}" ]; then
		declare name=$(realpath $("${cc}" --print-file-name='libestdc++.so'))
	fi
	
	declare soname=$("${readelf}" -d "${name}" | grep 'SONAME' | sed --regexp-extended 's/.+\[(.+)\]/\1/g')
	
	cp "${name}" "${toolchain_directory}/lib/${soname}"
	
	# OpenBSD does not have a libgcc library
	if [[ "${host}" != *'-openbsd'* ]]; then
		# libgcc_s
		declare name=$(realpath $("${cc}" --print-file-name='libgcc_s.so.1'))
		
		# libegcc
		if ! [ -f "${name}" ]; then
			declare name=$(realpath $("${cc}" --print-file-name='libegcc.so'))
		fi
		
		declare soname=$("${readelf}" -d "${name}" | grep 'SONAME' | sed --regexp-extended 's/.+\[(.+)\]/\1/g')
		
		cp "${name}" "${toolchain_directory}/lib/${soname}"
	fi
	
	# libatomic
	declare name=$(realpath $("${cc}" --print-file-name='libatomic.so'))
	
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
	
	cp "${clang_wrapper}" "${toolchain_directory}/bin/${triplet}${glibc_version}-clang"
	cp "${clang_wrapper}" "${toolchain_directory}/bin/${triplet}${glibc_version}-clang++"
	
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
	mkdir 'gcc' 'static'
	
	ln --symbolic './lib'*'.'{so,a}* './static'
	ln --symbolic './ld-'*'.so'* './static'
	ln --symbolic './'*'.o' './static'
	
	ln \
		--symbolic \
		--relative \
		"${toolchain_directory}/${triplet}/include/Block"* \
		'../include'
	
	ln \
		--symbolic \
		--relative \
		"${toolchain_directory}/${triplet}/lib/libBlocksRuntime.a" \
		'./'
	
	ln \
		--symbolic \
		--relative \
		"${toolchain_directory}/${triplet}/lib/libBlocksRuntime.a" \
		'./static'
	
	ln \
		--symbolic \
		--relative \
		"${toolchain_directory}/lib/gcc/${triplet}/${gcc_major}/"*'.'{a,o} \
		'./'
	
	ln \
		--symbolic \
		--relative \
		"${toolchain_directory}/lib/gcc/${triplet}/${gcc_major}/"*'.'{a,o} \
		'./static'
	
	if (( nz )); then
		mkdir 'nouzen'
		
		if [ -d "${nz_directory}" ]; then
			cp --recursive "${nz_directory}/"* './nouzen'
			
			ln \
				--symbolic \
				--relative \
				"${toolchain_directory}/lib/nouzen" \
				"${PWD}/nouzen/lib"
			
			mkdir --parent './nouzen/etc/nouzen/sources.list'
			
			echo -e "repository = ${repository}\nrelease = ${release}\nresource = ${resource}\narchitecture = ${architecture}" > './nouzen/etc/nouzen/sources.list/obggcc.conf'
		fi
	fi
	
	for library in "${libraries[@]}"; do
		for bit in "${bits[@]}"; do
			for file in "../../${triplet}/lib${bit}/${library}"*; do
				if [[ "${file}" = *'*' ]]; then
					continue
				fi
				
				if ! ( [[ "${file}" = *'.so'* ]] || [[ "${file}" = *'.a' ]] ); then
					continue
				fi
				
				echo "- Symlinking '${file}' to '${PWD}'"
				
				ln --force --symbolic "${file}" './'
				
				echo "- Symlinking '${file}' to '${PWD}/gcc'"
				
				ln --force --symbolic --relative "${file}" './gcc'
				
				if [[ "${file}" = *'.a' ]]; then
					echo "- Symlinking '${file}' to '${PWD}/static'"
					
					ln --force --symbolic --relative "${file}" './static'
				fi
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

[ -d "${toolchain_directory}/build" ] || mkdir "${toolchain_directory}/build"

ln \
	--symbolic \
	--relative \
	"${share_directory}/"* \
	"${toolchain_directory}/build"

for target in "${deprecated_targets[@]}"; do
	rm --force "${toolchain_directory}/bin/${target}"*
	rm --force "${share_directory}/"*"/${target}"*
	rm --force "${share_directory}/"*"/clang/${target}"*
done
