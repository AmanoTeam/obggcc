#!/bin/bash

if [ -z "${OBGGCC_HOME}" ]; then
	OBGGCC_HOME="$(realpath "$(( [ -n "${BASH_SOURCE}" ] && dirname "$(realpath "${BASH_SOURCE[0]}")" ) || dirname "$(realpath "${0}")")""/..")"
fi

declare -r triplet="${1}"

declare -ra targets=(
	'i386-unknown-linux-gnu'
	'arm-unknown-linux-gnueabi'
	'arm-unknown-linux-gnueabihf'
	'aarch64-unknown-linux-gnu'
	'x86_64-unknown-linux-gnu'
)

set -eu

if [ -z "${triplet}" ]; then
	echo 'Usage: gcc-stl-install <triplet>' 1>&2
	exit '1'
fi

declare -r stl_directory="${OBGGCC_HOME}/build/gcc-stl"

declare status='0'

for target in "${targets[@]}"; do
	if [ "${triplet}" != "${target}" ]; then
		continue
	fi
	
	status='1'
		
	break
done

if ! (( status )); then
	echo "fatal error: unknown triplet: ${triplet}" 1>&2
	exit '1'
fi

mkdir --parent "${stl_directory}"

declare url="https://github.com/AmanoTeam/obggcc/releases/download/gcc-stl/${triplet}.tar.xz"

echo "- downloading data from ${url}"

curl \
	--url "${url}" \
	--retry '3' \
	--retry-delay '0' \
	--retry-all-errors \
	--retry-max-time '0' \
	--location \
	--silent \
	--output '-' \
	--show-error \
		| tar \
			--xz \
			--directory="${stl_directory}" \
			--extract \
			--file='-'

for directory in "${stl_directory}/${triplet}/"*; do
	declare filename="${directory}/lib/libgcc_n.a"
	
	echo "- creating '${filename}'"
	echo "INPUT ( AS_NEEDED ( ../../../../../${triplet}/lib/libgcc.a ) )" > "${filename}"
	
	filename="${directory}/lib/static/libgcc_n.a"
	
	echo "- creating '${filename}'"
	echo "INPUT ( AS_NEEDED ( ../../../../../../${triplet}/lib/libgcc.a ) )" > "${filename}"
done

echo "- installed GCC runtime libraries to '${stl_directory}/${triplet}'"
