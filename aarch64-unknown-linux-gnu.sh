#!/bin/bash

declare packages=(
	'https://archive.debian.org/debian/pool/main/l/linux/linux-libc-dev_3.16.56-1+deb8u1_arm64.deb'
	'https://archive.debian.org/debian/pool/main/g/glibc/libc6-dev_2.19-18+deb8u10_arm64.deb'
	'https://archive.debian.org/debian/pool/main/g/glibc/libc6_2.19-18+deb8u10_arm64.deb'
	'https://archive.debian.org/debian/pool/main/s/systemtap/systemtap-sdt-dev_2.6-0.2_arm64.deb'
)

declare extra_configure_flags=''

declare triple='aarch64-unknown-linux-gnu'
declare host='aarch64-linux-gnu'

declare output_format='elf64-littleaarch64'
declare ld='ld-linux-aarch64.so.1'

declare debian_release_major='8'
