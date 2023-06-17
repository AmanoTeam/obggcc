#!/bin/bash

for filename in *.tar.xz; do
	declare name="$(basename "${filename}" '.tar.xz')"
	
	[ -d "${name}" ] || mkdir "${name}"
	tar --directory="${name}" --extract --file="${filename}"
	
	rm "${filename}"
	rm "${filename}.sha256"
done

exit 0
