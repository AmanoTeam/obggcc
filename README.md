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
| Debian 5 (Lenny)          | Glibc 2.7        | Linux 2.6.26       | 2009               |
| Debian 6 (Squeeze)        | Glibc 2.11       | Linux 2.6.32       | 2011               |
| Debian 7 (Wheezy)         | Glibc 2.13       | Linux 3.2.78       | 2013               |
| CentOS 7                          | Glibc 2.17       | Linux 3.10.0       | 2014               |
| Debian 8 (Jessie)         | Glibc 2.19       | Linux 3.16.56      | 2015               |
| Debian 9 (Stretch)        | Glibc 2.24       | Linux 4.9.228       | 2017               |
| Debian 10 (Buster)        | Glibc 2.28       | Linux 4.19.249      | 2019               |
| Debian 11 (Bullseye)      | Glibc 2.31       | Linux 5.10.223      | 2021               |

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

Cross-compiling CMake projects involves using a CMake toolchain file, which is a special file that defines the cross-compilation tools and also the system root CMake should use. Inside the `${OBGGCC_HOME}/usr/local/share/obggcc/cmake` directory, there is a custom CMake toolchain file for each cross-compilation target OBGGCC supports:

```bash
$ ls ${OBGGCC_HOME}/usr/local/share/obggcc/cmake
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
# Setup the environment for cross-compilation
$ cmake \
    -DCMAKE_TOOLCHAIN_FILE=${OBGGCC_HOME}/usr/local/share/obggcc/cmake/aarch64-unknown-linux-gnu2.19.cmake \
    -B build \
    -S  ./
# Build the project
$ cmake --build ./build
```

### GNU Autotools (aka GNU Build System)

Configuring an autotools project for cross-compilation usually requires the user to modify the environment variables and define the compilation tools the project should use. Inside the `${OBGGCC_HOME}/usr/local/share/obggcc/autotools` directory, there is a custom bash script for each cross-compilation target that OBGGCC supports:

```bash
$ ls ${OBGGCC_HOME}/usr/local/share/obggcc/autotools
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
$ source ${OBGGCC_HOME}/usr/local/share/obggcc/autotools/aarch64-unknown-linux-gnu2.19.sh

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

If your project depends on something other than the GNU C library (or the C++ standard libraries, for C++ programs), you should build it yourself.

## Linking with system libraries

By default, you can't mix headers and libraries from both the cross-compiler's system root and your host machine's system root. The main reason is that there may be incompatibilities between the two systems, which could cause the compilation to break or produce broken binaries even if the compilation succeeds. Due to this, the standard practice is that you also cross-compile any external dependency that your program may need to use and put them inside the cross-compiler's system root or explicitly point the compiler to where to find the cross-compiled dependencies by passing the appropriate flags while compiling your binary (`-I`, `-L`, and `-l`).

That being said, the GNU C library is portable enough to reduce these incompatibilities to some extent.

The environment variable `OBGGCC_WANTS_SYSTEM_LIBRARIES` can be used to control the default behavior when cross-compiling software.

Let's take this program as an example:

```c
#include <string.h>
#include <stdio.h>

#include <openssl/sha.h>

int main(void) {
    const char* s = "OBGGCC";
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(s, strlen(s), hash);
    
    puts("It works!");
    
    return 0;
}
```

It uses OpenSSL to perform a simple SHA-256 calculation. When compiling this program on my host machine, it builds fine:

```bash
$ gcc main.c -lcrypto -o main
$ ./main
It works!
```

However, it fails when cross-compiling with OBGGCC:

```bash
$ ${OBGGCC_HOME}/bin/x86_64-unknown-linux-gnu2.7-gcc main.c -lcrypto -o main
main.c:3:10: fatal error: openssl/sha.h: No such file or directory
    3 | #include <openssl/sha.h>
      |          ^~~~~~~~~~~~~~~
compilation terminated.
```

That's expected, as there are no prebuilt OpenSSL binaries inside the cross-compiler's system root.

Now let's try setting the `OBGGCC_WANTS_SYSTEM_LIBRARIES` environment variable:

```bash
$ export OBGGCC_WANTS_SYSTEM_LIBRARIES=1
```

And then compiling the program again:

```bash
$ ${OBGGCC_HOME}/bin/x86_64-unknown-linux-gnu2.7-gcc main.c -lcrypto -o main
$ ./main
It works!
```

The reason for the build to succeed this time is that `OBGGCC_WANTS_SYSTEM_LIBRARIES` changes the default cross-compilation behavior so that system directories of the host machine get included in both the library and header location search lists of the cross-compiler. Essentially, it:

- Adds `/usr/include ` to the list of include directories
- Adds `/usr/local/lib64`, `/usr/local/lib`, `/lib64`, `/lib`, `/usr/lib64`, and `/usr/lib` to the list of library directories

Note that, despite explicitly adding directories of the host machine to the compiler invocation, any program you compile will still use headers and libraries of the cross-compiler's GLIBC. That's due to the fact that the directories of the cross-compiler's system root are checked first before the directories of your host machine's system root. Only when the compiler can't find a specific header or library in the cross-compiler's system root does it search for it in your machine's system root. This way, we can build portable programs while still linking against third-party libraries from the host machine.

## Behavior changes

Usually, we attempt to keep the default GCC behavior unchanged. Sometimes we patch GCC here and there to make it work with older glibc versions or fix breakages after a toolchain/dependency update, but most of these changes don't affect the default behavior of GCC in any way. However, currently, there are some specific changes in place that modify the default behavior in some scenarios.

### Warnings no longer treated as errors

The warnings `-Wimplicit-function-declaration`, `-Wimplicit-int`, and `-Wint-conversion` are no longer treated as errors by default. Treating these warnings as errors intentionally breaks the compilation of old/legacy code and makes it difficult for people who just want to compile something without having to modify any code.

You can revert to the old behavior by passing `-Werror=implicit-function-declaration -Werror=implicit-int -Werror=int-conversion` to the compiler command invocation.

### The default language version for C compilation is still `gnu17` (aka C17)

Changing the default language version for C compilation from `-std=gnu17` to `-std=gnu23` was one of the most noticeable changes in GCC 15, not because of all the new features it comes with, but because of the compilation breakage it caused in some [GCC dependencies](https://github.com/compiler-explorer/compiler-explorer/issues/7360) and even GCC itself after the update.

It's something that essentially breaks old/legacy code. Since it causes more harm than good, we just reverted this change.

You can revert to the old behavior by passing `-std=gnu23` to the compiler command invocation.

## Releases

You can obtain OBGGCC releases from the [releases](https://github.com/AmanoTeam/obggcc/releases) page.