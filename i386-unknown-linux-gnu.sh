#!/bin/bash

declare extra_configure_flags='--with-arch=i586 --enable-cxx-flags="-D_GLIBCXX_HAVE_FLOORF -D_GLIBCXX_HAVE_CEILF -D_GLIBCXX_HAVE_HYPOTF -D_GLIBCXX_HAVE_HYPOTL -D_GLIBCXX_HAVE_FLOORL -D_GLIBCXX_HAVE_CEILL"'

declare triplet='i386-unknown-linux-gnu'

declare sysroot='https://github.com/AmanoTeam/debian-sysroot/releases/latest/download/i386-unknown-linux-gnu2.3.tar.xz'
