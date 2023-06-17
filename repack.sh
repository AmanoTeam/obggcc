#!/bin/bash

set -eu

for directory in *; do
	tar --directory="${directory}" --create --file=- 'obggcc' |  xz --compress -9 > "${directory}.tar.xz"
	sha256sum "${directory}.tar.xz" > "${directory}.tar.xz.sha256"
	
	rm --recursive "${directory}"
done

exit 0
