#!/bin/bash

declare -r workdir="${PWD}"

declare -ra versions=(
	'4'
	'5'
	'6'
	'7'
	'8'
	'9'
	'10'
	'11'
	'12'
	'13'
	'14'
	'15'
	'16'
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
)

declare -ra targets=(
	'i386-unknown-linux-gnu'
	'arm-unknown-linux-gnueabi'
	'arm-unknown-linux-gnueabihf'
	'aarch64-unknown-linux-gnu'
	'x86_64-unknown-linux-gnu'
)

for version in "${versions[@]}"; do
	declare url="https://github.com/AmanoTeam/obggcc/releases/download/gcc-${version}/x86_64-unknown-linux-gnu.tar.xz"
	
	rm \
		--force \
		--recursive "${PWD}/obggcc"
	
	echo "- Downloading data from ${url}"
	
	curl \
		--url "${url}" \
		--retry '30' \
		--retry-delay '0' \
		--retry-all-errors \
		--retry-max-time '0' \
		--location \
		--silent \
		--output '-' \
			| tar \
				--xz \
				--extract \
				--file='-'
	
	for target in "${targets[@]}"; do
		echo "- Copying GCC ${version} libraries (${target})"
		
		rm \
			--force \
			--recursive "${PWD}/${target}/${version}/"*
		
		mkdir --parent "${PWD}/${target}/${version}/"{lib,include}
		mkdir "${PWD}/${target}/${version}/lib/static"
		
		for library in "${libraries[@]}"; do
			mv \
				"${PWD}/obggcc/${target}/lib/${library}"* \
				"${PWD}/${target}/${version}/lib" 2>/dev/null
		done
		
		find "${PWD}/${target}/${version}/lib" -xtype 'l' -delete
		
		mv \
			--force \
			"${PWD}/obggcc/lib/gcc/${target}/${version}/"*'.'{a,o} \
			"${PWD}/${target}/${version}/lib"
		
		ln \
			--symbolic \
			--relative \
			"${PWD}/${target}/${version}/lib/"*'.'{a,o} \
			"${PWD}/${target}/${version}/lib/static"
		
		mv \
			"${PWD}/obggcc/${target}/include/c++" \
			"${PWD}/${target}/${version}/include"
		
		cp \
			--recursive \
			"${PWD}/${target}/${version}/include/c++/${version}/${target}/"* \
			"${PWD}/${target}/${version}/include/c++/${version}"
		
		if (( version <= 5 )); then
			patch --directory="${PWD}/${target}/${version}/include/c++/${version}" --strip='1' --input="${workdir}/patches/gcc-stl/gcc-4/0001-Backport-__is_nothrow_swappable.patch"
		fi
	done
done

for target in "${targets[@]}"; do
	echo "- Creating tarball for ${target} libraries"
	
	tar \
		--directory="${PWD}" \
		--create \
		--file=- \
		"${target}" \
			| xz \
				--threads='0' \
				--extreme \
				--memlimit-compress='100%' \
				--compress \
				-9 \
					> "${PWD}/${target}.tar.xz"
done