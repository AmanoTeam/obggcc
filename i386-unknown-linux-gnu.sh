#!/bin/bash

declare extra_configure_flags='--with-arch=i586'

declare triple='i386-unknown-linux-gnu'
declare host='i386-linux-gnu'

declare output_format='elf32-i386'
declare ld='ld-linux.so.2'

declare debian_release_major='5'

declare sysroot='https://github.com/AmanoTeam/debian-sysroot/releases/latest/download/i386-unknown-linux-gnu2.7.tar.xz'
