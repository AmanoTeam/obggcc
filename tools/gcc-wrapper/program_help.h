/*
This file is auto-generated. Use the tool at ../tools/program_help.h.py to regenerate.
*/

#if !defined(PROGRAM_HELP_H)
#define PROGRAM_HELP_H

#define PROGRAM_HELP \
	"usage: %s [-f[no-]builtin-loader] [-f[no-]runtime-rpath] [-f[no-]system-libraries] [-f[no-]nz] [-f[no-]static-runtime] [-f[no-]verbose]\n"\
	"\n"\
	"Linux C/C++ cross-compiler targeting older glibc versions.\n"\
	"\n"\
	"options:\n"\
	"  -f[no-]builtin-loader\n"\
	"                        Override the default dynamic linker to point to the local glibc installation within OBGGCC.\n"\
	"  -f[no-]runtime-rpath  Automatically append the path to the directory containing GCC runtime libraries to every executable's DT_RPATH.\n"\
	"  -f[no-]system-libraries\n"\
	"                        Allow compilation and linking with libraries and headers from your host machine\u2019s system root alongside the cross-compiler\u2019s system root.\n"\
	"  -f[no-]nz             Use libraries and headers installed by OBGGCC\u2019s package manager (nz) during cross-compilation.\n"\
	"  -f[no-]static-runtime\n"\
	"                        Force linking with static variants of the GCC runtime libraries instead of the dynamic versions.\n"\
	"  -f[no-]verbose        Display the GCC subcommand invocation and the current working directory.\n"\
	"\n"\
	"The flag switches listed above are not guaranteed to be portable and are primarily intended for testing purposes.\n"\

#endif
