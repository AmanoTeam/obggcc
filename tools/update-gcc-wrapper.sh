#!/bin/bash

set -e #u

declare -r app_directory="$(realpath "$(( [ -n "${BASH_SOURCE}" ] && dirname "$(realpath "${BASH_SOURCE[0]}")" ) || dirname "$(realpath "${0}")")")"

declare -r gcc_wrapper='/tmp/gcc-wrapper'
declare -r binutils_wrapper='/tmp/binutils-wrapper'

declare -r source_directory='/tmp/obggcc-source'

declare source=''
declare destination=''

function copy() {
	
	local source="${1}"
	local destination="${2}"
	
	echo "- Copying ${source} to ${destination}"
	cp "${source}" "${destination}"

}

[ -d "${source_directory}" ] && rm --force --recursive "${source_directory}"

git clone \
	--depth '1' \
	'https://github.com/AmanoTeam/obggcc.git' \
	"${source_directory}"

make \
	-C "${source_directory}/tools/gcc-wrapper" \
	PREFIX="$(dirname "${gcc_wrapper}")" \
	FLAVOR='OBGGCC' \
	all

source="${gcc_wrapper}"

for name in "${app_directory}/"*-{gcc,g++,clang,clang++}; do
	destination="${name}"
	base="$(basename "${destination}" | awk -F  '-' '{print $4}')"
	
	[[ "${base: -1}" =~ [0-9] ]] || continue
	
	copy "${source}" "${destination}"
done

destination="${app_directory}/clang"

copy "${source}" "${destination}"

destination="${app_directory}/clang++"

copy "${source}" "${destination}"

unlink "${source}"

source="${binutils_wrapper}"

destination="${app_directory}/llvm-strip"

copy "${source}" "${destination}"

destination="${app_directory}/llvm-objcopy"

copy "${source}" "${destination}"

unlink "${source}"
