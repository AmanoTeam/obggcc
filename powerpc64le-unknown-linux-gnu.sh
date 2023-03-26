#!/bin/bash

declare packages=(
	'http://archive.debian.org/debian/pool/main/l/linux/linux-libc-dev_3.16.56-1+deb8u1_ppc64el.deb'
	'http://archive.debian.org/debian/pool/main/g/glibc/libc6-dev_2.19-18+deb8u10_ppc64el.deb'
	'http://archive.debian.org/debian/pool/main/g/glibc/libc6_2.19-18+deb8u10_ppc64el.deb'
)

declare extra_configure_flags=''

declare triple='powerpc64le-unknown-linux-gnu'
declare host='powerpc64le-linux-gnu'

declare output_format='elf64-powerpcle'
declare ld='ld64.so.2'

declare debian_release_major='8'
