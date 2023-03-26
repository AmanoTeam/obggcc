#!/bin/bash

declare packages=(
	'http://archive.debian.org/debian/pool/main/l/linux/linux-libc-dev_3.2.78-1_s390x.deb'
	'http://archive.debian.org/debian/pool/main/e/eglibc/libc6-dev_2.13-38+deb7u10_s390x.deb'
	'http://archive.debian.org/debian/pool/main/e/eglibc/libc6_2.13-38+deb7u10_s390x.deb'
)

declare extra_configure_flags='--disable-libsanitizer'

declare triple='s390x-unknown-linux-gnu'
declare host='s390x-linux-gnu'

declare output_format='elf64-s390'
declare ld='ld64.so.1'

declare debian_release_major='7'
