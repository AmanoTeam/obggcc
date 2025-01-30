#!/bin/bash

declare extra_configure_flags=''

declare triple='s390-unknown-linux-gnu'
declare host='s390-linux-gnu'

declare output_format='elf32-s390'
declare ld='ld.so.1'

declare debian_release_major='5'

declare sysroot='https://github.com/AmanoTeam/debian-sysroot/releases/latest/download/s390-unknown-linux-gnu2.7.tar.xz'
