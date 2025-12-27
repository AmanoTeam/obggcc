#!/bin/bash

declare extra_configure_flags='--with-arch=armv6 --with-tune=arm1136j-s --with-float=soft'
declare triplet='armv6-unknown-linux-gnueabi'

declare sysroot='https://github.com/AmanoTeam/debian-sysroot/releases/latest/download/armv6-unknown-linux-gnueabi2.3.tar.xz'
