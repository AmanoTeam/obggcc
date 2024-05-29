# OBGGCC

A GCC cross-compiler targeting very old glibc versions.

## How does it work?

I basically extracted the sysroot of some very old Linux distributions (e.g., Debian 5) and built a GCC cross-compiler against it. The result is a cross-compiler that is able to generate Linux binaries compatible with glibc versions ranging from 2.7 up to 2.19.

## Use cases

Initially, I just wanted the get rid of errors like this:

```
./main: /lib64/libc.so.6: version `GLIBC_2.33' not found (required by ./main)
./main: /lib64/libc.so.6: version `GLIBC_2.34' not found (required by ./main)
```

This error can occur when you build your binaries on a system with a glibc newer than the one installed on the target system. This is pretty annoying because it prevents users on older Linux distributions from using your software.

Since OBGGCC targets older glibc versions by default, these errors are (almost) unlikely to happen. You can distribute portable Linux binaries without worrying about them being incompatible with some older distribution.

OBGGCC can also be useful if you just want to test whether your program builds or runs on older systems (for portability purposes).

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
