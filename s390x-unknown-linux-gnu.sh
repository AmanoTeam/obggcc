#!/bin/bash

declare extra_configure_flags='--disable-libsanitizer'

declare triple='s390x-unknown-linux-gnu'
declare host='s390x-linux-gnu'

declare output_format='elf64-s390'
declare ld='ld64.so.1'

declare debian_release_major='7'

declare sysroot='https://github.com/AmanoTeam/debian-sysroot/releases/latest/download/s390x-unknown-linux-gnu2.13.tar.xz'
