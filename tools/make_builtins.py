import json
import os
import subprocess
import tempfile

source = """
#include <%s.h>

int main(void) {
	(void) %s;
	return 0;
}
"""

destination = """
#if !defined(BUILTIN_COMPAT_%s_H)
#define BUILTIN_COMPAT_%s_H
%s

#endif
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

content = None

cc = os.getenv(key = "CC")

assert cc is not None

def check_symbols_exists(symbol):
	
	(name, header) = (symbol["name"], symbol["header"])
	
	text = source % (header, name)
	
	with open(file = temporary_file, mode = "w") as file:
		file.write(text)
	
	process = subprocess.run([cc, "-w", "-fno-builtin", temporary_file, "-o", "%s.o" % temporary_file])
	
	return process.returncode == 0

def dump_builtins(builtins, name):
	
	s = ""
	
	items = getattr(builtins, name)
	
	if not items:
		return None
	
	items.sort()
	
	for item in items:
		s += ("\n#define %s __builtin_%s" % (item, item))
	
	dump = destination % (name.upper(), name.upper(), s)
	
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

dump_builtins(builtins, "math")
dump_builtins(builtins, "stdio")
dump_builtins(builtins, "complex")
dump_builtins(builtins, "stdlib")
dump_builtins(builtins, "ctype")
