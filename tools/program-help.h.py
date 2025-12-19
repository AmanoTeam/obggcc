#!/usr/bin/env python3

import argparse
import os
import io
import json

parser = argparse.ArgumentParser(
	prog = "%s",
	description = "Linux C/C++ cross-compiler targeting older glibc versions.",
	allow_abbrev = False,
	add_help = False,
	epilog = "The flag switches listed above are not guaranteed to be portable and are primarily intended for testing purposes."
)

parser.add_argument(
	"-f[no-]builtin-loader",
	required = False,
	action = "store_true",
	help = "Override the default dynamic linker to point to the local glibc installation within OBGGCC."
)

parser.add_argument(
	"-f[no-]runtime-rpath",
	required = False,
	action = "store_true",
	help = "Automatically append the path to the directory containing GCC runtime libraries to every executable's DT_RPATH."
)

parser.add_argument(
	"-f[no-]system-libraries",
	required = False,
	action = "store_true",
	help = "Allow compilation and linking with libraries and headers from your host machine’s system root alongside the cross-compiler’s system root."
)

parser.add_argument(
	"-f[no-]nz",
	required = False,
	action = "store_true",
	help = "Use libraries and headers installed by OBGGCC’s package manager (nz) during cross-compilation."
)

parser.add_argument(
	"-f[no-]static-runtime",
	required = False,
	action = "store_true",
	help = "Force linking with static variants of the GCC runtime libraries instead of the dynamic versions."
)

parser.add_argument(
	"-f[no-]verbose",
	required = False,
	action = "store_true",
	help = "Display the GCC subcommand invocation and the current working directory."
)

os.environ["LINES"] = "1000"
os.environ["COLUMNS"] = "1000"

file = io.StringIO()
parser.print_help(file = file)
file.seek(0, io.SEEK_SET)

text = file.read()

header = """/*
This file is auto-generated. Use the tool at ../tools/program_help.h.py to regenerate.
*/

#if !defined(PROGRAM_HELP_H)
#define PROGRAM_HELP_H

#define PROGRAM_HELP \\\n\
"""

for line in text.splitlines():
	line = json.dumps(
		obj = (
			(line + "\n")
				.replace("|", "\n")
		)
	)
	header += '\t%s\\\n' % line

header += "\n#endif\n"

destination = os.path.join(
	os.path.dirname(
		p = os.path.dirname(
			p = os.path.realpath(
				filename = __file__
			)
		)
	),
	"tools/gcc-wrapper/program_help.h"
)
	
print("Saving to '%s'" % (destination))

with open(file = destination, mode = "w") as file:
	file.write(header)
