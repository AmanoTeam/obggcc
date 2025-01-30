#!/bin/bash

declare extra_configure_flags=''

declare triple='x86_64-unknown-linux-gnu'
declare host='x86_64-linux-gnu'

declare output_format='elf64-x86-64'
declare ld='ld-linux-x86-64.so.2'

declare debian_release_major='5'

declare sysroot='https://github.com/AmanoTeam/debian-sysroot/releases/latest/download/x86_64-unknown-linux-gnu2.7.tar.xz'
