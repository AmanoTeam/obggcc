import json
import os
import subprocess
import tempfile
import shutil

headers = (
	"math",
	"stdio",
	"complex",
	"stdlib",
	"ctype",
	"wctype",
	"inttypes"
)

source = """\
#include <%s.h>

int main(void) {
	(void) %s;
	return 0;
}
"""

destination = """\
/* This file is auto generated. */
%s
"""

temporary_file = os.path.join(tempfile.gettempdir(), "main.c")

symbols_file = os.path.join(
	os.path.dirname(p = __file__),
	"builtin_symbols.json"
)

class Builtins:
	
	def __init__(self):
		self.math = []
		self.stdio = []
		self.complex = []
		self.stdlib = []
		self.ctype = []
		self.wctype = []
		self.inttypes = []

content = None

cc = os.getenv(key = "CC")
c_include_path = os.getenv(key = "C_INCLUDE_PATH")

assert cc is not None
assert c_include_path is not None

def check_symbols_exists(symbol):
	
	(name, header) = (symbol["name"], symbol["header"])
	
	text = source % (header, name)
	
	with open(file = temporary_file, mode = "w") as file:
		file.write(text)
	
	process = subprocess.run([cc, "-std=c11", "-w", "-fno-builtin", temporary_file, "-o", "%s.o" % temporary_file])
	
	return process.returncode == 0

def dump_builtins(builtins, name):
	
	s = ""
	
	items = getattr(builtins, name)
	
	if not items:
		return None
	
	items.sort()
	
	for item in items:
		s += ("\n#if !defined(%s)\n#define %s __builtin_%s\n#endif\n" % (item, item, item))
	
	dump = destination % (
		s
	)
	
	with open(file = "builtin_%s.h" % (name), mode = "w") as file:
		file.write(dump)

with open(file = symbols_file, mode = "r") as file:
	content = file.read()

symbols = json.loads(s = content)

builtins = Builtins()

for symbol in symbols:
	exists = check_symbols_exists(symbol = symbol)
	
	if exists:
		continue
	
	(name, header) = (
		symbol["name"],
		symbol["header"]
	)
	
	items = getattr(builtins, header)
	items.append(name)

for name in headers:
	dump_builtins(builtins = builtins, name = name)
	
	src = "builtin_%s.h" % (name)
	dst = os.path.join(c_include_path, "%s.h" % (name))
	
	if not os.path.exists(path = src):
		continue
	
	print("- Modifying '%s'" % (dst))
	
	with open(file = dst, mode = "a+") as file:
		file.write("\n\n#include <%s>" % (src))
	
	dst = os.path.join(c_include_path, src)
	
	if os.path.exists(path = dst):
		os.remove(dst)
	
	shutil.move(src = src, dst = dst)
