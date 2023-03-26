#!/bin/bash

declare packages=(
	'http://archive.debian.org/debian/pool/main/l/linux/linux-libc-dev_4.9.228-1_mips64el.deb'
	'http://archive.debian.org/debian/pool/main/g/glibc/libc6-dev_2.24-11+deb9u4_mips64el.deb'
	'http://archive.debian.org/debian/pool/main/g/glibc/libc6_2.24-11+deb9u4_mips64el.deb'
)

declare extra_configure_flags=''

declare triple='mips64el-unknown-linux-gnuabi64'
declare host='mips64el-linux-gnuabi64'

declare output_format='elf64-tradlittlemips'
declare ld='ld.so.1'

declare debian_release_major='9'
