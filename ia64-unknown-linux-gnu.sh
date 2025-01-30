#!/bin/bash

declare extra_configure_flags='--enable-obsolete'

declare triple='ia64-unknown-linux-gnu'
declare host='ia64-linux-gnu'

declare output_format='elf64-ia64-little'
declare ld='ld-linux-ia64.so.2'

declare debian_release_major='5'

declare sysroot='https://github.com/AmanoTeam/debian-sysroot/releases/latest/download/ia64-unknown-linux-gnu2.7.tar.xz'
