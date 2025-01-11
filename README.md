# OBGGCC

A GCC cross-compiler targeting older glibc versions.

## How does it work?

I extracted the sysroot from almost all major Debian releases and built a GCC cross-compiler for each of them. This results in a GCC toolchain targeting software for a specific Debian version (and thus, a specific glibc version as well).

This eliminates the need to install an ancient Linux distribution in Docker/LXC just to build portable binaries, which is the current standard practice.

## Use cases

Initially, I just wanted the get rid of errors like this:

```
./main: /lib64/libc.so.6: version `GLIBC_2.33' not found (required by ./main)
./main: /lib64/libc.so.6: version `GLIBC_2.34' not found (required by ./main)
```

This error can occur when you build your binaries on a system with a glibc newer than the one installed on the target system. This is pretty annoying because it prevents users on older Linux distributions from using your software.

Since OBGGCC targets older glibc versions by default, these errors are (almost) unlikely to happen. You can distribute portable Linux binaries without worrying about them being incompatible with some older distribution.

OBGGCC can also be useful if you just want to test whether your program builds or runs on older systems.

## Portability with C++ programs

Unlike C programs, you cannot easily distribute C++ binaries in a portable way without also shipping the libstdc++ library (and sometimes, libgcc too) along with your release binary. Usually, shipping the libstdc++ library with your release binary doesn't offer much benefit in terms of libc version portability, as your program would still depend on the same libc version that libstdc++ was linked against (in this case, the one installed on your system).

When building C++ programs with OBGGCC, however, your program automatically links against our variant of libstdc++, which is compiled against the same old libc version that the toolchain you're using provides. Because of this, you can statically link it with your binary (or ship the shared library with your release binary) without worrying about it increasing the libc version requirement.

## Will my program only run on Debian machines?!

No! Cross-compiling your software targeting a specific Debian version doesn't mean it will only run on Debian machines. In fact, since Debian uses the GNU C Library, it will run just fine on most Linux distributions.

## Target architectures

We currently build cross-compilers targeting many Linux architectures. See above for more info:

|         target         |  distribution_version  |     glibc_version     |     linux_version     | 
| ---------------------- | ------------------------------- | -------------------------- | ------------------------- |
| `alpha-unknown-linux-gnu` | Debian 5 | Glibc 2.7 | Linux 2.6.26 |
| `aarch64-unknown-linux-gnu` | Debian 8 | Glibc 2.19 | Linux 3.16.56 |
| `arm-unknown-linux-gnueabi` | Debian 5 | Glibc 2.7 | Linux 2.6.26 |
| `arm-unknown-linux-gnueabihf` | Debian 7 | Glibc 2.13 | Linux 3.2.102 |
| `hppa-unknown-linux-gnu` | Debian 5 | Glibc 2.7 | Linux 2.6.26 |
| `i386-unknown-linux-gnu` | Debian 5 | Glibc 2.7 | Linux 2.6.26 |
| `mips64el-unknown-linux-gnuabi64` | Debian 9 | Glibc 2.24 | Linux 4.9.228 |
| `mips-unknown-linux-gnu` | Debian 5 | Glibc 2.7 | Linux 2.6.26 |
| `mipsel-unknown-linux-gnu` | Debian 5 | Glibc 2.7 | Linux 2.6.26 |
| `powerpc64le-unknown-linux-gnu` | Debian 8 | Glibc 2.19 | Linux 3.16.56 |
| `powerpc-unknown-linux-gnu` | Debian 5 | Glibc 2.7 | Linux 2.6.26 |
| `s390-unknown-linux-gnu` | Debian 5 | Glibc 2.7 | Linux 2.6.26 |
| `s390x-unknown-linux-gnu` | Debian 7 | Glibc 2.13 | Linux 3.2.78 |
| `sparc-unknown-linux-gnu` | Debian 5 | Glibc 2.7 | Linux 2.6.26 |
| `x86_64-unknown-linux-gnu` | Debian 5 | Glibc 2.7 | Linux 2.6.26 |
| `ia64-unknown-linux-gnu` | Debian 5 | Glibc 2.7 | Linux 2.6.26 |

## Releases

You can obtain OBGGCC releases from the  [releases](https://github.com/AmanoTeam/obggcc/releases) page.
