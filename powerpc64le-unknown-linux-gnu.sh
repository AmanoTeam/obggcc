#!/bin/bash

declare extra_configure_flags=''

declare triple='powerpc64le-unknown-linux-gnu'
declare host='powerpc64le-linux-gnu'

declare output_format='elf64-powerpcle'
declare ld='ld64.so.2'

declare debian_release_major='8'

declare sysroot='https://github.com/AmanoTeam/debian-sysroot/releases/latest/download/powerpc64le-unknown-linux-gnu2.19.tar.xz'
