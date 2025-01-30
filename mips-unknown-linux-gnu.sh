#!/bin/bash

declare extra_configure_flags=''

declare triple='mips-unknown-linux-gnu'
declare host='mips-linux-gnu'

declare output_format='elf32-tradbigmips'
declare ld='ld.so.1'

declare debian_release_major='5'

declare sysroot='https://github.com/AmanoTeam/debian-sysroot/releases/latest/download/mips-unknown-linux-gnu2.7.tar.xz'
