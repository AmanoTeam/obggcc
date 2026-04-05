# OBGGCC

A Clang and GCC cross-compiler targeting older glibc versions.

## How it works

I extracted the sysroots from almost all major legacy Debian releases and built a cross-compiler for each. Each toolchain targets a specific Debian version, and therefore a specific glibc version.

This eliminates the need to install an ancient Linux distribution in Docker/LXC just to build portable binaries, which is the current standard practice.

## Supported targets

Currently, OBGGCC provides cross-compilers targeting 6 major Ubuntu releases and 9 major Debian releases on 5 system architectures.

### Distributions

| distribution_version              | glibc_version | linux_version | gcc_version | availability_date |
| :-------------------------------: | :-----------: | :-----------: | :---------: | :---------------: |
| Debian 4 (Etch)                  | glibc 2.3.6   | Linux 2.6.18  | GCC 4.1.1   | 2007              |
| Debian 5 (Lenny)                 | glibc 2.7     | Linux 2.6.26  | GCC 4.3.2   | 2009              |
| Debian 6 (Squeeze)               | glibc 2.11    | Linux 2.6.32  | GCC 4.4.5   | 2011              |
| Debian 7 (Wheezy)                | glibc 2.13    | Linux 3.2.78  | GCC 4.7.2   | 2013              |
| Ubuntu 12.04 (Precise Pangolin)  | glibc 2.15    | Linux 3.2.0   | GCC 4.6.3   | 2012              |
| Debian 7 (Sid)                   | glibc 2.17    | Linux 3.12.9  | GCC 4.8.2   | 2014              |
| Debian 8 (Jessie)                | glibc 2.19    | Linux 3.16.56 | GCC 4.9.2   | 2015              |
| Ubuntu 16.04 (Xenial Xerus)      | glibc 2.23    | Linux 4.4.0   | GCC 5.3.1   | 2016              |
| Debian 9 (Stretch)               | glibc 2.24    | Linux 4.9.228 | GCC 6.3.0   | 2017              |
| Ubuntu 18.04 (Bionic Beaver)     | glibc 2.27    | Linux 4.15.0  | GCC 7.3.0   | 2018              |
| Debian 10 (Buster)               | glibc 2.28    | Linux 4.19.249| GCC 8.3.0   | 2019              |
| Debian 11 (Bullseye)             | glibc 2.31    | Linux 5.10.223| GCC 10.2.1  | 2021              |
| Ubuntu 22.04 (Jammy Jellyfish)   | glibc 2.35    | Linux 5.15.0  | GCC 11.2.0  | 2022              |
| Ubuntu 24.04 (Noble Numbat)      | glibc 2.39    | Linux 6.8.0   | GCC 13.2.0  | 2024              |

### System architectures

- `aarch64-unknown-linux-gnu`
- `arm-unknown-linux-gnueabi`
- `arm-unknown-linux-gnueabihf`
- `i386-unknown-linux-gnu`
- `x86_64-unknown-linux-gnu`

### Quick start

First, start by [downloading the precompiled binaries for your platform](#releases). For reference, you can download the prebuilt binaries of the toolchain for Linux x86_64 with:

```
$ wget https://github.com/AmanoTeam/obggcc/releases/download/latest/x86_64-unknown-linux-gnu.tar.xz
$ tar --extract --file=x86_64-unknown-linux-gnu.tar.xz
```

After unpacking the tarball, you will find the cross-compiler as well as wrapper scripts for targeting specific glibc versions inside the `obggcc/bin` directory:

```
$ ls obggcc/bin
...
x86_64-unknown-linux-gnu2.31-clang
x86_64-unknown-linux-gnu2.31-clang++
x86_64-unknown-linux-gnu2.31-g++
x86_64-unknown-linux-gnu2.31-gcc
...
```

You can use them as follows:

```
# Compile C programs
$ x86_64-unknown-linux-gnu2.31-gcc main.c -o main
# Compile C++ programs
$ x86_64-unknown-linux-gnu2.31-g++ main.c -o main
```

If you prefer, you can use Clang instead of GCC. Just make sure to replace the `gcc`/`g++` suffixes with `clang`/`clang++`.

## Cross-compilation

### CMake

Inside the `obggcc/build/cmake` directory, there is a custom CMake toolchain file for each cross-compilation target OBGGCC supports:

```bash
$ ls obggcc/build/cmake
aarch64-unknown-linux-gnu.cmake
aarch64-unknown-linux-gnu2.19.cmake
...
arm-unknown-linux-gnueabi.cmake
arm-unknown-linux-gnueabi2.11.cmake
...
arm-unknown-linux-gnueabihf.cmake
arm-unknown-linux-gnueabihf2.13.cmake
...
i386-unknown-linux-gnu.cmake
i386-unknown-linux-gnu2.11.cmake
...
x86_64-unknown-linux-gnu.cmake
x86_64-unknown-linux-gnu2.11.cmake
...
```

To use one of them, pass the `CMAKE_TOOLCHAIN_FILE` definition to your CMake command invocation:

```bash
# Setup the environment for cross-compilation
$ cmake \
    -DCMAKE_TOOLCHAIN_FILE=obggcc/build/cmake/aarch64-unknown-linux-gnu2.19.cmake \
    -B build \
    -S  ./
# Build the project
$ cmake --build ./build
```

### GNU Autotools (aka GNU Build System)

Inside the `obggcc/build/autotools` directory, there is a custom bash script for each cross-compilation target that OBGGCC supports:

```bash
$ ls obggcc/build/autotools
aarch64-unknown-linux-gnu.sh
aarch64-unknown-linux-gnu2.19.sh
...
arm-unknown-linux-gnueabi.sh
arm-unknown-linux-gnueabi2.11.sh
...
arm-unknown-linux-gnueabihf.sh
arm-unknown-linux-gnueabihf2.13.sh
...
i386-unknown-linux-gnu.sh
i386-unknown-linux-gnu2.11.sh
...
x86_64-unknown-linux-gnu.sh
x86_64-unknown-linux-gnu2.11.sh
...
```

They are meant to be `source`d by you whenever you want to cross-compile something:

```bash
# Setup the environment for cross-compilation
$ source obggcc/build/autotools/aarch64-unknown-linux-gnu2.19.sh

# Configure & build the project
$ ./configure --host="${CROSS_COMPILE_TRIPLET}"
$ make
```

Essentially, these scripts handle the setup of `CC`, `CXX`, `LD`, and other environment variables so you don’t need to configure them manually.

To restore your environment to its original state, run the `deactivate.sh` script from the same directory:

```bash
$ source obggcc/build/autotools/deactivate.sh
```

## Controlling OBGGCC Behavior

OBGGCC allows you to change its behavior in certain scenarios through the use of environment variables. Below are all the switches OBGGCC supports and their intended purposes:

- `OBGGCC_SYSTEM_LIBRARIES`  
  - Allows you to compile and link code using libraries and headers from your host machine’s system root in conjunction with the cross-compiler’s system root. See [Linking with system libraries](#linking-with-system-libraries).

- `OBGGCC_BUILTIN_LOADER`  
  - Allows you to change the dynamic linker of an executable during linkage so that it can be run using OBGGCC’s glibc libraries rather than your host machine’s glibc libraries. See [Running binaries with a specific glibc](#running-binaries-with-a-specific-glibc).

- `OBGGCC_RUNTIME_RPATH`  
  - Automatically appends the path to the directory containing the GCC runtime libraries (e.g., libsanitizer (AddressSanitizer), libatomic, and libstdc++) to your executables’ RPATH. This option is enabled by default when using `OBGGCC_BUILTIN_LOADER`.

- `OBGGCC_NZ`  
  - Allows you to use libraries and headers installed using OBGGCC’s package manager (nz) during cross-compilation. See [Software availability](#software-availability).

- `OBGGCC_STATIC_RUNTIME`  
  - Tells the cross-compiler to prefer linking with the static versions of the GCC runtime libraries rather than the dynamic ones.

- `OBGGCC_ARM_MODE`  
  - Tells the cross-compiler to generate code in ARM mode rather than Thumb-1/Thumb-2 mode.

- `OBGGCC_VERBOSE`  
  - Display the GCC subcommand invocation and the current working directory for every compilation process.

- `OBGGCC_STL_VERSION`  
  - Cross-compile code targeting a specific version of the GCC runtime libraries, rather than using the ones shipped with the compiler. See [Choosing an arbitrary GCC runtime version](#choosing-an-arbitrary-gcc-runtime-version).

You can enable a switch by setting its value to `true` (e.g: `export OBGGCC_NZ=true`), and disable it by setting its value to `false` (e.g: `export OBGGCC_NZ=false`).

For every environment variable, there is a matching command-line flag switch available, which you can check with `--obggcc-help`.

```
$ arm-unknown-linux-gnueabi2.7-gcc --obggcc-help
usage: arm-unknown-linux-gnueabi2.7-gcc [-f[no-]builtin-loader] [-f[no-]runtime-rpath] [-f[no-]system-libraries] [-f[no-]nz] [-f[no-]static-runtime] [-f[no-]verbose]

Linux C/C++ cross-compiler for targeting older glibc versions.

options:
  -f[no-]builtin-loader
                        Override the default dynamic linker to point to the local glibc installation within OBGGCC.
  -f[no-]runtime-rpath  Automatically append the path to the directory containing GCC runtime libraries to every executable's DT_RPATH.
  -f[no-]system-libraries
                        Allow compilation and linking with libraries and headers from your host machine’s system root alongside the cross-compiler’s system root.
  -f[no-]nz             Use libraries and headers installed by OBGGCC’s package manager (nz) during cross-compilation.
  -f[no-]static-runtime
                        Force linking with static variants of the GCC runtime libraries instead of the dynamic versions.
  -f[no-]verbose        Display the GCC subcommand invocation and the current working directory.

The flag switches listed above are not guaranteed to be portable and are primarily intended for testing purposes.
```

## Software availability

The cross-compiler ships only with the minimum required to build a working C/C++ program. That is, you won't find any prebuilt binaries of popular projects like OpenSSL or zlib available for use, as you would on an average Linux distribution.

If your project depends on something other than the GNU C library (or the C++ standard libraries, for C++ programs), you need to either build it yourself or install it from somewhere else. For convenience, OBGGCC provides an APT-like tool to install packages from APT repositories to a local directory and enable their usage during cross-compilations.

### Installing project dependencies with `nz`

You can install packages to a specific system root using the corresponding `<triplet><glibc_version>-nz` command inside the `obggcc/bin` directory.

For example, let's suppose you want to cross-compile curl for Ubuntu 16.04 (glibc 2.23) with SSL and HTTP/2 support:

#### Step 1

First, fetch curl sources and generate the required build files:

```bash
$ git clone -b curl-8_14_1 https://github.com/curl/curl
$ cd curl
$ autoreconf -fi
```

#### Step 2

Now, configure the environment for cross-compilation:

```bash
$ source obggcc/build/autotools/x86_64-unknown-linux-gnu2.23.sh
```

#### Step 3

Install the required dependencies:

```bash
$ x86_64-unknown-linux-gnu2.23-nz \
    --install 'libnghttp2-dev;libssl-dev;zlib1g-dev;libpsl-dev'
```

There is also an `apt` script wrapper around `nz` that you can use to install packages _à la_ the APT way:

```bash
$ x86_64-unknown-linux-gnu2.23-apt install \
    libnghttp2-dev \
    libssl-dev \
    zlib1g-dev \
    libpsl-dev
```

#### Step 4

Before cross-compiling curl, set the `OBGGCC_NZ` environment variable to enable OBGGCC to use libraries from nz's system root during the build:

```bash
$ export OBGGCC_NZ=true
```

#### Step 5

Now you can build curl the usual way:

```bash
$ ./configure \
    --host="${CROSS_COMPILE_TRIPLET}" \
    --with-openssl \
    --with-nghttp2 
$ make
```

## Linking with system libraries

By default, you can't mix headers and libraries from both the cross-compiler's system root and your host machine's system root. The main reason is that there may be incompatibilities between the two systems, which could cause the compilation to break or produce broken binaries even if the compilation succeeds. Due to this, the standard practice is that you also cross-compile any external dependency that your program may need to use and put them inside the cross-compiler's system root or explicitly point the compiler to where to find the cross-compiled dependencies by passing the appropriate flags when compiling your binary (`-I`, `-L`, and `-l`).

That being said, the GNU C library is portable enough to reduce these incompatibilities to some extent.

The environment variable `OBGGCC_SYSTEM_LIBRARIES` can be used to force OBGGCC to use your host machine's system root when cross-compiling.

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

That's because there are no prebuilt OpenSSL binaries in the cross-compiler's sysroot.

Now let's try setting the `OBGGCC_SYSTEM_LIBRARIES` environment variable:

```bash
$ export OBGGCC_SYSTEM_LIBRARIES=true
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

Note that, despite explicitly adding directories of the host machine to the compiler invocation, any program you compile will still link against the cross-compiler's glibc. That's due to the fact that the directories of the cross-compiler's system root are checked first before the directories of your host machine's system root. Only when the compiler can't find a specific header or library in the cross-compiler's system root does it search for it in your machine's system root. This way, we can build portable programs while still linking against third-party libraries from the host machine.

Keep in mind that `OBGGCC_SYSTEM_LIBRARIES` is only useful when you are cross-compiling software to a target that matches the one of your host machine (e.g., your host machine is an `x86_64-linux-gnu` system, and you are cross-compiling software targeting an `x86_64-linux-gnu` system as well), as you can't link binaries with mismatching ABIs:

```bash
# My machine is an x86_64 system
$ gcc -dumpmachine
x86_64-unknown-linux-gnu

# This will work, as both targets match
$ x86_64-unknown-linux-gnu2.3-gcc main.c -lcrypto -o main

# But this won't work; mismatching architectures
$ aarch64-unknown-linux-gnu2.19-gcc main.c -lcrypto -o main
ld: skipping incompatible /lib64/libcrypto.so when searching for -lcrypto
ld: skipping incompatible /usr/lib64/libcrypto.so when searching for -lcrypto
ld: cannot find -lcrypto: No such file or directory
ld: skipping incompatible /lib64/libcrypto.so when searching for -lcrypto
ld: skipping incompatible /usr/lib64/libcrypto.so when searching for -lcrypto
collect2: error: ld returned 1 exit status
```

This also works if your system has multilib support (i.e., both 32-bit and 64-bit libraries coexist on the same system) and you are cross-compiling software targeting the 32-bit version of your system:

```bash
$ dpkg --print-architecture
amd64
$ dpkg --print-foreign-architectures
i386

# This wil work
$ x86_64-unknown-linux-gnu2.3-gcc main.c -lcrypto -o main
$ ./main
It works!

# This will work as well
$ i386-unknown-linux-gnu2.3-gcc main.c -lcrypto -o main
$ ./main
It works!
```

## Running binaries with a specific glibc

There may be cases where you might want to run your software under a specific glibc version, either to check how it will behave or to try a new glibc feature that was added in a later version but isn’t available in the glibc installed on your system.

The environment variable `OBGGCC_BUILTIN_LOADER` can be used to change the default loader (aka dynamic linker) of an executable when cross-compiling. In other words, you can use this to force your binary to use a different glibc at runtime, ignoring the one available in your system.

For some background, here is my current system:

```bash
$ cat /etc/os-release
NAME="CentOS Linux"
VERSION="7 (Core)"
...

$ ldd --version
ldd (GNU libc) 2.17
...
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

I can try cross-compiling my code to glibc 2.25+, but then I would not be able to run it on my machine due to the older glibc it has:

```
$ x86_64-unknown-linux-gnu2.27-gcc main.c -o main
$ ./main
./main: /lib64/libc.so.6: version `GLIBC_2.25' not found (required by ./main)
```

However, things change when I use `OBGGCC_BUILTIN_LOADER`:

```bash
$ export OBGGCC_BUILTIN_LOADER=true
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

## Choosing an arbitrary GCC runtime version

By default, cross-compilation links against the runtime libraries of the GCC version used for the build. For bleeding-edge releases, this gives you access to the latest features the compiler has to offer, but comes with the drawback that you will probably need to bundle the GCC libraries or statically link them with your binary (especially for C++ code) if you want it to run out of the box on systems that use an older version of GCC as the base system compiler.

If portability takes precedence over new features for you, it is possible to choose an arbitrary GCC runtime version to use during the build.

### Usage

The extra runtimes are not shipped with ordinary releases by default because they take a lot of storage space. To use them, you need to manually install them using the `gcc-stl-install` tool:

```bash
$ gcc-stl-install x86_64-unknown-linux-gnu
```

For other architectures, replace `x86_64-unknown-linux-gnu` with the corresponding target triplet.

To make the cross-compiler use them, set the `OBGGCC_STL_VERSION` environment variable to the GCC version you intend the compiled code to be compatible with. Currently, this variable accepts values from `3.4` to `16`:

```bash
# Set it to use the GCC 12 runtime
$ export OBGGCC_STL_VERSION=12
# Now just use the compiler as you normally would
$ x86_64-unknown-linux-gnu2.31-g++ [...]
```

To make the runtime version match the one used by the distro you are targeting, simply set the version to the one specified in the `gcc_version` field of the [Distributions](#distributions) table.

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
...
SUMMARY: AddressSanitizer: SEGV /home/runner/main.c:4 in main
==19765==ABORTING
```

If you are playing around with AddressSanitizer, you might want to set the `OBGGCC_RUNTIME_RPATH` environment variable:

```bash
export OBGGCC_RUNTIME_RPATH=true
```

This tells the linker to automatically add the `RPATH` of the directory containing the AddressSanitizer libraries to your executable, so you don't have to bother with setting the `LD_LIBRARY_PATH` or adding the rpath manually:

```bash
$ x86_64-unknown-linux-gnu2.3-gcc -fsanitize=address main.c -o main
$ ./main
./main: error while loading shared libraries: libasan.so.8: cannot open shared object file: No such file or directory
$ export OBGGCC_RUNTIME_RPATH=true
$ x86_64-unknown-linux-gnu2.3-gcc -fsanitize=address main.c -o main
$ ./main
<it works>
```

## GDB (GNU Debugger)

There are also bundled binaries of GDB available for use:

```
$ ls obggcc/bin/*-gdb
obggcc/bin/aarch64-unknown-linux-gnu-gdb
obggcc/bin/arm-unknown-linux-gnueabi-gdb
obggcc/bin/arm-unknown-linux-gnueabihf-gdb
obggcc/bin/i386-unknown-linux-gnu-gdb
obggcc/bin/x86_64-unknown-linux-gnu-gdb
```

Note that we don't provide prebuilt binaries of the gdb-server. If you want to use GDB for cross-debugging, you should build it yourself.

## Building OBGGCC

*Compiling OBGGCC from source is best supported on Linux. We recommend using a Debian/Ubuntu-based distribution for this.*

Start by installing the dependencies with:

```
$ sudo apt-get install build-essential autoconf libtool jq python3-minimal zsh
```

Then, clone the repository with:

```
$ git clone https://github.com/AmanoTeam/obggcc
$ cd obggcc
$ git submodule update --init --depth=1
$ git -C submodules/nz submodule update --init --depth=1
```

Build the project with:

```
$ ./build.sh
```

This will fetch the GCC sources and build a cross-compiler for all supported architectures.

By default, it will install everything to `/var/tmp`, but you can customize the location by setting the `OBGGCC_BUILD_DIRECTORY` environment variable:

```
$ export OBGGCC_BUILD_DIRECTORY=path/to/directory
$ ./build.sh
```

You can also customize the build parallelism with `OBGGCC_BUILD_PARALLEL_LEVEL`:

```
$ export OBGGCC_BUILD_PARALLEL_LEVEL=$(nproc)
$ ./build.sh
```

The GCC version can be selected using `OBGGCC_RELEASE`.

```
$ export OBGGCC_RELEASE=15
$ ./build.sh
```

You can also customize which targets the toolchain will support for cross-compilation using `OBGGCC_TARGETS`. For a minimal build with support for a single architecture and a specific glibc version, you can do something like:

```
# Build a cross-compiler targeting glibc 2.17 on x86_64.
$ export OBGGCC_TARGETS='x86_64-unknown-linux-gnu,x86_64-unknown-linux-gnu2.17'
$ ./build.sh
```

You can find the list of supported target triplets [here](#system-architectures) and the list of supported glibc versions [here](#distributions). 

## Releases

There are precompiled binaries available for multiple GCC major releases, listed below:

- [GCC 16](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-16) - current development release
- [GCC 15](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-15) - current stable release
- [GCC 14](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-14)
- [GCC 13](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-13)
- [GCC 12](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-12)
- [GCC 11](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-11)
- [GCC 10](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-10)
- [GCC 9](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-9)
- [GCC 8](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-8)
- [GCC 7](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-7)
- [GCC 6](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-6)
- [GCC 5](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-5)
- [GCC 4.9](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-4.9)
- [GCC 4.8](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-4.8)
- [GCC 4.7](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-4.7)
- [GCC 4.6](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-4.6)
- [GCC 4.5](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-4.5)
- [GCC 4.4](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-4.4)
- [GCC 4.3](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-4.3)
- [GCC 4.2](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-4.2)
- [GCC 4.1](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-4.1)
- [GCC 4.0](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-4.0)
- [GCC 3.4](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-3.4)
- [GCC 3.3](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-3.3)
- [GCC 3.2](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-3.2)
- [GCC 3.1](https://github.com/AmanoTeam/obggcc/releases/tag/gcc-3.1)

Only the **stable** and **development** releases are actively maintained. Older releases may occasionally receive updates to incorporate fixes or improvements from maintained versions, but they are treated as low priority.
