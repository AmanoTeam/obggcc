#!/bin/bash

declare packages=(
	'http://archive.debian.org/debian-security/pool/updates/main/l/linux/linux-libc-dev_3.2.102-1_armhf.deb'
	'http://archive.debian.org/debian-security/pool/updates/main/e/eglibc/libc6-dev_2.13-38+deb7u12_armhf.deb'
	'http://archive.debian.org/debian-security/pool/updates/main/e/eglibc/libc6_2.13-38+deb7u12_armhf.deb'
	'http://archive.debian.org/debian/pool/main/s/systemtap/systemtap-sdt-dev_1.7-1+deb7u1_armhf.deb'
)

declare extra_configure_flags='--with-arch=armv7-a --with-fpu=vfpv3-d16 --with-float=hard --with-mode=thumb' 

declare triple='arm-unknown-linux-gnueabihf'
declare host='arm-linux-gnueabihf'

declare output_format='elf32-littlearm'
declare ld='ld-linux-armhf.so.3'
