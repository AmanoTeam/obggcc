# OBGGCC

A GCC cross-compiler targeting older glibc versions.

## How does it work?

I extracted the sysroot from almost all major legacy/obsolete Debian releases and built a GCC cross-compiler for each of them. This results in a GCC toolchain targeting software for a specific Debian version (and thus, a specific glibc version as well).

This eliminates the need to install an ancient Linux distribution in Docker/LXC just to build portable binaries, which is the current standard practice.

## Use cases

Initially, I just wanted to get rid of errors like this:

```
./main: /lib64/libc.so.6: version `GLIBC_2.33' not found (required by ./main)
./main: /lib64/libc.so.6: version `GLIBC_2.34' not found (required by ./main)
```

This error can occur when you build your binaries on a system with a glibc version newer than the one installed on the target system. This is pretty annoying because it prevents users on older Linux distributions from using your software.

Since OBGGCC targets older glibc versions by default, these errors are (almost) unlikely to happen. You can distribute portable Linux binaries without worrying about them being incompatible with some older distributions.

OBGGCC can also be useful if you just want to test whether your program builds or runs on older systems.

## Supported targets

Currently, OBGGCC provides cross-compilers targeting 6 major Ubuntu releases, 8 major Debian releases and 1 major CentOS release on 16 system architectures.

### Distributions

| distribution_version     | glibc_version    | linux_version   | availability_date | 
| :-------------------------: | :----------------: | :----------------: | :------------------: |
| Debian 4 (Etch)          | glibc 2.3.6        | Linux 2.6.18       | 2007               |
| Debian 5 (Lenny)          | glibc 2.7        | Linux 2.6.26       | 2009               |
| Debian 6 (Squeeze)        | glibc 2.11       | Linux 2.6.32       | 2011               |
| Debian 7 (Wheezy)         | glibc 2.13       | Linux 3.2.78       | 2013               |
| Ubuntu 12.04 (Precise Pangolin)      | glibc 2.15       | Linux 3.2.0      | 2012               |
| CentOS 7                          | glibc 2.17       | Linux 3.10.0       | 2014               |
| Debian 8 (Jessie)         | glibc 2.19       | Linux 3.16.56      | 2015               |
| Ubuntu 16.04 (Xenial Xerus)      | glibc 2.23       | Linux 4.4.0      | 2016               |
| Debian 9 (Stretch)        | glibc 2.24       | Linux 4.9.228       | 2017               |
| Ubuntu 18.04 (Bionic Beaver)      | glibc 2.27       | Linux 4.15.0      | 2018               |
| Debian 10 (Buster)        | glibc 2.28       | Linux 4.19.249      | 2019               |
| Debian 11 (Bullseye)      | glibc 2.31       | Linux 5.10.223      | 2021               |
| Ubuntu 22.04 (Jammy Jellyfish)      | glibc 2.35       | Linux 5.15.0      | 2022               |
| Ubuntu 24.04 (Noble Numbat)      | glibc 2.39       | Linux 6.8.0      | 2024               |

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

## Portability with C++ programs

Unlike C programs, you cannot easily distribute C++ binaries in a portable way without also shipping the libstdc++ library (and sometimes, libgcc too) along with your release binary. Usually, shipping the libstdc++ library with your release binary doesn't offer much benefit in terms of libc version portability, as your program would still depend on the same libc version that libstdc++ was linked against (in this case, the one installed on your system).

When building C++ programs with OBGGCC, however, your program automatically links against our variant of libstdc++, which is compiled against the same old libc version that the toolchain you're using provides. Because of this, you can statically link it with your binary (or ship the shared library with your release binary) without worrying about it increasing the libc version requirement.

## Security and stability implications

Some people might think that linking a program against an old glibc version will make the compiled binary less secure and more vulnerable, as supposedly, the program will be using symbols from a standard library that's unmaintained and no longer receives security fixes. However, that's not true. Binaries compiled against an old glibc version still benefit from security fixes introduced in newer glibc versions as long as the target machine is running an updated glibc.

The whole point of symbol versioning is to prevent behavior inconsistencies when running binaries compiled against different glibc versions. This is accomplished by bumping the symbol version every time a backward-incompatible change is introduced to some public function or API of the standard library. With this, programs compiled against newer versions of the standard library can benefit from newer features, while old programs will continue working as intended, as they will still be using the old version of that specific function or API.

The only exception to when a "backward-incompatible change" is not considered for a symbol version bump is when it modifies undocumented or undefined behavior. Security fixes are not considered for a version bump, as they essentially fix something that was not intended to work that way — undefined behavior.

Changes introduced in a newer glibc version that are not considered for a symbol version bump take effect in all versions of that symbol, even in programs that were compiled for a glibc version that didn't receive that change at all. That means your program will still be running secure and optimized code as long as it's running on an up-to-date system.

> [!NOTE]  
> It should be noted that "an up-to-date system" does not specifically refer to a system where all packages — including glibc — are updated to their latest versions, but to a system that, at the bare minimum, receives security updates even if the system itself or its packages don't receive a major upgrade. This is especially true for Long-Term Support (LTS) Linux distributions.

## Can we go even further?

Currently, the minimum supported glibc version for cross-compilation in OBGGCC is glibc 2.3.6. This version is over 20 years old, and while you might be wondering why anyone would be interested in building software for such ancient versions, you might also be curious about whether it is possible to go even further and cross-compile software for glibc 2.2/2.1/2.0 or even glibc 1.x.

### My findings

glibc 1.x and glibc 2.x are not backward compatible in any way, meaning that software built for one cannot run on the other. glibc 1.x also received a lot of criticism in the past for failing to fully comply with the POSIX standard at the time it was still developed. I have not tried to build a cross-compiler for it, but I find it very unlikely that any recent GCC version still has support for such versions. At a bare minimum, heavy patching of the toolchain would be needed to make it work. It's not really worth it.

glibc 2.0 and glibc 2.1 were not binary-compatible on some architectures, as stated in the [release notes](https://sourceware.org/legacy-ml/libc-alpha/1999-q1/msg00310.html) of the said version. Also, symbol versioning did not exist in glibc until the 2.1 release. I'm not sure if software built for glibc 2.0 can run on glibc 2.1 and up.

Support for x86_64 (amd64) first appeared in glibc 2.2.5, only gaining overall stability in glibc 2.3 and onwards. Even if we manage to build a working cross-compiler targeting pre-glibc 2.2 releases or older, we would only be able to target very old (and potentially no longer used) system architectures.

Also, starting from glibc 2.2 and lower, `libstdc++` fails to build due to the absence of some required functions in the standard library. It might work with some patching, but I did not bother trying.

So, with that in mind, glibc 2.3 seems to be the minimum version that GCC is able to produce a cross-compiler for without breaking anything.

## Software availability

Note that all the cross-compilers only contain the minimum required to build a working C/C++ program. That is, you won't find any prebuilt binaries of popular projects like OpenSSL or zlib available for use, as you would on an average Linux distribution.

If your project depends on something other than the GNU C library (or the C++ standard libraries, for C++ programs), you should build it yourself.

## Linking with system libraries

By default, you can't mix headers and libraries from both the cross-compiler's system root and your host machine's system root. The main reason is that there may be incompatibilities between the two systems, which could cause the compilation to break or produce broken binaries even if the compilation succeeds. Due to this, the standard practice is that you also cross-compile any external dependency that your program may need to use and put them inside the cross-compiler's system root or explicitly point the compiler to where to find the cross-compiled dependencies by passing the appropriate flags when compiling your binary (`-I`, `-L`, and `-l`).

That being said, the GNU C library is portable enough to reduce these incompatibilities to some extent.

The environment variable `OBGGCC_SYSTEM_LIBRARIES` can be used to control the default behavior when cross-compiling software.

Let's take as an example this program which uses OpenSSL to perform a simple SHA-256 calculation:

```c
#include <string.h>
#include <stdio.h>

#include <openssl/sha.h>

int main(void) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256("OBGGCC", 6, hash);
    
    puts("It works!");
    
    return 0;
}
```

When compiling this program on my host machine, it builds fine:

```bash
$ gcc main.c -lcrypto -o main
$ ./main
It works!
```

However, it fails when cross-compiling with OBGGCC:

```bash
$ x86_64-unknown-linux-gnu2.3-gcc main.c -lcrypto -o main
main.c:3:10: fatal error: openssl/sha.h: No such file or directory
    3 | #include <openssl/sha.h>
      |          ^~~~~~~~~~~~~~~
compilation terminated.
```

That's expected, as there are no prebuilt OpenSSL binaries inside the cross-compiler's system root.

Now let's try setting the `OBGGCC_SYSTEM_LIBRARIES` environment variable:

```bash
$ export OBGGCC_SYSTEM_LIBRARIES=1
```

And then compiling the program again:

```bash
$ x86_64-unknown-linux-gnu2.3-gcc main.c -lcrypto -o main
$ ./main
It works!
```

The reason for the build to succeed this time is that `OBGGCC_SYSTEM_LIBRARIES` changes the default cross-compilation behavior so that system directories of the host machine get included in both the library and header location search lists of the cross-compiler.

Essentially, it:

- Adds `/usr/include ` to the list of include directories
- Adds `/usr/local/lib64`, `/usr/local/lib`, `/lib64`, `/lib`, `/usr/lib64`, and `/usr/lib` to the list of library directories

Note that, despite explicitly adding directories of the host machine to the compiler invocation, any program you compile will still use headers and libraries of the cross-compiler's GLIBC. That's due to the fact that the directories of the cross-compiler's system root are checked first before the directories of your host machine's system root. Only when the compiler can't find a specific header or library in the cross-compiler's system root does it search for it in your machine's system root. This way, we can build portable programs while still linking against third-party libraries from the host machine.

Keep in mind that `OBGGCC_SYSTEM_LIBRARIES` is only useful when you are cross-compiling software to a target that matches the one of your host machine (e.g., your host machine is an `x86_64-linux-gnu` system, and you are cross-compiling software targeting an `x86_64-linux-gnu` system as well), as you can't link binaries with mismatching ABIs:

```bash
$ gcc -dumpmachine
x86_64-unknown-linux-gnu
$ aarch64-unknown-linux-gnu2.19-gcc main.c -lcrypto -o main
/home/runner/obggcc/bin/../lib/gcc/aarch64-unknown-linux-gnu/15/../../../../aarch64-unknown-linux-gnu/bin/ld: skipping incompatible /lib64/libcrypto.so when searching for -lcrypto
/home/runner/obggcc/bin/../lib/gcc/aarch64-unknown-linux-gnu/15/../../../../aarch64-unknown-linux-gnu/bin/ld: skipping incompatible /usr/lib64/libcrypto.so when searching for -lcrypto
/home/runner/obggcc/bin/../lib/gcc/aarch64-unknown-linux-gnu/15/../../../../aarch64-unknown-linux-gnu/bin/ld: cannot find -lcrypto: No such file or directory
/home/runner/obggcc/bin/../lib/gcc/aarch64-unknown-linux-gnu/15/../../../../aarch64-unknown-linux-gnu/bin/ld: skipping incompatible /lib64/libcrypto.so when searching for -lcrypto
/home/runner/obggcc/bin/../lib/gcc/aarch64-unknown-linux-gnu/15/../../../../aarch64-unknown-linux-gnu/bin/ld: skipping incompatible /usr/lib64/libcrypto.so when searching for -lcrypto
collect2: error: ld returned 1 exit status
```

This also works if your system has multilib support (i.e., both 32-bit and 64-bit libraries coexist on the same system) and you are cross-compiling software targeting the 32-bit version of your system:

```bash
$ dpkg --print-architecture
amd64
$ dpkg --print-foreign-architectures
i386
$ i386-unknown-linux-gnu2.3-gcc main.c -lcrypto -o main
$ ./main
It works!
```

## Running binaries with a specific glibc

There may be cases where you might want to run your software under a specific glibc version, either to check how it will behave or to try a new glibc feature that was added in a later version but is not available in the glibc installed on your system.

The environment variable `OBGGCC_BUILTIN_LOADER` can be used to change the default loader (aka dynamic linker) of an executable when cross-compiling. In other words, you can use this to force your binary to use a different glibc at runtime, ignoring the one available in your system.

For some background, here is my current system:

```bash
$ cat /etc/os-release
NAME="CentOS Linux"
VERSION="7 (Core)"
ID="centos"
ID_LIKE="rhel fedora"
VERSION_ID="7"
PRETTY_NAME="CentOS Linux 7 (Core)"
ANSI_COLOR="0;31"
CPE_NAME="cpe:/o:centos:centos:7"
HOME_URL="https://www.centos.org/"
BUG_REPORT_URL="https://bugs.centos.org/"

CENTOS_MANTISBT_PROJECT="CentOS-7"
CENTOS_MANTISBT_PROJECT_VERSION="7"
REDHAT_SUPPORT_PRODUCT="centos"
REDHAT_SUPPORT_PRODUCT_VERSION="7"

$ ldd --version
ldd (GNU libc) 2.17
Copyright (C) 2012 Free Software Foundation, Inc.
This is free software; see the source for copying condit
ions.There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A 
PARTICULAR PURPOSE.
Written by Roland McGrath and Ulrich Drepper.
```

I'm running CentOS 7 with glibc 2.17. Let's suppose I want to try the new `getrandom()` routine that was introduced in glibc 2.25. I would then write something like this:

```c
#include <stdio.h>

#include <sys/random.h>

int main(void) {
    
    size_t index = 0;
    
    unsigned char buffer[16];
    getrandom(buffer, sizeof(buffer), 0);
    
    for (index = 0; index < sizeof(buffer); index++) {
        if (index != 0) {
            printf(" ");
        }
        
        printf("0x%X", buffer[index]);
    }
    
    printf("\nIt works!\n");
    
}
```

I can try compiling it on my host machine, but that will just fail:

```bash
$ gcc main.c
main.c:3:10: fatal error: sys/random.h: No such file or directory
    3 | #include <sys/random.h>
      |          ^~~~~~~~~~~~~~
compilation terminated.
```

There is obviously no `sys/random.h` in glibc 2.17. I can try cross-compiling my code to glibc 2.25+, but then I would not be able to run it on my machine due to the older glibc it has:

```
$ x86_64-unknown-linux-gnu2.27-gcc main.c -o main
$ ./main
./main: /lib64/libc.so.6: version `GLIBC_2.25' not found (required by ./main)
```

However, things change when I use `OBGGCC_BUILTIN_LOADER`:

```bash
$ export OBGGCC_BUILTIN_LOADER=1
$ x86_64-unknown-linux-gnu2.27-gcc main.c -o main
$ ./main
0x67 0xE4 0xD3 0x9B 0xBD 0xD2 0x59 0x86 0xC0 0xE7 0x79 0xD2 0x2 0x92 0x3C 0x85
It works!
```

This worked this time because `OBGGCC_BUILTIN_LOADER` changed the default loader for the executable `main` so that at runtime it uses the glibc bundled within OBGGCC instead of relying on the glibc library installed on my system.

We can check this by inspecting the executable with `readelf`:

```bash
$ readelf -l main | grep "interpreter:"
    [Requesting program interpreter: /home/runner/obggcc/x86_64-unknown-linux-gnu2.27/lib/ld-linux-x86-64.so.2]
$ readelf -d main | grep "RPATH"
    Library rpath: [/home/runner/obggcc/x86_64-unknown-linux-gnu2.27/lib]
```

## Debugging

### AddressSanitizer

AddressSanitizer is available for almost all cross-compilation targets, so you can use it with the usual `-fsanitize=...` flags if needed:

```bash
$ cat << asan > main.c
#include <stddef.h>

int main() {
	char s =  *((char*) NULL);
}
asan
$ x86_64-unknown-linux-gnu2.3-gcc -fsanitize=address main.c -o main -O0 -g
$ ./main
AddressSanitizer:DEADLYSIGNAL
=================================================================
==19765==ERROR: AddressSanitizer: SEGV on unknown address 0x000000000000 (pc 0x561e38623255 bp 0x7ffefb9bc660 sp 0x7ffefb9bc650 T0)
==19765==The signal is caused by a READ memory access.
==19765==Hint: address points to the zero page.
    #0 0x561e38623255 in main /home/runner/main.c:4
    #1 0x7fee199a7554 in __libc_start_main (/lib64/libc.so.6+0x22554) (BuildId: 32c239e375f673bcca264b717e7ac4b7bd5abadc)
    #2 0x561e38623099  (/home/runner/x+0x1099) (BuildId: 697cbcafa9e8da9eed79035cfb8755db7456b763)

==19765==Register values:
rax = 0x0000000000000000  rbx = 0x0000000000000000  rcx = 0x0000000000000000  rdx = 0x0000000000000000  
rdi = 0x0000000000000000  rsi = 0x00007ffefb9bc700  rbp = 0x00007ffefb9bc660  rsp = 0x00007ffefb9bc650  
 r8 = 0x00007fee19d4de80   r9 = 0x0000000000000000  r10 = 0x00007ffefb9bc0a0  r11 = 0x00007fee19e773d0  
r12 = 0x0000561e38623070  r13 = 0x00007ffefb9bc740  r14 = 0x0000000000000000  r15 = 0x0000000000000000  
AddressSanitizer can not provide additional info.
SUMMARY: AddressSanitizer: SEGV /home/runner/main.c:4 in main
==19765==ABORTING
```

If you are playing around with AddressSanitizer, you might want to set the `OBGGCC_RUNTIME_RPATH` environment variable:

```bash
export OBGGCC_RUNTIME_RPATH=1
```

This tells the linker to automatically add the `RPATH` of the directory containing the AddressSanitizer libraries to your executable, so you don't have to bother with setting the `LD_LIBRARY_PATH` or adding the rpath manually:

```bash
$ x86_64-unknown-linux-gnu2.3-gcc -fsanitize=address main.c -o main
$ ./main
./main: error while loading shared libraries: libasan.so.8: cannot open shared object file: No such file or directory
$ export OBGGCC_RUNTIME_RPATH=1
$ x86_64-unknown-linux-gnu2.3-gcc -fsanitize=address main.c -o main
$ ./main
<it works>
```

## GDB (GNU Debugger)

There are also bundled binaries of GDB available for use:

```
$ ls ${OBGGCC_HOME}/bin/*-gdb
obggcc/bin/aarch64-unknown-linux-gnu-gdb                obggcc/bin/alpha-unknown-linux-gnu-gdb
obggcc/bin/arm-unknown-linux-gnueabi-gdb
obggcc/bin/arm-unknown-linux-gnueabihf-gdb
obggcc/bin/hppa-unknown-linux-gnu-gdb
obggcc/bin/i386-unknown-linux-gnu-gdb
obggcc/bin/ia64-unknown-linux-gnu-gdb
obggcc/bin/mips64el-unknown-linux-gnuabi64-gdb
obggcc/bin/mipsel-unknown-linux-gnu-gdb
obggcc/bin/mips-unknown-linux-gnu-gdb
obggcc/bin/powerpc64le-unknown-linux-gnu-gdb
obggcc/bin/powerpc-unknown-linux-gnu-gdb
obggcc/bin/s390-unknown-linux-gnu-gdb
obggcc/bin/s390x-unknown-linux-gnu-gdb
obggcc/bin/x86_64-unknown-linux-gnu-gdb
```

Note that we don't provide prebuilt binaries of the gdb-server. If you want to use GDB for cross-debugging, you should build it yourself.

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