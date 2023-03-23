#!/bin/bash

declare packages=(
	'http://archive.debian.org/debian-security/pool/updates/main/l/linux/linux-libc-dev_3.2.102-1_i386.deb'
	'http://archive.debian.org/debian-security/pool/updates/main/e/eglibc/libc6-dev_2.13-38+deb7u12_i386.deb'
	'http://archive.debian.org/debian-security/pool/updates/main/e/eglibc/libc6_2.13-38+deb7u12_i386.deb'
	'http://archive.debian.org/debian/pool/main/s/systemtap/systemtap-sdt-dev_1.7-1+deb7u1_i386.deb'
)

declare extra_configure_flags='--with-arch=i586'

declare triple='i486-unknown-linux-gnu'
declare host='i386-linux-gnu'

declare output_format='elf32-i386'
declare ld='ld-linux.so.2'
