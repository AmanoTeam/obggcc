#!/bin/bash

declare extra_configure_flags='--with-arch=armv7-a --with-fpu=vfpv3-d16 --with-float=hard --with-mode=thumb' 

declare triple='arm-unknown-linux-gnueabihf'
declare host='arm-linux-gnueabihf'

declare output_format='elf32-littlearm'
declare ld='ld-linux-armhf.so.3'

declare debian_release_major='7'

declare sysroot='https://github.com/AmanoTeam/debian-sysroot/releases/latest/download/arm-unknown-linux-gnueabihf2.13.tar.xz'
