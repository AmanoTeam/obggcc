#!/bin/bash

declare packages=(
	'https://archive.debian.org/debian/pool/main/l/linux-2.6/linux-libc-dev_2.6.26-29_armel.deb'
	'https://archive.debian.org/debian/pool/main/g/glibc/libc6-dev_2.7-18lenny7_armel.deb'
	'https://archive.debian.org/debian/pool/main/g/glibc/libc6_2.7-18lenny7_armel.deb'
)

declare extra_configure_flags='--with-arch=armv4t --with-float=soft --disable-libsanitizer'

declare triple='arm-unknown-linux-gnueabi'
declare host='arm-linux-gnueabi'

declare output_format='elf32-littlearm'
declare ld='ld-linux.so.3'

declare debian_release_major='5'