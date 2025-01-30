#!/bin/bash

declare extra_configure_flags=''

declare triple='powerpc-unknown-linux-gnu'
declare host='powerpc-linux-gnu'

declare output_format='elf32-powerpc'
declare ld='ld.so.1'

declare debian_release_major='5'

declare sysroot='https://github.com/AmanoTeam/debian-sysroot/releases/latest/download/powerpc-unknown-linux-gnu2.7.tar.xz'
