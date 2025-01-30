#!/bin/bash

declare extra_configure_flags=''

declare triple='hppa-unknown-linux-gnu'
declare host='hppa-linux-gnu'

declare output_format='elf32-hppa-linux'
declare ld='ld.so.1'

declare debian_release_major='5'

declare sysroot='https://github.com/AmanoTeam/debian-sysroot/releases/latest/download/hppa-unknown-linux-gnu2.7.tar.xz'
