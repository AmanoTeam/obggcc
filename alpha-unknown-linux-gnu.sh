#!/bin/bash

declare extra_configure_flags=''

declare triple='alpha-unknown-linux-gnu'
declare host='alpha-linux-gnu'

declare output_format='elf64-alpha'
declare ld='ld-linux.so.2'

declare debian_release_major='5'

declare sysroot='https://github.com/AmanoTeam/debian-sysroot/releases/latest/download/alpha-unknown-linux-gnu2.7.tar.xz'
