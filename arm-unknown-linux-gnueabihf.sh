#!/bin/bash

declare extra_configure_flags='--with-arch=armv7-a --with-fpu=vfpv3-d16 --with-float=hard --with-mode=thumb' 
declare triplet='arm-unknown-linux-gnueabihf'

declare sysroot='https://github.com/AmanoTeam/debian-sysroot/releases/latest/download/arm-unknown-linux-gnueabihf2.13.tar.xz'
