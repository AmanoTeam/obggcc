# OBGGCC

A GCC cross-compiler targeting older glibc versions.

## How does it work?

I extracted the sysroot from almost all major legacy/obsolete Debian releases and built a GCC cross-compiler for each of them. This results in a GCC toolchain targeting software for a specific Debian version (and thus, a specific glibc version as well).

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

## Supported targets

Currently, OBGGCC provides cross-compilers targeting 7 major Debian releases and 1 major CentOS release on 16 system architectures.

### Distributions

| distribution_version     | glibc_version    | linux_version   | availability_date | 
| :-------------------------: | :----------------: | :----------------: | :------------------: |
| Debian 5 (Lenny)          | Glibc 2.7        | Linux 2.6       | 2009               |
| Debian 6 (Squeeze)        | Glibc 2.11       | Linux 2.6       | 2011               |
| Debian 7 (Wheezy)         | Glibc 2.13       | Linux 3.2       | 2013               |
| CentOS 7                          | Glibc 2.17       | Linux 3.10       | 2014               |
| Debian 8 (Jessie)         | Glibc 2.19       | Linux 3.16      | 2015               |
| Debian 9 (Stretch)        | Glibc 2.24       | Linux 4.9       | 2017               |
| Debian 10 (Buster)        | Glibc 2.28       | Linux 4.19      | 2019               |
| Debian 11 (Bullseye)      | Glibc 2.31       | Linux 5.10      | 2021               |

### System architectures

- `alpha-unknown-linux-gnu`
- `aarch64-unknown-linux-gnu`
- `arm-unknown-linux-gnueabi`
- `arm-unknown-linux-gnueabihf`
- `hppa-unknown-linux-gnu`
- `i386-unknown-linux-gnu`
- `mips64el-unknown-linux-gnuabi64`
- `mips-unknown-linux-gnu`
- `mipsel-unknown-linux-gnu`
- `powerpc64le-unknown-linux-gnu`
- `powerpc-unknown-linux-gnu`
- `s390-unknown-linux-gnu`
- `s390x-unknown-linux-gnu`
- `sparc-unknown-linux-gnu`
- `x86_64-unknown-linux-gnu`
- `ia64-unknown-linux-gnu`

## Cross-compilation

### CMake

Cross-compiling CMake projects involves using a CMake toolchain file, which is a special file that defines the cross-compilation tools and also the system root CMake should use. Inside the `usr/local/share/obggcc/cmake` directory, there is a custom CMake toolchain file for each cross-compilation target OBGGCC supports:

```bash
$ ls ./usr/local/share/obggcc/cmake
aarch64-unknown-linux-gnu2.19.cmake
aarch64-unknown-linux-gnu2.24.cmake
aarch64-unknown-linux-gnu2.28.cmake
aarch64-unknown-linux-gnu2.31.cmake
alpha-unknown-linux-gnu2.7.cmake
arm-unknown-linux-gnueabi2.11.cmake
arm-unknown-linux-gnueabi2.13.cmake
arm-unknown-linux-gnueabi2.19.cmake
arm-unknown-linux-gnueabi2.24.cmake
...
```

To use one of them, pass the `CMAKE_TOOLCHAIN_FILE` definition to your CMake command invocation:

```bash
cmake \
    -DCMAKE_TOOLCHAIN_FILE=../obggcc/usr/local/share/obggcc/cmake/aarch64-unknown-linux-gnu2.19.cmake \
    -B build \
    -S  ./
```

### GNU Autotools (aka GNU Build System)

Configuring an autotools project for cross-compilation usually requires the user to modify the environment variables and define the compilation tools the project should use. Inside the `usr/local/share/obggcc/autotools` directory, there is a custom bash script for each cross-compilation target that OBGGCC supports:

```bash
$ ls ./usr/local/share/obggcc/autotools
aarch64-unknown-linux-gnu2.19.sh
aarch64-unknown-linux-gnu2.24.sh
aarch64-unknown-linux-gnu2.28.sh
aarch64-unknown-linux-gnu2.31.sh
alpha-unknown-linux-gnu2.7.sh
arm-unknown-linux-gnueabi2.11.sh
arm-unknown-linux-gnueabi2.13.sh
arm-unknown-linux-gnueabi2.19.sh
arm-unknown-linux-gnueabi2.24.sh
...
```

They are meant to be `source`d by you whenever you want to cross-compile something:

```bash
# Setup the environment for cross-compilation
$ source ../obggcc/usr/local/share/obggcc/autotools/aarch64-unknown-linux-gnu2.19.sh

# Configure & build the project
$ ./configure --host="${CROSS_COMPILE_TRIPLET}"
$ make
```

## Will my program only run on Debian machines?!

No! Cross-compiling your software targeting a specific Debian version doesn't mean it will only run on Debian machines. In fact, since Debian uses the GNU C Library, it will run just fine on most Linux distributions.

## Portability with C++ programs

Unlike C programs, you cannot easily distribute C++ binaries in a portable way without also shipping the libstdc++ library (and sometimes, libgcc too) along with your release binary. Usually, shipping the libstdc++ library with your release binary doesn't offer much benefit in terms of libc version portability, as your program would still depend on the same libc version that libstdc++ was linked against (in this case, the one installed on your system).

When building C++ programs with OBGGCC, however, your program automatically links against our variant of libstdc++, which is compiled against the same old libc version that the toolchain you're using provides. Because of this, you can statically link it with your binary (or ship the shared library with your release binary) without worrying about it increasing the libc version requirement.

## Software availability

Note that all the cross-compilers only contain the minimum required to build a working C/C++ program. That is, you won't find any prebuilt binaries of popular projects like OpenSSL or zlib available for use, as you would on an average Linux distribution.

If your project depends on something other than the core GNU C libraries (or the stdc++ libraries, for C++ programs), you should build it yourself.

## Releases

* [GCC 14.x](https://github.com/AmanoTeam/obggcc/releases/tag/1.1)
* [GCC 15.x](https://github.com/AmanoTeam/obggcc/releases/latest)
