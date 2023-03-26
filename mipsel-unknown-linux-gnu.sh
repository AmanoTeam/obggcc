#!/bin/bash

declare packages=(
	'http://archive.debian.org/debian/pool/main/l/linux-2.6/linux-libc-dev_2.6.26-29_mipsel.deb'
	'http://archive.debian.org/debian/pool/main/g/glibc/libc6-dev_2.7-18lenny7_mipsel.deb'
	'http://archive.debian.org/debian/pool/main/g/glibc/libc6_2.7-18lenny7_mipsel.deb'
)

declare extra_configure_flags=''

declare triple='mipsel-unknown-linux-gnu'
declare host='mipsel-linux-gnu'

declare output_format='elf32-tradlittlemips'
declare ld='ld.so.1'

declare debian_release_major='5'
