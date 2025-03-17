import glob
import sys
import os

GLIBC_VERSIONS = (
	"2.31",
	"2.28",
	"2.24",
	"2.19",
	"2.17",
	"2.13",
	"2.11",
	"2.7",
	"2.3",
	"2.2",
	"2.1",
	"2"
)

install_prefix = sys.argv[1]
triplet = sys.argv[2]

headers = set()

for version in GLIBC_VERSIONS:
	sysroot = os.path.join(install_prefix, triplet) + version
	include_directory = os.path.join(sysroot, "include")
	include_missing_directory = os.path.join(sysroot, "include-missing")
	
	if not os.path.exists(path = sysroot):
		continue
	
	items = glob.glob(
		pathname = os.path.join(include_directory, "**"),
		recursive = True
	)
	
	for item in items:
		if not item.endswith(".h"):
			continue
		
		item = item.replace(include_directory + "/", "")
		
		if not item:
			continue
		
		headers.add(item)

for version in GLIBC_VERSIONS:
	sysroot = os.path.join(install_prefix, triplet) + version
	include_directory = os.path.join(sysroot, "include")
	include_missing_directory = os.path.join(sysroot, "include-missing")
	
	if not os.path.exists(path = sysroot):
		continue
	
	if not os.path.exists(path = include_missing_directory):
		os.mkdir(path = include_missing_directory)
	
	for header in headers:
		path = os.path.join(include_directory, header)
		
		if os.path.exists(path = path):
			continue
		
		broken_header = os.path.join(include_missing_directory, header)
		
		if os.path.exists(path = broken_header):
			continue
		
		print("- The header '%s' does not exists in GLIBC %s; creating a broken one at '%s'" % (header, version, broken_header))
		
		path = os.path.dirname(p = broken_header)
		
		if not os.path.exists(path = path):
			os.makedirs(name = path, exist_ok = True)
		
		with open(file = broken_header, mode = "w") as file:
			file.write("#error This header is not available in GLIBC %s\n\n" % (version))
