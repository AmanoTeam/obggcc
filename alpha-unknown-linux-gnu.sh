#!/bin/bash

declare packages=(
	'https://web.archive.org/web/0if_/https://archive.debian.org/debian/pool/main/l/linux-2.6/linux-libc-dev_2.6.26-29_alpha.deb'
	'https://web.archive.org/web/0if_/https://archive.debian.org/debian/pool/main/g/glibc/libc6.1-dev_2.7-18lenny7_alpha.deb'
	'https://web.archive.org/web/0if_/https://archive.debian.org/debian/pool/main/g/glibc/libc6.1_2.7-18lenny7_alpha.deb'
)

declare extra_configure_flags=''

declare triple='alpha-unknown-linux-gnu'
declare host='alpha-linux-gnu'

declare output_format='elf64-alpha'
declare ld='ld-linux.so.2'

declare debian_release_major='5'
