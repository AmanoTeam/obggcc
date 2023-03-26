#!/bin/bash

declare packages=(
	'http://archive.debian.org/debian/pool/main/l/linux-2.6/linux-libc-dev_2.6.26-29_sparc.deb'
	'http://archive.debian.org/debian/pool/main/g/glibc/libc6-dev_2.7-18lenny7_sparc.deb'
	'http://archive.debian.org/debian/pool/main/g/glibc/libc6_2.7-18lenny7_sparc.deb'
)

declare extra_configure_flags=''

declare triple='sparc-unknown-linux-gnu'
declare host='sparc-linux-gnu'

declare output_format='elf32-sparc'
declare ld='ld-linux.so.2'

declare debian_release_major='5'
