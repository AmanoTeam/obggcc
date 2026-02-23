#!/bin/bash

function replace_symlinks() {
	
	echo "Removing symlinks from '${1}'"
	
	while read source; do
		if [[ "${source}" = *'ldscripts'* ]]; then
			continue
		fi
		
		destination="$(readlink "${source}")"
		
		echo "Replacing symlink at '${source}' with a linker script"
		
		unlink "${source}"
		echo "INPUT(${destination})" > "${source}"
	done <<< "$(find "${1}/lib" -type 'l')"
	
	if [  -h "${1}/include" ]; then
		return
	fi
	
	while read source; do
		if [ -z "${source}" ]; then
			break
		fi
		
		destination="$(realpath "${source}")"
		
		echo "Replacing symlink at '${source}' with the actual file"
		
		unlink "${source}"
		cp "${destination}" "${source}"
	done <<< "$(find "${1}/include" -type 'l')"
	
}

function rename_soname_libraries() {
	
	declare -A mapping=()
	
	while read file; do
		readelf -d "${file}" 1>/dev/null 2>/dev/null || continue
		
		directory="$(dirname "${file}")"
		
		source="$(basename "${file}")"
		destination="$(readelf -d "${file}" | grep 'SONAME' | sed --regexp-extended 's/.+\[(.+)\]/\1/g')"
		
		if [ -z "${destination}" ]; then
			continue
		fi
		
		if [ "${source}" = "${destination}" ]; then
			continue
		fi
		
		mapping["${source}"]="${destination}"
		
		a="${directory}/${source}"
		b="${directory}/${destination}"
		
		echo "- Renaming '${a}' to '${b}'"
		
		unlink "${b}"
		mv "${a}" "${b}"
	done <<< "$(find "${1}/lib" -maxdepth '1' -type 'f' -name '*.so*')"
	
	while read file; do
		directory="$(dirname "${file}")"
		
		name="$(basename "$(readlink "${file}")")"
		
		if [ -z "${name}" ]; then
			continue
		fi
		
		destination="${mapping["${name}"]:-}"
		
		if [ -z "${destination}" ]; then
			continue
		fi
		
		unlink "${file}"
		
		a="${directory}/${destination}"
		b="${file}"
		
		echo "- Symlinking '${a}' to '${b}'"
		
		ln --symbolic --relative --force "${a}" "${b}"
		
	done <<< "$(find "${1}/lib" -maxdepth '1' -type 'l')"
	
}
