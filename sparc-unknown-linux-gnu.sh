#!/bin/bash

declare extra_configure_flags=''

declare triple='sparc-unknown-linux-gnu'
declare host='sparc-linux-gnu'

declare output_format='elf32-sparc'
declare ld='ld-linux.so.2'

declare debian_release_major='5'

declare sysroot='https://github.com/AmanoTeam/debian-sysroot/releases/latest/download/sparc-unknown-linux-gnu2.7.tar.xz'
