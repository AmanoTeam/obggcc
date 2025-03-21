#!/bin/bash

declare extra_configure_flags='--with-arch=armv4t --with-float=soft'
declare extra_cxx_flags=''

declare triplet='arm-unknown-linux-gnueabi'

declare sysroot='https://github.com/AmanoTeam/debian-sysroot/releases/latest/download/arm-unknown-linux-gnueabi2.7.tar.xz'
