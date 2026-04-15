#!/usr/bin/env zsh

unsetopt nomatch

declare -r workdir="${PWD}"

declare build="$(env CC=gcc "${workdir}/tools/config.guess")"

echo "Build: ${build}"

build="${build/-unknown-/-pc-}"

if [ -z "${CROSS_COMPILE_TRIPLET}" ]; then
	declare host="${build}"
	declare -r native='1'
else
	declare host="${CROSS_COMPILE_TRIPLET}"
	declare -r native='0'
fi

echo "Host: ${host}"

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

if [ -z "${OBGGCC_RELEASE}" ]; then
	declare -r gcc_major='16'
else
	declare -r gcc_major="${OBGGCC_RELEASE}"
fi

if [ -z "${OBGGCC_TARGETS}" ]; then
	declare -r gcc_targets='*'
else
	declare -r gcc_targets="${OBGGCC_TARGETS}"
fi

set -eu

declare -r revision="$(git rev-parse --short HEAD)"

declare -r toolchain_directory="${build_directory}/obggcc"
declare -r share_directory="${toolchain_directory}/usr/local/share/obggcc"

declare -r environment="LD_LIBRARY_PATH=${toolchain_directory}/lib PATH=${PATH}:${toolchain_directory}/bin"

declare -r autotools_directory="${share_directory}/autotools"

declare -r gmp_tarball="${build_directory}/gmp.tar.xz"
declare -r gmp_directory="${build_directory}/gmp"

declare -r mpfr_tarball="${build_directory}/mpfr.tar.gz"
declare -r mpfr_directory="${build_directory}/mpfr-master"

declare -r mpc_tarball="${build_directory}/mpc.tar.gz"
declare -r mpc_directory="${build_directory}/mpc-master"

declare -r isl_tarball="${build_directory}/isl.tar.gz"
declare -r isl_directory="${build_directory}/isl-master"

declare -r binutils_tarball="${build_directory}/binutils.tar.xz"
declare -r binutils_directory="${build_directory}/binutils"

declare -r gold_tarball="${build_directory}/gold.tar.xz"
declare -r gold_directory="${build_directory}/gold"

declare gcc_url='https://github.com/gcc-mirror/gcc/archive/master.tar.gz'
declare -r gcc_tarball="${build_directory}/gcc.tar.gz"
declare gcc_directory="${build_directory}/gcc-master"

declare -r libsanitizer_tarball="${build_directory}/libsanitizer.tar.xz"
declare -r libsanitizer_directory="${build_directory}/libsanitizer"

declare -r gdb_tarball="${build_directory}/gdb.tar.xz"
declare -r gdb_directory="${build_directory}/gdb"

declare -r zstd_tarball="${build_directory}/zstd.tar.gz"
declare -r zstd_directory="${build_directory}/zstd-dev"

declare -r zlib_tarball="${build_directory}/zlib.tar.gz"
declare -r zlib_directory="${build_directory}/zlib-develop"

declare -r yasm_tarball='/tmp/yasm.tar.gz'
declare -r yasm_directory='/tmp/yasm-1.3.0'

declare -r ninja_tarball='/tmp/ninja.tar.gz'
declare -r ninja_directory='/tmp/ninja-master'

declare -r bison_tarball="${build_directory}/bison.tar.xz"
declare -r bison_directory="${build_directory}/bison"

declare -r cmake_directory="${workdir}/submodules/cmake"

declare -r curl_directory="${workdir}/submodules/curl"

declare -r nz_directory="${workdir}/submodules/nz"
declare -r nz_prefix="${build_directory}/nz"

declare -r ccflags='-O2'
declare -r linkflags=''

declare -r sysroot_tarball="${build_directory}/sysroot.tar.xz"

declare gdb='1'

declare build_cmake='0'
declare build_curl='0'
declare build_nz='1'

declare exe=''
declare dll='.so'

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
	'libcilkrts'
	'libvtv'
	'libgcov'
	'libmpx'
	'libmudflap'
	'libmudflapth'
)

declare -ra bits=(
	''
	'64'
)

declare -r languages='c,c++'

declare -ra deprecated_targets=(
	'armv6-unknown-linux-gnueabi'
)

declare -a targets=(
	'arm-unknown-linux-gnueabi'
	'arm-unknown-linux-gnueabihf'
	'aarch64-unknown-linux-gnu'
	'i386-unknown-linux-gnu'
	'x86_64-unknown-linux-gnu'
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

export libat_cv_have_ifunc='no'
export ac_cv_header_stdc='yes'

if [[ "${host}" = *'-android'* ]]; then
	export ac_cv_func_ffsll='yes'
fi

if [[ "${host}" = *'-mingw32' ]]; then
	build_nz='0'
	exe='.exe'
	dll='.dll'
fi

if (( gcc_major < 15 )); then
	build_nz='0'
fi

if [ "${gcc_major}" != '16' ]; then
	if (( gcc_major == 3.1 )); then
		# 3.1 was improperly tagged and includes some 3.2 changes
		tag='4f77a8528f4ccda031f29634f6264eb77f91d13b'
	else
		tag="releases/gcc-${gcc_major}"
	fi
	
	gcc_url="https://github.com/gcc-mirror/gcc/archive/${tag}.tar.gz"
	gcc_directory="${build_directory}/gcc-${tag//\//-}"
fi

declare -r gcc_wrapper="${build_directory}/gcc-wrapper${exe}"
declare -r binutils_gnu_wrapper="${build_directory}/binutils-gnu-wrapper${exe}"
declare -r binutils_llvm_wrapper="${build_directory}/binutils-llvm-wrapper${exe}"
declare -r clang_wrapper="${build_directory}/clang-wrapper${exe}"

source "${workdir}/utils.sh"

rm --force --recursive "${toolchain_directory}"
mkdir --parent "${build_directory}"

export PATH="${build_directory}:${build_directory}/bin:${PATH}"

if (( build_cmake )); then
	cd "${cmake_directory}"
	
	CC= CXX= CMAKE_TOOLCHAIN_FILE= \
		./bootstrap \
		--prefix="${build_directory}" \
		--parallel="${max_jobs}"
	
	make all --jobs="${max_jobs}"
	make install
	
	sed \
		--in-place \
		'/CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG/d' \
		"${build_directory}/share/cmake-"*'/Modules/Platform/Android.cmake'
fi

cmake --version

if (( build_curl )); then
	CC= CXX= CMAKE_TOOLCHAIN_FILE= \
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
fi

curl --version

if ! [ -f "${gmp_tarball}" ]; then
	curl \
		--url 'https://github.com/AmanoTeam/gmplib-snapshots/releases/latest/download/gmp.tar.xz' \
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
	
	sed \
		--in-place \
		's/-Xlinker --out-implib -Xlinker $lib/-Xlinker --out-implib -Xlinker $lib.a/g' \
		"${gmp_directory}/configure"
fi

if ! [ -f "${mpfr_tarball}" ]; then
	curl \
		--url 'https://github.com/AmanoTeam/mpfr/archive/master.tar.gz' \
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
	
	cd "${mpfr_directory}"
	autoreconf --force --install
	
	patch --directory="${mpfr_directory}" --strip='1' --input="${workdir}/patches/0001-Remove-hardcoded-RPATH-and-versioned-SONAME-from-libmpfr.patch"
fi

if ! [ -f "${mpc_tarball}" ]; then
	curl \
		--url 'https://github.com/AmanoTeam/mpc/archive/master.tar.gz' \
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
	
	cd "${mpc_directory}"
	autoreconf --force --install
	
	patch --directory="${mpc_directory}" --strip='1' --input="${workdir}/patches/0001-Remove-hardcoded-RPATH-and-versioned-SONAME-from-libmpc.patch"
fi

if ! [ -f "${isl_tarball}" ]; then
	curl \
		--url 'https://github.com/AmanoTeam/isl/archive/master.tar.gz' \
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
	
	cd "${isl_directory}"
	autoreconf --force --install
	
	patch --directory="${isl_directory}" --strip='1' --input="${workdir}/patches/0001-Remove-hardcoded-RPATH-and-versioned-SONAME-from-libisl.patch"
	
	for name in "${isl_directory}/isl_test"*; do
		echo 'int main() {}' > "${name}"
	done
	
	sed \
		--in-place \
		--regexp-extended \
		's/(allow_undefined)=.*$/\1=no/' \
		"${isl_directory}/ltmain.sh" \
		"${isl_directory}/interface/ltmain.sh"
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
	
	if [[ "${host}" = *'bsd'* ]] || [[ "${host}" = *'dragonfly' ]] then
		sed \
			--in-place \
			's/-Xlinker -rpath/-Xlinker -z -Xlinker origin -Xlinker -rpath/g' \
			"${workdir}/patches//0001-Add-relative-RPATHs-to-binutils-host-tools.patch"
	fi
	
	if [[ "${host}" = *'-darwin'* ]]; then
		sed \
			--in-place \
			's/$$ORIGIN/@loader_path/g' \
			"${workdir}/patches/0001-Add-relative-RPATHs-to-binutils-host-tools.patch"
	fi
	
	patch --directory="${binutils_directory}" --strip='1' --input="${workdir}/patches/0001-Add-relative-RPATHs-to-binutils-host-tools.patch"
	patch --directory="${binutils_directory}" --strip='1' --input="${workdir}/patches/0001-Don-t-warn-about-local-symbols-within-the-globals.patch"
	patch --directory="${binutils_directory}" --strip='1' --input="${workdir}/patches/0001-Don-t-warn-about-errors-while-trying-to-create-the-.eh_frame_hdr-section.patch"
fi

if ! [ -f "${gold_tarball}" ]; then
	curl \
		--url 'https://github.com/AmanoTeam/binutils-snapshots/releases/latest/download/gold.tar.xz' \
		--retry '30' \
		--retry-all-errors \
		--retry-delay '0' \
		--retry-max-time '0' \
		--show-error \
		--location \
		--silent \
		--output "${gold_tarball}"
	
	tar \
		--directory="$(dirname "${gold_directory}")" \
		--extract \
		--file="${gold_tarball}"
	
	if [[ "${host}" = *'bsd'* ]] || [[ "${host}" = *'dragonfly' ]] then
		sed \
			--in-place \
			's/-Xlinker -rpath/-Xlinker -z -Xlinker origin -Xlinker -rpath/g' \
			"${workdir}/patches/0001-Add-relative-RPATHs-to-gold-host-tools.patch"
	fi
	
	if [[ "${host}" = *'-darwin'* ]]; then
		sed \
			--in-place \
			's/$$ORIGIN/@loader_path/g' \
			"${workdir}/patches/0001-Add-relative-RPATHs-to-gold-host-tools.patch"
	fi
	
	patch --directory="${gold_directory}" --strip='1' --input="${workdir}/patches/0001-Revert-gold-Use-char16_t-char32_t-instead-of-uint16_t-uint32_t-as-character-types.patch"
	patch --directory="${gold_directory}" --strip='1' --input="${workdir}/patches/0001-Make-gold-linker-ignore-unknown-z-options.patch"
	patch --directory="${gold_directory}" --strip='1' --input="${workdir}/patches/0001-Add-relative-RPATHs-to-gold-host-tools.patch"
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
	
	sed \
		--in-place \
		's/(UNIX)/(1)/g; s/(NOT APPLE)/(0)/g' \
		"${zlib_directory}/CMakeLists.txt"
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
	
	sed \
		--in-place \
		's/LANGUAGES C   # M/LANGUAGES C CXX  # M/g' \
		"${zstd_directory}/build/cmake/CMakeLists.txt"
fi

if ! [ -f "${yasm_tarball}" ]; then
	curl \
		--url 'https://deb.debian.org/debian/pool/main/y/yasm/yasm_1.3.0.orig.tar.gz' \
		--retry '30' \
		--retry-delay '0' \
		--retry-all-errors \
		--retry-max-time '0' \
		--location \
		--silent \
		--output "${yasm_tarball}"
	
	tar \
		--directory="$(dirname "${yasm_directory}")" \
		--extract \
		--file="${yasm_tarball}"
	
	cd "${yasm_directory}"
	autoreconf --force --install
fi

if ! [ -f "${ninja_tarball}" ]; then
	curl \
		--url 'https://github.com/ninja-build/ninja/archive/master.tar.gz' \
		--retry '30' \
		--retry-delay '0' \
		--retry-all-errors \
		--retry-max-time '0' \
		--location \
		--silent \
		--output "${ninja_tarball}"
	
	tar \
		--directory="$(dirname "${ninja_directory}")" \
		--extract \
		--file="${ninja_tarball}"
fi

if ! [ -f "${gcc_tarball}" ]; then
	echo "${gcc_url}"
	
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
	
	if [[ "${host}" = *'bsd'* ]] || [[ "${host}" = *'dragonfly' ]] then
		sed \
			--in-place \
			's/-Xlinker -rpath/-Xlinker -z -Xlinker origin -Xlinker -rpath/g' \
			"${workdir}/patches/0007-Add-relative-RPATHs-to-GCC-host-tools.patch"
		
		sed \
			--in-place \
			's/-Xlinker -rpath/-Xlinker -z -Xlinker origin -Xlinker -rpath/g' \
			"${workdir}/patches/gcc-"*"/0007-Add-relative-RPATHs-to-GCC-host-tools.patch"
	fi
	
	if [[ "${host}" = *'-darwin'* ]]; then
		sed \
			--in-place \
			's/$$ORIGIN/@loader_path/g' \
			"${workdir}/patches/0007-Add-relative-RPATHs-to-GCC-host-tools.patch"
		
		sed \
			--in-place \
			's/$$ORIGIN/@loader_path/g' \
			"${workdir}/patches/gcc-"*"/0007-Add-relative-RPATHs-to-GCC-host-tools.patch"
	fi
	
	if (( gcc_major <= 10 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-10/0001-Fix-build-when-compiling-in-C-17-mode.patch"
	fi
	
	if (( gcc_major >= 8 && gcc_major <= 10 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-8/0001-Cygwin-MinGW-Do-not-version-lto-plugins.patch"
	elif (( gcc_major >= 5 && gcc_major <= 7 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-5/0001-MinGW-Do-not-version-lto-plugins.patch"
	elif (( gcc_major >= 4.6 && gcc_major <= 4.8 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-4.6/0001-MinGW-Do-not-version-lto-plugins.patch"
	elif (( gcc_major == 4.9 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-4.9/0001-MinGW-Do-not-version-lto-plugins.patch"
	fi
	
	if (( gcc_major >= 5 && gcc_major <= 7 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-7/0001-Fix-std-nullptr_t-to-bool-conversion-error.patch"
	fi
	
	if (( gcc_major >= 4.0 && gcc_major <= 4.2 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-4.0/0001-Add-host-support-for-x64-MinGW.patch"
	elif (( gcc_major <= 3.3 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-3.1/0001-Add-host-support-for-x64-MinGW.patch"
	elif (( gcc_major <= 3.4 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-3.4/0001-Add-host-support-for-x64-MinGW.patch"
	fi
	
	if (( gcc_major == 11 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-11/0001-Unpoison-calloc-on-musl-hosts.patch"
	fi
	
	if (( gcc_major >= 3.3 && gcc_major <= 3.4 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-${gcc_major}/0001-Force-disable-libunwind-support.patch"
	fi
	
	if (( gcc_major >= 5 && gcc_major <= 11 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-11/0001-Add-missing-sys-select.h-include-on-BSD.patch"
	fi
	
	if (( gcc_major >= 7 && gcc_major <= 11 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-7/0001-Darwin-Arm64-Initial-support-for-the-self-host-driver.patch"
	fi
	
	if (( gcc_major >= 5 && gcc_major <= 12 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-5/0001-Fix-definition-of-abort-on-Windows.patch"
	elif (( gcc_major >= 3.4 && gcc_major <= 4.9 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-3.4/0001-Fix-definition-of-abort-on-Windows.patch"
	elif (( gcc_major <= 3.3 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-3.1/0001-Fix-definition-of-abort-on-Windows.patch"
	fi
	
	if (( gcc_major >= 11 && gcc_major <= 12 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-11/0001-Fix-missing-definition-of-PTR-macro.patch"
	fi
	
	if (( gcc_major >= 4.9 && gcc_major <= 7 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-4.9/0001-Build-libcilkrts-with-D_GNU_SOURCE.patch"
	fi
	
	if (( gcc_major <= 4.9 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-3.1/0001-strerror.c-Do-not-declare-sys_nerr-or-sys_errlist-if-already-macros.patch"
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-3.1/0001-Avoid-incorrectly-declaring-the-caddr_t-alias-on-Linux.patch"
	fi
	
	if (( gcc_major <= 4.0 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-3.1/0001-Add-missing-argument-to-open-call.patch"
	fi
	
	if (( gcc_major >= 4.2 && gcc_major <= 4.7 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-4.2/0001-Support-Darwin-AArch64-host.patch"
	fi
	
	if (( gcc_major == 3.1 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-3.1/0001-Allow-compilation-with-GCC-4.4-and-up.patch"
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-3.1/0001-Fix-redeclaration-error.patch"
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-3.1/0001-Fix-locale-support-for-glibc-2.3.patch"
	fi
	
	if (( gcc_major >= 3.1 && gcc_major <= 3.2 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-3.1/0001-Don-t-error-out-for-unknown-platforms.patch"
	elif (( gcc_major == 3.3 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-3.3/0001-Don-t-error-out-for-unknown-platforms.patch"
	fi
	
	if (( gcc_major >= 3.3 && gcc_major <= 3.4 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-${gcc_major}/0001-Handle-enable-checking-release.patch"
	elif (( gcc_major >= 3.1 && gcc_major <= 3.2 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-3.1/0001-Handle-enable-checking-release.patch"
	fi
	
	if (( gcc_major >= 4.5 && gcc_major <= 4.8 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-4.5/0001-Add-missing-_attribute__-__gnu_inline__.patch"
	elif (( gcc_major >= 3.4 && gcc_major <= 4.4 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-3.4/0001-Add-missing-_attribute__-__gnu_inline__.patch"
	elif (( gcc_major >= 3.1 && gcc_major <= 3.2 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-3.1/0001-Add-missing-_attribute__-__gnu_inline__.patch"
	elif (( gcc_major == 3.3 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-3.3/0001-Add-missing-_attribute__-__gnu_inline__.patch"
	fi
	
	if (( gcc_major >= 4.1 && gcc_major <= 4.7 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-${gcc_major}/0001-Disable-building-documentation.patch"
	elif (( gcc_major >= 3.4 && gcc_major <= 4.0 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-3.4/0001-Disable-building-documentation.patch"
	elif (( gcc_major >= 3.1 && gcc_major <= 3.2 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-3.1/0001-Disable-building-documentation.patch"
	elif (( gcc_major == 3.3 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-3.3/0001-Disable-building-documentation.patch"
	fi
	
	if (( gcc_major >= 4.2 && gcc_major <= 4.4 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-4.2/0001-Fix-duplicate-declarations-of-_log2-functions.patch"
	elif (( gcc_major >= 4.0 && gcc_major <= 4.1 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-4.0/0001-Fix-duplicate-declarations-of-_log2-functions.patch"
	fi
	
	if (( gcc_major >= 4.6 && gcc_major <= 5 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-4.6/0001-Fix-wrong-usage-of-bool.patch"
	fi
	
	if (( gcc_major >= 6 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-6/0001-Prevent-use-of-_unlocked-functions.patch"
	elif (( gcc_major >= 4.0 && gcc_major <= 5 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-4.0/0001-Prevent-use-of-_unlocked-functions-and-disable-inclusion-of-malloc.h.patch"
	elif (( gcc_major == 3.4 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-${gcc_major}/0001-Prevent-use-of-_unlocked-functions-and-disable-inclusion-of-malloc.h.patch"
	elif (( gcc_major >= 3.1 && gcc_major <= 3.3 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-3.1/0001-Prevent-use-of-_unlocked-functions-and-disable-inclusion-of-malloc.h.patch"
	fi
	
	if (( gcc_major == 6 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-6/0001-PR-libstdc-87822-fix-layout-change-for-nested-std-pair.patch"
	fi
	
	if (( gcc_major >= 14 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0001-Turn-Wimplicit-function-declaration-back-into-an-warning.patch"
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-14/0001-Avoid-emitting-calls-to-__cxa_call_terminate-on-older-runtimes.patch"
	fi
	
	if (( gcc_major >= 13 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0002-Fix-libsanitizer-build-on-older-platforms.patch"
	fi
	
	if (( gcc_major >= 15 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0003-Change-the-default-language-version-for-C-compilation-from-std-gnu23-to-std-gnu17.patch"
	fi
	
	if (( gcc_major >= 16 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0004-Turn-Wimplicit-int-back-into-an-warning.patch"
	elif (( gcc_major >= 14 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-${gcc_major}/0004-Turn-Wimplicit-int-back-into-an-warning.patch"
	fi
	
	if (( gcc_major >= 15 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0005-Turn-Wint-conversion-back-into-an-warning.patch"
	elif (( gcc_major >= 14 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-${gcc_major}/0005-Turn-Wint-conversion-back-into-an-warning.patch"
	fi
	
	if (( gcc_major >= 16 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0006-Turn-Wincompatible-pointer-types-back-into-an-warning.patch"
	elif (( gcc_major >= 14 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-${gcc_major}/0006-Turn-Wincompatible-pointer-types-back-into-an-warning.patch"
	fi
	
	if (( gcc_major >= 15 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0007-Add-relative-RPATHs-to-GCC-host-tools.patch"
	elif (( gcc_major >= 6 && gcc_major <= 7 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-6/0007-Add-relative-RPATHs-to-GCC-host-tools.patch"
	elif (( gcc_major >= 3.1 && gcc_major <= 3.3 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-3.1/0007-Add-relative-RPATHs-to-GCC-host-tools.patch"
	else
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-${gcc_major}/0007-Add-relative-RPATHs-to-GCC-host-tools.patch"
	fi
	
	if (( gcc_major >= 4.8 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0010-Prefer-DT_RPATH-over-DT_RUNPATH.patch"
	elif (( gcc_major >= 4.7 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-4.7/0010-Prefer-DT_RPATH-over-DT_RUNPATH.patch"
	elif (( gcc_major >= 4.3 && gcc_major <= 4.6 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-4.3/0010-Prefer-DT_RPATH-over-DT_RUNPATH.patch"
	elif (( gcc_major >= 4.0 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-4.0/0010-Prefer-DT_RPATH-over-DT_RUNPATH.patch"
	elif (( gcc_major >= 3.3 && gcc_major <= 3.4 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-${gcc_major}/0010-Prefer-DT_RPATH-over-DT_RUNPATH.patch"
	elif (( gcc_major >= 3.1 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-3.1/0010-Prefer-DT_RPATH-over-DT_RUNPATH.patch"
	fi
	
	if (( gcc_major == 3.1 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-3.1/0001-Remove-hardcoded-paths-for-crt-files.patch"
	fi
	
	if (( gcc_major >= 16 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0011-Revert-configure-Always-add-pre-installed-header-directories-to-search-path.patch"
	fi
	
	if (( gcc_major >= 13 && gcc_major <= 15 )); then
		# Could be backported to GCC < 13, but it's not worth the effort
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-${gcc_major}/0001-Enable-automatic-linking-of-libatomic.patch"
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0001-Enable-automatic-linking-of-librt.patch"
	fi
	
	if (( gcc_major >= 4.6 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0001-AArch64-enable-libquadmath.patch"
	fi
	
	if (( gcc_major >= 14 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0001-Prevent-libstdc-from-trying-to-implement-math-stubs.patch"
	elif (( gcc_major >= 5 && gcc_major <= 13 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-5/0001-Prevent-libstdc-from-trying-to-implement-math-stubs.patch"
	elif (( gcc_major >= 4.4 && gcc_major <= 4.8 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-${gcc_major}/0001-Prevent-libstdc-from-trying-to-implement-math-stubs.patch"
	fi
	
	if (( gcc_major >= 15 )); then
		patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/0001-Ignore-header-files-under-prefix-system-root-include-missing.patch"
	elif (( gcc_major >= 12 )); then
		true # patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-12/0001-Ignore-header-files-under-prefix-system-root-include-missing.patch"
	elif (( gcc_major >= 10 )); then
		true # patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-10/0001-Ignore-header-files-under-prefix-system-root-include-missing.patch"
	elif (( gcc_major >= 7 )); then
		true # patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-7/0001-Ignore-header-files-under-prefix-system-root-include-missing.patch"
	elif (( gcc_major >= 4.3 && gcc_major <= 6 )); then
		true # patch --directory="${gcc_directory}" --strip='1' --input="${workdir}/patches/gcc-4.3/0001-Ignore-header-files-under-prefix-system-root-include-missing.patch"
	fi
	
	ln \
		--symbolic \
		--force \
		"${workdir}/tools/config.sub" \
		"${gcc_directory}/config.sub"
fi

if (( gcc_major <= 4.0)) && ! [ -f "${bison_tarball}" ]; then
	echo "https://github.com/Kartatz/bison-legacy/releases/latest/download/${build/-pc-/-unknown-}.tar.xz"
	
	declare version='2.7'
	
	if (( 1 || gcc_major <= 3.3 )); then
		version='2.3'
	fi
	
	curl \
		--url "https://github.com/AmanoTeam/bison-legacy/releases/download/${version}/${build/-pc-/-unknown-}.tar.xz" \
		--retry '30' \
		--retry-delay '0' \
		--retry-all-errors \
		--retry-max-time '0' \
		--location \
		--silent \
		--output "${bison_tarball}"
	
	tar \
		--directory="$(dirname "${bison_directory}")" \
		--extract \
		--file="${bison_tarball}"
	
	export PATH="${bison_directory}/bin:${PATH}"
	export BISON_PKGDATADIR="${bison_directory}/share/bison"
	
	bison --version
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
	"${gcc_directory}/libsanitizer/configure" || true

# Avoid using absolute hardcoded install_name values on macOS
sed \
	--in-place \
	's|-install_name \\$rpath/\\$soname|-install_name @rpath/\\$soname|g' \
	"${isl_directory}/configure" \
	"${mpc_directory}/configure" \
	"${mpfr_directory}/configure" \
	"${gmp_directory}/configure"

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

cmake \
	-S "${zlib_directory}" \
	-B "${PWD}" \
	-DCMAKE_INSTALL_PREFIX="${toolchain_directory}" \
	-DCMAKE_PLATFORM_NO_VERSIONED_SONAME='ON' \
	-DZLIB_BUILD_TESTING='OFF'

cmake --build "${PWD}" -- --jobs
cmake --install "${PWD}" --strip

make all --jobs
make install

[ -d "${yasm_directory}/build" ] || mkdir "${yasm_directory}/build"

cd "${yasm_directory}/build"

../configure \
	--build="${build}" \
	--host="${host}" \
	--prefix="${toolchain_directory}" \
	--enable-shared \
	--disable-static \
	CFLAGS="${ccflags}" \
	CXXFLAGS="${ccflags}" \
	LDFLAGS="${linkflags}"

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

cmake --build "${PWD}" -- --jobs
cmake --install "${PWD}" --strip

if (( build_nz )); then
	[ -d "${nz_directory}/build" ] || mkdir "${nz_directory}/build"
	
	cd "${nz_directory}/build"
	rm --force --recursive ./*
	
	cmake \
		-S "${nz_directory}" \
		-B "${PWD}" \
		-DCMAKE_C_FLAGS="${ccflags}" \
		-DCMAKE_CXX_FLAGS="${ccflags}" \
		-DCMAKE_INSTALL_PREFIX="${nz_prefix}"
	
	cmake --build "${PWD}" -- --jobs='1'
	cmake --install "${PWD}" --strip
	
	mkdir --parent "${toolchain_directory}/lib/nouzen"
	mv "${nz_prefix}/lib/"* "${toolchain_directory}/lib/nouzen"
	rmdir "${nz_prefix}/lib"
fi

[ -d "${ninja_directory}/build" ] || mkdir "${ninja_directory}/build"

cd "${ninja_directory}/build"
rm --force --recursive ./*

if [[ "${host}" != *'-android'* ]]; then
	cmake \
		-S "${ninja_directory}" \
		-B "${PWD}" \
		-DBUILD_TESTING='OFF' \
		-DCMAKE_POLICY_VERSION_MINIMUM='3.5' \
		-DCMAKE_INSTALL_PREFIX="${toolchain_directory}" \
		-DCMAKE_INSTALL_RPATH='$ORIGIN/../lib'
	
	cmake --build "${PWD}"
	cmake --install "${PWD}" --strip
fi

[ -d "${gold_directory}/build" ] || mkdir "${gold_directory}/build"

cd "${gold_directory}/build"

../configure \
	--build="${build}" \
	--host="${host}" \
	--target='arm-linux-gnueabi' \
	--prefix="${toolchain_directory}" \
	--enable-gold \
	--enable-lto \
	--enable-separate-code \
	--enable-rosegment \
	--enable-relro \
	--enable-compressed-debug-sections='all' \
	--enable-default-compressed-debug-sections-algorithm='zstd' \
	--disable-binutils \
	--disable-gas \
	--disable-install-libiberty \
	--disable-werror \
	--without-static-standard-libraries \
	--with-zstd="${toolchain_directory}" \
	--with-system-zlib \
	CFLAGS="-I${toolchain_directory}/include ${ccflags}" \
	CXXFLAGS="-I${toolchain_directory}/include ${ccflags}" \
	LDFLAGS="-L${toolchain_directory}/lib ${linkflags}"

make all --jobs

mkdir --parent "${toolchain_directory}/bin"

mv "${PWD}/gold/ld-new${exe}" "${toolchain_directory}/bin/ld.gold${exe}"
mv "${PWD}/gold/dwp${exe}" "${toolchain_directory}/bin/dwp${exe}"

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
declare strip='strip'

if ! (( native )); then
	cc="${CC}"
	readelf="${READELF}"
	strip="${STRIP}"
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
	LDFLAGS="${linkflags}" \
	all

if (( gcc_major <= 4.7 )); then
	targets=("${(@)targets:#aarch64-unknown-linux-gnu}")
fi

if (( gcc_major <= 4.6 )); then
	targets=("${(@)targets:#arm-unknown-linux-gnueabihf}")
fi

if (( gcc_major <= 4.0 )); then
	targets=("${(@)targets:#arm-unknown-linux-gnueabi}")
fi

for target in "${targets[@]}"; do
	check_target_exists "${gcc_targets}" "${target}" || continue
	
	declare specs=''
	
	if (( gcc_major >= 3.4 )); then
		specs+='%{!Qy: -Qn}'
		
		if (( gcc_major >= 4.8 )); then
			specs+=' %{!fgnu-unique: %{!fno-gnu-unique: -fno-gnu-unique}}'
		fi
	fi
	
	if (( native && gcc_major <= 3.4 )); then
		ln \
			--symbolic \
			--force \
			"$(where ranlib | head -n 1)" \
			"${build_directory}/${target}-ranlib"
	fi
	
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
	
	if (( gcc_major <= 4.6 )); then
		# Older GCC versions do not support --with-native-system-header-dir
		ln \
			--symbolic \
			--relative \
			--force \
			"${toolchain_directory}/${triplet}" \
			"${toolchain_directory}/${triplet}/usr"
	fi
	
	if (( gcc_major >= 13 && gcc_major <= 15 )); then
		cp "${workdir}/patches/librt_asneeded.so" "${toolchain_directory}/${triplet}/lib"
		cp "${workdir}/patches/librt_asneeded.a" "${toolchain_directory}/${triplet}/lib"
	fi
	
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
		--enable-leak-check \
		--disable-gprofng \
		--disable-gold \
		--disable-default-execstack \
		--disable-install-libiberty \
		--disable-werror \
		--with-sysroot="${toolchain_directory}/${triplet}" \
		--without-static-standard-libraries \
		--with-zstd="${toolchain_directory}" \
		--with-system-zlib \
		CFLAGS="-I${toolchain_directory}/include ${ccflags}" \
		CXXFLAGS="-I${toolchain_directory}/include ${ccflags}" \
		LDFLAGS="-L${toolchain_directory}/lib ${linkflags}"
	
	make all --jobs
	make install
	
	ln \
		--symbolic \
		--relative \
		--force \
		"${toolchain_directory}/bin/ld.gold${exe}" \
		"${toolchain_directory}/bin/${triplet}-ld.gold${exe}"
	
	ln \
		--symbolic \
		--relative \
		--force \
		"${toolchain_directory}/bin/dwp${exe}" \
		"${toolchain_directory}/bin/${triplet}-dwp${exe}"
	
	touch \
		"${toolchain_directory}/${triplet}/bin/ld.gold${exe}" \
		"${toolchain_directory}/${triplet}/bin/dwp${exe}"
	
	for bin in "${toolchain_directory}/${triplet}/bin/"*; do
		unlink "${bin}"
		cp "${binutils_gnu_wrapper}" "${bin}"
	done
	
	rm --force --recursive "${PWD}"
	
	if (( gcc_major >= 6 )) && ( [ "${triplet}" = 'x86_64-unknown-linux-gnu' ] || [ "${triplet}" = 'i386-unknown-linux-gnu' ] ); then
		specs+=' %{!fno-plt: %{!fplt: -fno-plt}}'
	fi
	
	if [[ "${triplet}" = 'mips'* ]]; then
		hash_style='sysv'
	fi
	
	if [[ "${host}" != *'-darwin'* ]] && [[ "${host}" != *'-mingw32' ]]; then
		extra_configure_flags+=' --enable-host-bind-now'
	fi
	
	if [[ "${host}" != *'-mingw32' ]]; then
		extra_configure_flags+=' --enable-host-pie'
		extra_configure_flags+=' --enable-host-shared'
	fi
	
	if ! (( native )); then
		extra_configure_flags+=" --with-cross-host=${host}"
		extra_configure_flags+=" --with-toolexeclibdir=${toolchain_directory}/${triplet}/lib/"
	fi
	
	if (( gcc_major <= 6 )); then
		# GCC 6 and earlier use isl_multi_val_set_val(), which was removed in
		# newer versions of isl. Using an outdated isl version just to make
		# the build succeed is not worth it.
		extra_configure_flags+=' --without-isl'
	fi
	
	if (( gcc_major <= 4.5 )); then
		# These versions require manually installing a third-party library (libelf) for LTO to work,
		# which isn’t worth the hassle here.
		extra_configure_flags+=' --disable-lto'
	fi
	
	if (( gcc_major <= 7 )) && [[ "${host}" = *'-mingw32' ]]; then
		extra_configure_flags+=' --disable-plugin'
	fi
	
	if (( gcc_major >= 4.5 )); then
		extra_configure_flags+=' --disable-c++-tools'
	fi
	
	declare -a args=(
		${=extra_configure_flags}
	)
	
	declare -a env=(
		"CFLAGS=-fPIC ${ccflags}"
		"CXXFLAGS=-fPIC ${ccflags}"
		"LDFLAGS=-L${toolchain_directory}/lib ${linkflags}"
	)
	
	[ -d "${gcc_directory}/build" ] || mkdir "${gcc_directory}/build"
	
	if (( gcc_major >= 3.1 && gcc_major <= 4.3 )) && [[ "${host}" != *'-darwin' ]]; then
		# -ftree-pre (enabled by default with -O2+) produces a broken g++ that
		# ICEs when parsing function declarations.
		printf "exec '%s' \"\${@}\" -fno-tree-pre\n" "${cc}" > "${build_directory}/cc"
		chmod +x "${build_directory}/cc"
		
		env+=(
			"CC=${build_directory}/cc"
		)
	fi
	
	cd "${gcc_directory}/build"
	rm --force --recursive ./*
	
	env "${env[@]}" \
		../configure \
		--build="${build/unknown-/}" \
		--host="${host}" \
		--target="${triplet}" \
		--prefix="${toolchain_directory}" \
		--with-linker-hash-style='both' \
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
		--enable-autolink-librt \
		--enable-libgomp \
		--enable-libstdcxx-verbose \
		--with-specs="${specs}" \
		--with-pic \
		--with-gnu-as \
		--with-gnu-ld \
		"${args[@]}" \
		--disable-libsanitizer \
		--disable-bootstrap \
		--disable-libstdcxx-pch \
		--disable-libstdcxx-debug \
		--disable-libmudflap \
		--disable-werror \
		--disable-multilib \
		--disable-nls \
		--disable-canonical-system-headers \
		--disable-win32-utf8-manifest \
		--without-static-standard-libraries
	
	env=()
	
	if (( native )); then
		env=(
			"LD_LIBRARY_PATH=${toolchain_directory}/lib"
			"PATH=${PATH}:${toolchain_directory}/bin"
		)
	fi
	
	if (( gcc_major <= 3.2 )); then
		unlink "${build_directory}/ln"
	fi
	
	env "${env[@]}" make \
		gcc_cv_objdump="${host}-objdump" \
		all \
		--jobs="${max_jobs}"
	
	if (( gcc_major <= 3.2 )); then
		cp "${workdir}/tools/ln.sh" "${build_directory}/ln"
	fi
	
	make install
	
	if (( gcc_major <= 4.6 )); then
		unlink "${toolchain_directory}/${triplet}/usr"
	fi
	
	if (( gcc_major <= 4.1 )); then
		mv \
			"${toolchain_directory}/include/c++" \
			"${toolchain_directory}/${triplet}/include"
	fi
	
	if (( gcc_major <= 3.3 )); then
		ln \
			--symbolic \
			--relative \
			"${toolchain_directory}/lib/gcc-lib" \
			"${toolchain_directory}/lib/gcc"
		
		ln \
			--symbolic \
			--relative \
			"${toolchain_directory}/lib/gcc" \
			"${toolchain_directory}/libexec"
	fi
	
	if (( gcc_major <= 6 )); then
		# There was no --with-gcc-major-version-only back then
		if (( gcc_major >= 3.4 )); then
			ln \
				--symbolic \
				--relative \
				"${toolchain_directory}/libexec/gcc/${triplet}/${gcc_major}."* \
				"${toolchain_directory}/libexec/gcc/${triplet}/${gcc_major}"
		fi
		
		ln \
			--symbolic \
			--relative \
			"${toolchain_directory}/lib/gcc/${triplet}/${gcc_major}."* \
			"${toolchain_directory}/lib/gcc/${triplet}/${gcc_major}"
		
		ln \
			--symbolic \
			--relative \
			"${toolchain_directory}/${triplet}/include/c++/${gcc_major}."* \
			"${toolchain_directory}/${triplet}/include/c++/${gcc_major}"
	fi
	
	if (( gcc_major >= 4.3 && gcc_major <= 11 )); then
		ln \
			--symbolic \
			--relative \
			"${toolchain_directory}/lib/gcc/${triplet}/${gcc_major}/install-tools/include/limits.h" \
			"${toolchain_directory}/lib/gcc/${triplet}/${gcc_major}/include"
		
		ln \
			--symbolic \
			--relative \
			"${toolchain_directory}/lib/gcc/${triplet}/${gcc_major}/install-tools/gsyslimits.h" \
			"${toolchain_directory}/lib/gcc/${triplet}/${gcc_major}/include/syslimits.h"
	fi
	
	cd "${toolchain_directory}/${triplet}/lib64" 2>/dev/null || cd "${toolchain_directory}/${triplet}/lib"
	
	if [[ "$(basename "${PWD}")" = 'lib64' ]]; then
		mv './'* '../lib' || true
		rmdir "${PWD}"
		cd '../lib'
	fi
	
	"${triplet}-strip" "${PWD}/lib"*'.so' 2>/dev/null || true
	
	if (( gcc_major >= 13 && gcc_major <= 15 )); then
		patch --directory="${PWD}" --strip='1' --input="${workdir}/patches/0001-Workaround-mold-linker-issue.patch"
	fi
	
	if (( gcc_major >= 4.1 )); then
		env "${env[@]}" make \
			-C "${workdir}/tools/libblocksruntime" \
			PREFIX="${toolchain_directory}/${triplet}" \
			CC="${triplet}-gcc" \
			CFLAGS="${ccflags}" \
			CXXFLAGS="${ccflags}" \
			LDFLAGS="${linkflags}"
	fi
	
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
	
	rm \
		--force \
		"${clang_include_dir}/"*'intrin'*'.h' \
		"${clang_include_dir}/arm"*'.h' \
		"${clang_include_dir}/"*'3dnow'*'.h' \
		"${clang_include_dir}/stdatomic.h"
	
	rm --force './libiberty.a'
	
	if (( gcc_major >= 3.1 )); then
		cp  \
			"${gcc_directory}/gcc/gsyslimits.h" \
			"${toolchain_directory}/lib/gcc/${triplet}/${gcc_major}/include/syslimits.h"
		
		mv "${toolchain_directory}/lib"{64,}"/libgcc"* "${PWD}" 2>/dev/null || true
	fi
	
	rm --force './libgcc_s.so'
	echo 'GROUP ( libgcc_s.so.1 -lgcc )' > './libgcc_s.so'
	
	if (( gcc_major <= 4.8 )); then
		rm --force "${toolchain_directory}/lib/libiberty.a"
	fi
	
	rm \
		--force \
		"${toolchain_directory}/bin/${triplet}-${triplet}-"* \
		"${toolchain_directory}/bin/${triplet}-gcc-${gcc_major}${exe}"
	
	if [[ "${host}" = *'-mingw32' ]]; then
		unlink "${toolchain_directory}/bin/${triplet}-ld${exe}"
		unlink "${toolchain_directory}/bin/${triplet}-c++${exe}"
		cp "${toolchain_directory}/bin/${triplet}-ld.bfd${exe}" "${toolchain_directory}/bin/${triplet}-ld${exe}"
		cp "${toolchain_directory}/bin/${triplet}-g++${exe}" "${toolchain_directory}/bin/${triplet}-c++${exe}"
	fi
	
	cat "${workdir}/patches/c++config.h" >> "${toolchain_directory}/${triplet}/include/c++/${gcc_major}/${triplet}/bits/c++config.h"
	
	if (( gcc_major >= 4.6 )); then
		if [[ "${host}" = *'-mingw32' ]]; then
			cp \
				"${toolchain_directory}/libexec/gcc/${triplet}/${gcc_major}/liblto_plugin${dll}" \
				"${toolchain_directory}/lib/bfd-plugins"
		else
			ln \
				--symbolic \
				--relative \
				--force \
				"${toolchain_directory}/libexec/gcc/${triplet}/${gcc_major}/liblto_plugin${dll}" \
				"${toolchain_directory}/lib/bfd-plugins"
		fi
	fi
	
	rename_soname_libraries "${toolchain_directory}/${triplet}"
	
	if [[ "${host}" = *'-mingw32' ]]; then
		replace_symlinks "${toolchain_directory}/${triplet}" || true
	fi
done

while read triplet; do
	if ! [ -d "${toolchain_directory}/${triplet}" ]; then
		continue
	fi
	
	declare url="https://github.com/AmanoTeam/libsanitizer/releases/download/gcc-${gcc_major}/${triplet}.tar.xz"
	
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
		--file="${libsanitizer_tarball}" || continue
	
	cp --recursive "${libsanitizer_directory}/lib/gcc" "${toolchain_directory}/lib"
	cp --recursive "${libsanitizer_directory}/lib/lib"* "${toolchain_directory}/${triplet}/lib"
	
	rm --recursive "${libsanitizer_directory}"
done <<< "$(jq --raw-output --compact-output '.[]' "${workdir}/submodules/libsanitizer/triplets.json")"

declare url="https://github.com/AmanoTeam/GDB-Builds/releases/latest/download/${host/-pc-/-unknown-}.tar.xz"

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
	cd "${gdb_directory}/bin"
	
	for name in *-{gdb,gdb-add-index,gstack,run,gcore}; do
		value="${name/-gdb-add-index/}"
		value="${value/-gstack/}"
		value="${value/-run/}"
		value="${value/-gdb/}"
		value="${value/-gcore/}"
		
		if check_target_exists "${gcc_targets}" "${value}"; then
			continue
		fi
		
		echo "- Removing ${name}"
		unlink "${name}"
	done
	
	rm --force "${gdb_directory}/bin/"*'android'*
	rm --force "${gdb_directory}/bin/armv"*
	
	cp --recursive "${gdb_directory}/bin" "${toolchain_directory}"
	rm --recursive "${gdb_directory}"
fi

# Delete libtool files and other unnecessary files GCC installs
rm \
	--force \
	--recursive \
	"${toolchain_directory}/share" \
	"${toolchain_directory}/info" \
	"${toolchain_directory}/man" \
	"${toolchain_directory}/lib/lib"*'.a' \
	"${toolchain_directory}/include" \
	"${toolchain_directory}/lib/pkgconfig" \
	"${toolchain_directory}/lib/cmake"

find \
	"${toolchain_directory}" \
	-name '*.la' -delete -o \
	-name '*.py' -delete -o \
	-name '*.json' -delete

cd "${workdir}"

# Bundle both libstdc++ and libgcc within host tools
if ! (( native )) && [[ "${host}" != *'-darwin'* ]]; then
	[ -d "${toolchain_directory}/lib" ] || mkdir "${toolchain_directory}/lib"
	
	# libestdc++
	declare name=$(realpath $("${cc}" --print-file-name="libestdc++${dll}"))
	
	# libstdc++
	if ! [ -f "${name}" ]; then
		declare name=$(realpath $("${cc}" --print-file-name="libstdc++${dll}"))
	fi
	
	declare soname=''
	
	if [[ "${host}" != *'-mingw32' ]]; then
		soname=$("${readelf}" -d "${name}" | grep 'SONAME' | sed --regexp-extended 's/.+\[(.+)\]/\1/g')
	fi
	
	cp "${name}" "${toolchain_directory}/lib/${soname}"
	
	if [[ "${host}" = *'-mingw32' ]]; then
		cp "${name}" "${toolchain_directory}/bin/${soname}"
	fi
	
	if (( build_nz )); then
		ln \
			--symbolic \
			--relative \
			"${toolchain_directory}/lib/${soname}" \
			"${toolchain_directory}/lib/nouzen"
	fi
	
	# libegcc
	declare name=$(realpath $("${cc}" --print-file-name="libegcc${dll}"))
	
	if ! [ -f "${name}" ]; then
		# libgcc_s.so.1
		declare name=$(realpath $("${cc}" --print-file-name="libgcc_s${dll}.1"))
	fi
	
	if ! [ -f "${name}" ]; then
		# libgcc_s
		declare name=$(realpath $("${cc}" --print-file-name="libgcc_s${dll}"))
	fi
	
	if [[ "${host}" = *'-mingw32' ]]; then
		if ! [ -f "${name}" ]; then
			# libgcc_s_seh
			declare name=$(realpath $("${cc}" --print-file-name="libgcc_s_seh${dll}"))
		fi
		
		if ! [ -f "${name}" ]; then
			# libgcc_s_sjlj
			declare name=$(realpath $("${cc}" --print-file-name="libgcc_s_sjlj${dll}"))
		fi
	fi
	
	if [[ "${host}" != *'-mingw32' ]]; then
		soname=$("${readelf}" -d "${name}" | grep 'SONAME' | sed --regexp-extended 's/.+\[(.+)\]/\1/g')
	fi
	
	cp "${name}" "${toolchain_directory}/lib/${soname}"
	
	if [[ "${host}" = *'-mingw32' ]]; then
		cp "${name}" "${toolchain_directory}/bin/${soname}"
	fi
	
	if (( build_nz )); then
		ln \
			--symbolic \
			--relative \
			"${toolchain_directory}/lib/${soname}" \
			"${toolchain_directory}/lib/nouzen"
	fi
	
	# libatomic
	declare name=$(realpath $("${cc}" --print-file-name="libatomic${dll}"))
	
	if [[ "${host}" != *'-mingw32' ]]; then
		soname=$("${readelf}" -d "${name}" | grep 'SONAME' | sed --regexp-extended 's/.+\[(.+)\]/\1/g')
	fi
	
	cp "${name}" "${toolchain_directory}/lib/${soname}"
	
	if [[ "${host}" = *'-mingw32' ]]; then
		cp "${name}" "${toolchain_directory}/bin/${soname}"
	fi
	
	if (( build_nz )); then
		ln \
			--symbolic \
			--relative \
			"${toolchain_directory}/lib/${soname}" \
			"${toolchain_directory}/lib/nouzen"
	fi
	
	# libiconv
	declare name=$(realpath $("${cc}" --print-file-name="libiconv${dll}"))
	
	if [ -f "${name}" ]; then
		if [[ "${host}" != *'-mingw32' ]]; then
			soname=$("${readelf}" -d "${name}" | grep 'SONAME' | sed --regexp-extended 's/.+\[(.+)\]/\1/g')
		fi
		
		cp "${name}" "${toolchain_directory}/lib/${soname}"
		
		if [[ "${host}" = *'-mingw32' ]]; then
			cp "${name}" "${toolchain_directory}/bin/${soname}"
		fi
		
		if (( build_nz )); then
			ln \
				--symbolic \
				--relative \
				"${toolchain_directory}/lib/${soname}" \
				"${toolchain_directory}/lib/nouzen"
		fi
	fi
	
	# libcharset
	declare name=$(realpath $("${cc}" --print-file-name="libcharset${dll}"))
	
	if [ -f "${name}" ]; then
		if [[ "${host}" != *'-mingw32' ]]; then
			soname=$("${readelf}" -d "${name}" | grep 'SONAME' | sed --regexp-extended 's/.+\[(.+)\]/\1/g')
		fi
		
		cp "${name}" "${toolchain_directory}/lib/${soname}"
		
		if [[ "${host}" = *'-mingw32' ]]; then
			cp "${name}" "${toolchain_directory}/bin/${soname}"
		fi
		
		if (( build_nz )); then
			ln \
				--symbolic \
				--relative \
				"${toolchain_directory}/lib/${soname}" \
				"${toolchain_directory}/lib/nouzen"
		fi
	fi
	
	if [[ "${host}" = *'-mingw32' ]]; then
		# libwinpthread
		declare name=$(realpath $("${cc}" --print-file-name="libwinpthread-1${dll}"))
		cp "${name}" "${toolchain_directory}/bin/${soname}"
	fi
	
	if [[ "${host}" = *'-mingw32' ]]; then
		for target in "${targets[@]}"; do
			for source in "${toolchain_directory}/"{bin,lib}"/lib"*'.dll'; do
				cp "${source}" "${toolchain_directory}/libexec/gcc/${target}/${gcc_major}"
			done
		done
		
		rm "${toolchain_directory}/lib/lib"*'.'{dll,lib}
	fi
fi

cp "${binutils_llvm_wrapper}" "${toolchain_directory}/bin/llvm-objcopy${exe}"
cp "${binutils_llvm_wrapper}" "${toolchain_directory}/bin/llvm-strip${exe}"

while read item; do
	declare glibc_version="$(jq '.glibc_version' <<< "${item}")"
	declare triplet="$(jq --raw-output '.triplet' <<< "${item}")"
	
	check_target_exists "${gcc_targets}" "${triplet}${glibc_version}" || continue
	
	if (( gcc_major <= 4.7 )) && [[ "${triplet}" = 'aarch64'* ]]; then
		continue
	fi
	
	if (( gcc_major <= 4.6 )) && [[ "${triplet}" = 'arm-'*'-gnueabihf' ]]; then
		continue
	fi
	
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
	
	cp "${gcc_wrapper}" "${toolchain_directory}/bin/${triplet}${glibc_version}-gcc${exe}"
	cp "${gcc_wrapper}" "${toolchain_directory}/bin/${triplet}${glibc_version}-g++${exe}"
	cp "${gcc_wrapper}" "${toolchain_directory}/bin/${triplet}${glibc_version}-c++${exe}"
	
	cp "${clang_wrapper}" "${toolchain_directory}/bin/${triplet}${glibc_version}-clang${exe}"
	cp "${clang_wrapper}" "${toolchain_directory}/bin/${triplet}${glibc_version}-clang++${exe}"
	
	if [[ "${languages}" = *'m2'* ]]; then
		cp "${gcc_wrapper}" "${toolchain_directory}/bin/${triplet}${glibc_version}-gm2${exe}"
	fi
	
	if [[ "${languages}" = *'fortran'* ]]; then
		cp "${gcc_wrapper}" "${toolchain_directory}/bin/${triplet}${glibc_version}-gfortran${exe}"
	fi
	
	cp "${workdir}/tools/pkg-config.sh" "${toolchain_directory}/bin/${triplet}${glibc_version}-pkg-config"
	
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
	
	rename_soname_libraries "${toolchain_directory}/${triplet}${glibc_version}"
	
	mkdir "${toolchain_directory}/${triplet}${glibc_version}/lib/"{gcc,static}
	
	ln \
		--symbolic \
		--relative \
		"${toolchain_directory}/${triplet}${glibc_version}/lib/lib"*'.'{so,a}* \
		"${toolchain_directory}/${triplet}${glibc_version}/lib/static"
	
	ln \
		--symbolic \
		--relative \
		"${toolchain_directory}/${triplet}${glibc_version}/lib/ld-"*'.so'* \
		"${toolchain_directory}/${triplet}${glibc_version}/lib/static"
	
	ln \
		--symbolic \
		--relative \
		"${toolchain_directory}/${triplet}${glibc_version}/lib/"*'.o' \
		"${toolchain_directory}/${triplet}${glibc_version}/lib/static"
	
	if (( gcc_major >= 4.1 )); then
		ln \
			--symbolic \
			--relative \
			"${toolchain_directory}/${triplet}/include/Block"* \
			"${toolchain_directory}/${triplet}${glibc_version}/include"
		
		ln \
			--symbolic \
			--relative \
			"${toolchain_directory}/${triplet}/lib/libBlocksRuntime.a" \
			"${toolchain_directory}/${triplet}${glibc_version}/lib"
		
		ln \
			--symbolic \
			--relative \
			"${toolchain_directory}/${triplet}/lib/libBlocksRuntime.a" \
			"${toolchain_directory}/${triplet}${glibc_version}/lib/static"
	fi
	
	ln \
		--symbolic \
		--relative \
		"${toolchain_directory}/lib/gcc/${triplet}/${gcc_major}/"*'.'{a,o} \
		"${toolchain_directory}/${triplet}${glibc_version}/lib"
	
	ln \
		--symbolic \
		--relative \
		"${toolchain_directory}/lib/gcc/${triplet}/${gcc_major}/"*'.'{a,o} \
		"${toolchain_directory}/${triplet}${glibc_version}/lib/static"
	
	if (( gcc_major >= 13 && gcc_major <= 15 )); then
		ln \
			--symbolic \
			--relative \
			"${toolchain_directory}/${triplet}/lib/librt_asneeded.so" \
			"${toolchain_directory}/${triplet}${glibc_version}/lib/librt_asneeded.so"
		
		ln \
			--symbolic \
			--relative \
			"${toolchain_directory}/${triplet}/lib/librt_asneeded.a" \
			"${toolchain_directory}/${triplet}${glibc_version}/lib/librt_asneeded.a"
		
		ln \
			--symbolic \
			--relative \
			"${toolchain_directory}/${triplet}/lib/librt_asneeded.so" \
			"${toolchain_directory}/${triplet}${glibc_version}/lib/static/librt_asneeded.so"
	fi
	
	if [ "${repository}" != 'null' ] && (( build_nz )); then
		mkdir "${toolchain_directory}/${triplet}${glibc_version}/lib/nouzen"
		
		cp \
			--recursive "${nz_prefix}/"* \
			"${toolchain_directory}/${triplet}${glibc_version}/lib/nouzen"
		
		mkdir \
			--parent \
			"${toolchain_directory}/${triplet}${glibc_version}/lib/nouzen/lib"
		
		ln \
			--symbolic \
			--relative \
			"${toolchain_directory}/lib/nouzen/lib"* \
			"${toolchain_directory}/${triplet}${glibc_version}/lib/nouzen/lib"
		
		mkdir \
			--parent \
			"${toolchain_directory}/${triplet}${glibc_version}/lib/nouzen/etc/nouzen/sources.list"
		
		echo -e \
			"repository = ${repository}\nrelease = ${release}\nresource = ${resource}\narchitecture = ${architecture}\nformat = apt" > "${toolchain_directory}/${triplet}${glibc_version}/lib/nouzen/etc/nouzen/sources.list/obggcc.conf"
		
		mkdir "${toolchain_directory}/${triplet}${glibc_version}/bin"
		
		ln \
			--symbolic \
			--relative \
			"${toolchain_directory}/${triplet}${glibc_version}/lib/nouzen/bin/"* \
			"${toolchain_directory}/${triplet}${glibc_version}/bin"
		
		ln \
			--symbolic \
			--relative \
			"${toolchain_directory}/${triplet}${glibc_version}/bin/nz" \
			"${toolchain_directory}/bin/${triplet}${glibc_version}-nz"
		
		ln \
			--symbolic \
			--relative \
			"${toolchain_directory}/${triplet}${glibc_version}/bin/apt" \
			"${toolchain_directory}/bin/${triplet}${glibc_version}-apt"
		
		ln \
			--symbolic \
			--relative \
			"${toolchain_directory}/${triplet}${glibc_version}/bin/apt-get" \
			"${toolchain_directory}/bin/${triplet}${glibc_version}-apt-get"
	fi
	
	for library in "${libraries[@]}"; do
		for bit in "${bits[@]}"; do
			for file in "${toolchain_directory}/${triplet}/lib${bit}/${library}"*; do
				if [[ "${file}" = *'*' ]]; then
					continue
				fi
				
				if ! ( [[ "${file}" = *'.so'* ]] || [[ "${file}" = *'.a' ]] ); then
					continue
				fi
				
				ln \
					--force \
					--symbolic \
					--relative \
					"${file}" \
					"${toolchain_directory}/${triplet}${glibc_version}/lib"
				
				ln \
					--force \
					--symbolic \
					--relative \
					"${file}" \
					"${toolchain_directory}/${triplet}${glibc_version}/lib/gcc"
				
				if [[ "${file}" = *'.a' ]]; then
					ln \
						--force \
						--symbolic \
						--relative \
						"${file}" \
						"${toolchain_directory}/${triplet}${glibc_version}/lib/static"
				fi
			done
		done
	done
	
	for name in "${symlink_tools[@]}"; do
		source="${toolchain_directory}/bin/${triplet}-${name}"
		destination="${toolchain_directory}/bin/${triplet}${glibc_version}-${name}"
		
		if ! [ -f "${source}" ]; then
			continue
		fi
		
		echo "- Symlinking '${source}' to '${destination}'"
		
		ln \
			--symbolic \
			--relative \
			"${source}" \
			"${destination}"
	done
	
	if [[ "${host}" = *'-mingw32' ]]; then
		replace_symlinks "${toolchain_directory}/${triplet}${glibc_version}"
	fi
done <<< "$(jq --compact-output '.[]' "${workdir}/submodules/debian-sysroot/dist.json")"

if (( gcc_major >= 15 )); then
	for triplet in "${targets[@]}"; do
		python3 -B "${workdir}/tools/include-missing/main.py" "${toolchain_directory}" "${triplet}"
	done
fi

mkdir --parent "${share_directory}"

cp --recursive "${workdir}/tools/dev/"* "${share_directory}"

[ -d "${toolchain_directory}/build" ] || mkdir "${toolchain_directory}/build"

ln \
	--symbolic \
	--relative \
	"${share_directory}/"* \
	"${toolchain_directory}/build"

for filename in "${toolchain_directory}/build/"*'/'*'.'{cmake,sh} "${toolchain_directory}/build/"*'/'*'/'*'.'{cmake,sh}; do
	target="$(basename "${filename}")"
	target="${target/.sh/}"
	target="${target/.cmake/}"
	
	if check_target_exists "${gcc_targets}" "${target}"; then
		continue
	fi
	
	echo "- Removing ${filename}"
	unlink "${filename}"
done

if [[ "${host}" = *'-mingw32' ]]; then
	while read name; do
		if [ -f "${name}" ]; then
			chmod 644 "${name}"
		elif [ -d "${name}" ]; then
			chmod 755 "${name}"
 		fi
	done <<< "$(find "${toolchain_directory}")"
fi

if (( gcc_major >= 15 )); then
	cp "${workdir}/tools/gcc-stl-install.sh" "${toolchain_directory}/bin/gcc-stl-install"
fi

if (( gcc_major <= 4.7 )); then
	rm \
		--force \
		"${toolchain_directory}/bin/aarch64"* \
		"${toolchain_directory}/build/"*"/aarch64"* \
		"${toolchain_directory}/build/"*"/"*"/aarch64"*
fi

if (( gcc_major <= 4.6 )); then
	rm \
		--force \
		"${toolchain_directory}/bin/"*"gnueabihf"* \
		"${toolchain_directory}/build/"*"/"*"gnueabihf"* \
		"${toolchain_directory}/build/"*"/"*"/"*"gnueabihf"*
fi

if (( gcc_major <= 4.0 )); then
	rm \
		--force \
		"${toolchain_directory}/bin/"*"gnueabi"* \
		"${toolchain_directory}/build/"*"/"*"gnueabi"* \
		"${toolchain_directory}/build/"*"/"*"/"*"gnueabi"*
fi

rm \
	--force \
	"${toolchain_directory}/bin/"*"armv6"* \
	"${toolchain_directory}/build/"*"/"*"armv6"* \
	"${toolchain_directory}/build/"*"/"*"/"*"armv6"*

while read name; do
	declare type="$(file --brief --mime-type "${name}")"
	
	if ! [[ "${type}" = *'sharedlib' || "${type}" = *'executable' ]]; then
		continue
	fi
	
	"${strip}" "${name}" 2>/dev/null || true
done <<< "$(find "${toolchain_directory}" -type 'f')"

if ( find "${toolchain_directory}" -xtype 'l' -print -quit | grep -q . ); then
	echo '* broken symbolic links *' && find "${toolchain_directory}" -xtype 'l'
	exit '1'
fi
