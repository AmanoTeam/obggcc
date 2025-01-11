#!/bin/bash

declare extra_configure_flags=''

declare triple='aarch64-unknown-linux-gnu'
declare host='aarch64-linux-gnu'

declare output_format='elf64-littleaarch64'
declare ld='ld-linux-aarch64.so.1'

declare debian_release_major='8'

declare sysroot='https://github.com/AmanoTeam/debian-sysroot/releases/latest/download/aarch64-unknown-linux-gnu2.19.tar.xz'
