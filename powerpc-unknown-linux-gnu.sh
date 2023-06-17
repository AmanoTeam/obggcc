#!/bin/bash

declare packages=(
	'http://archive.debian.org/debian/pool/main/l/linux-2.6/linux-libc-dev_2.6.26-29_powerpc.deb'
	'http://archive.debian.org/debian/pool/main/g/glibc/libc6-dev_2.7-18lenny7_powerpc.deb'
	'http://archive.debian.org/debian/pool/main/g/glibc/libc6_2.7-18lenny7_powerpc.deb'
)

declare extra_configure_flags='--disable-libsanitizer'

declare triple='powerpc-unknown-linux-gnu'
declare host='powerpc-linux-gnu'

declare output_format='elf32-powerpc'
declare ld='ld.so.1'

declare debian_release_major='5'
