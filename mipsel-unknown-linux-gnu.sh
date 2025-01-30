#!/bin/bash

declare extra_configure_flags=''

declare triple='mipsel-unknown-linux-gnu'
declare host='mipsel-linux-gnu'

declare output_format='elf32-tradlittlemips'
declare ld='ld.so.1'

declare debian_release_major='5'

declare sysroot='https://github.com/AmanoTeam/debian-sysroot/releases/latest/download/mipsel-unknown-linux-gnu2.7.tar.xz'
