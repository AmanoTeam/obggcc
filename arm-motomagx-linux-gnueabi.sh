#!/bin/bash

declare extra_configure_flags='--with-arch=armv6 --with-tune=arm1136j-s --with-float=soft'
declare extra_cxx_flags=''

declare triplet='arm-motomagx-linux-gnueabi'

declare sysroot='https://github.com/AmanoTeam/debian-sysroot/releases/latest/download/arm-motomagx-linux-gnueabi2.3.tar.xz'
