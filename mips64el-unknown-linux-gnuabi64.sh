#!/bin/bash

declare extra_configure_flags=''

declare triple='mips64el-unknown-linux-gnuabi64'
declare host='mips64el-linux-gnuabi64'

declare output_format='elf64-tradlittlemips'
declare ld='ld.so.1'

declare debian_release_major='9'

declare sysroot='https://github.com/AmanoTeam/debian-sysroot/releases/latest/download/mips64el-unknown-linux-gnuabi642.24.tar.xz'
