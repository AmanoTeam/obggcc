#!/bin/bash

declare packages=(
	'https://web.archive.org/web/0if_/https://archive.debian.org/debian/pool/main/l/linux-2.6/linux-libc-dev_2.6.26-29_i386.deb'
	'https://web.archive.org/web/0if_/https://archive.debian.org/debian/pool/main/g/glibc/libc6-dev_2.7-18lenny7_i386.deb'
	'https://web.archive.org/web/0if_/https://archive.debian.org/debian/pool/main/g/glibc/libc6_2.7-18lenny7_i386.deb'
)

declare extra_configure_flags='--with-arch=i586 --disable-libsanitizer'

declare triple='i386-unknown-linux-gnu'
declare host='i386-linux-gnu'

declare output_format='elf32-i386'
declare ld='ld-linux.so.2'

declare debian_release_major='5'
