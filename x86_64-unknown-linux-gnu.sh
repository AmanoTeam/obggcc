#!/bin/bash

declare packages=(
	'http://archive.debian.org/debian/pool/main/l/linux-2.6/linux-libc-dev_2.6.26-29_amd64.deb'
	'http://archive.debian.org/debian/pool/main/g/glibc/libc6-dev_2.7-18lenny7_amd64.deb'
	'http://archive.debian.org/debian/pool/main/g/glibc/libc6_2.7-18lenny7_amd64.deb'
)

declare extra_configure_flags='--disable-libsanitizer'

declare triple='x86_64-unknown-linux-gnu'
declare host='x86_64-linux-gnu'

declare output_format='elf64-x86-64'
declare ld='ld-linux-x86-64.so.2'

declare debian_release_major='5'
