#!/bin/bash

declare packages=(
	'http://archive.debian.org/debian/pool/main/l/linux-2.6/linux-libc-dev_2.6.26-29_ia64.deb'
	'http://archive.debian.org/debian/pool/main/g/glibc/libc6.1-dev_2.7-18lenny7_ia64.deb'
	'http://archive.debian.org/debian/pool/main/g/glibc/libc6.1_2.7-18lenny7_ia64.deb'
)

declare extra_configure_flags='--disable-libsanitizer --enable-obsolete'

declare triple='ia64-unknown-linux-gnu'
declare host='ia64-linux-gnu'

declare output_format='elf64-ia64-little'
declare ld='ld-linux-ia64.so.2'

declare debian_release_major='5'
