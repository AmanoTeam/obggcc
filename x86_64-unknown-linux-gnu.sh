#!/bin/bash

declare packages=(
	'http://archive.debian.org/debian-security/pool/updates/main/l/linux/linux-libc-dev_3.2.102-1_amd64.deb'
	'http://archive.debian.org/debian-security/pool/updates/main/e/eglibc/libc6-dev_2.13-38+deb7u12_amd64.deb'
	'http://archive.debian.org/debian-security/pool/updates/main/e/eglibc/libc6_2.13-38+deb7u12_amd64.deb'
	'http://archive.debian.org/debian/pool/main/s/systemtap/systemtap-sdt-dev_1.7-1+deb7u1_amd64.deb'
)

declare extra_configure_flags=''

declare triple='x86_64-unknown-linux-gnu'
declare host='x86_64-linux-gnu'

declare output_format='elf64-x86-64'
declare ld='ld-linux-x86-64.so.2'
