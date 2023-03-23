#!/bin/bash

declare packages=(
	'http://archive.debian.org/debian-security/pool/updates/main/l/linux/linux-libc-dev_3.2.102-1_armel.deb'
	'http://archive.debian.org/debian-security/pool/updates/main/e/eglibc/libc6-dev_2.13-38+deb7u12_armel.deb'
	'http://archive.debian.org/debian-security/pool/updates/main/e/eglibc/libc6_2.13-38+deb7u12_armel.deb'
	'http://archive.debian.org/debian/pool/main/s/systemtap/systemtap-sdt-dev_1.7-1+deb7u1_armel.deb'
)

declare extra_configure_flags='--with-arch=armv4t --with-float=soft'

declare triple='arm-unknown-linux-gnueabi'
declare host='arm-linux-gnueabi'

declare output_format='elf32-littlearm'
declare ld='ld-linux.so.3'
