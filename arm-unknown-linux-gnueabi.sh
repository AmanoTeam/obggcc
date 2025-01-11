#!/bin/bash

declare extra_configure_flags='--with-arch=armv4t --with-float=soft --disable-libsanitizer'

declare triple='arm-unknown-linux-gnueabi'
declare host='arm-linux-gnueabi'

declare output_format='elf32-littlearm'
declare ld='ld-linux.so.3'

declare debian_release_major='5'

declare sysroot='https://github.com/AmanoTeam/debian-sysroot/releases/latest/download/arm-unknown-linux-gnueabi2.7.tar.xz'
