#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <unistd.h>
#include <sys/wait.h>

#include "fs/getexec.h"
#include "fs/parentpath.h"
#include "fs/basename.h"
#include "fs/exists.h"
#include "fs/sep.h"
#include "fs/rm.h"
#include "fstream.h"
#include "errors.h"
#include "obggcc.h"
#include "query.h"

extern char** environ;

static const char HYPHEN[] = "-";
static const char BINUTILS_STRIP[] = "strip";

static const char LLVM_STRIP[] = "llvm-strip";
static const char LLVM_OBJCOPY[] = "llvm-objcopy";

static const char LLVM_COMMAND_PREFIX[] = "llvm-";

static const char LIBCXX_SHARED[] = "libc++_shared.so";

#define BINFMT_X86_64 (0x01)
#define BINFMT_i386 (0x02)
#define BINFMT_MIPS64EL (0x03)
#define BINFMT_MIPSEL (0x04)
#define BINFMT_RISCV64 (0x05)
#define BINFMT_AARCH64 (0x06)
#define BINFMT_ARM (0x07)

#define BINFMT_SIZE (40)

struct binfmt {
	int type;
	const char* name;
	unsigned char magic[BINFMT_SIZE];
	unsigned char mask[BINFMT_SIZE];
};

static const struct binfmt binfmts[] = {
	{
		.type = BINFMT_X86_64,
		.name = "x86_64",
		.magic = {0x7f, 0x45, 0x4c, 0x46, 0x2, 0x1, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x0, 0x3e, 0x0},
		.mask = {0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff}
	},
	{
		.type = BINFMT_i386,
		.name = "i386",
		.magic = {0x7f, 0x45, 0x4c, 0x46, 0x1, 0x1, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x0, 0x3, 0x0},
		.mask = {0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff}
	},
	{
		.type = BINFMT_MIPS64EL,
		.name = "mips64el",
		.magic = {0x7f, 0x45, 0x4c, 0x46, 0x2, 0x1, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x0, 0x8, 0x0},
		.mask = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff}
	},
	{
		.type = BINFMT_MIPSEL,
		.name = "mipsel",
		.magic = {0x7f, 0x45, 0x4c, 0x46, 0x1, 0x1, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x0, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
		.mask = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x20}
	},
	{
		.type = BINFMT_RISCV64,
		.name = "riscv64",
		.magic = {0x7f, 0x45, 0x4c, 0x46, 0x2, 0x1, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x0, 0xf3, 0x0},
		.mask = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff}
	},
	{
		.type = BINFMT_AARCH64,
		.name = "aarch64",
		.magic = {0x7f, 0x45, 0x4c, 0x46, 0x2, 0x1, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x0, 0xb7},
		.mask = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff}
	},
	{
		.type = BINFMT_ARM,
		.name = "arm",
		.magic = {0x7f, 0x45, 0x4c, 0x46, 0x1, 0x1, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x0, 0x28, 0x0},
		.mask = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff}
	}
};

static const struct binfmt* binfmt_guess(const char* const filename) {
	
	unsigned char chunk[BINFMT_SIZE];
	
	unsigned char a[BINFMT_SIZE];
	unsigned char b[BINFMT_SIZE];
	
	const struct binfmt* fmt = NULL;
	
	size_t index = 0;
	size_t offset = 0;
	
	fstream_t* stream = NULL;
	
	int status = 0;
	
	memset(chunk, '\0', sizeof(chunk));
	memset(a, '\0', sizeof(a));
	memset(b, '\0', sizeof(b));
	
	stream = fstream_open(filename, FSTREAM_READ);
	
	if (stream == NULL) {
		goto end;
	}
	
	if (fstream_read(stream, chunk, sizeof(chunk)) == FSTREAM_ERROR) {
		goto end;
	}
	
	for (index = 0; index < sizeof(binfmts) / sizeof(*binfmts); index++) {
		fmt = &binfmts[index];
		
		for (offset = 0; offset < BINFMT_SIZE; offset++) {
			a[offset] = (fmt->magic[offset] & fmt->mask[offset]);
		}
		
		for (offset = 0; offset < BINFMT_SIZE; offset++) {
			b[offset] = (chunk[offset] & fmt->mask[offset]);
		}
		
		if (memcmp(a, b, BINFMT_SIZE) == 0) {
			goto end;
		}
	}
	
	fmt = NULL;
	
	end:;
	
	fstream_close(stream);
	
	return fmt;
	
}

static const char* binfmt_get_triplet(const struct binfmt* const fmt) {
	
	switch (fmt->type) {
		case BINFMT_X86_64:
			#if defined(OBGGCC)
				return "x86_64-unknown-linux-gnu";
			#elif defined(PINO)
				return "x86_64-unknown-linux-android";
			#endif
		case BINFMT_i386:
			#if defined(OBGGCC)
				return "i386-unknown-linux-gnu";
			#elif defined(PINO)
				return "i686-unknown-linux-android";
			#endif
		case BINFMT_MIPS64EL:
			#if defined(OBGGCC)
				return "mips64el-unknown-linux-gnuabi64";
			#elif defined(PINO)
				return "mips64el-unknown-linux-android";
			#endif
		case BINFMT_MIPSEL:
			#if defined(OBGGCC)
				return "mipsel-unknown-linux-gnu";
			#elif defined(PINO)
				return "mipsel-unknown-linux-android";
			#endif
		case BINFMT_RISCV64:
			#if defined(PINO)
				return "riscv64-unknown-linux-android";
			#endif
		case BINFMT_AARCH64:
			#if defined(OBGGCC)
				return "aarch64-unknown-linux-gnu";
			#elif defined(PINO)
				return "aarch64-unknown-linux-android";
			#endif
		case BINFMT_ARM:
			#if defined(OBGGCC)
				return "arm-unknown-linux-gnueabihf";
			#elif defined(PINO)
				return "armv7-unknown-linux-androideabi";
			#endif
	}
	
	return NULL;
	
}

static int ends_with(const char* const string, const char* const sep) {
	
	const size_t asize = strlen(string);
	const size_t bsize = strlen(sep);
	
	const char* pos = string;
	
	if (bsize > asize) {
		return 0;
	}
	
	pos += (asize - bsize);
	
	return strcmp(pos, sep) == 0;
	
}

static const char* const ARGUMENT_EXPECTS_VALUE[] = {
	"-o",
	"--keep-section",
	"--remove-relocations",
	"--add-gnu-debuglink",
	"--keep-section",
	"--interleave-width",
	"--gap-fill",
	"--pad-to",
	"--set-start",
	"-K", "--keep-symbol",
	"-N", "--strip-symbol",
	"-R", "--remove-section",
	"-I", "--input-target",
	"-O", "--output-target",
	"-F", "--target",
	"-j", "--only-section",
	"-L", "--localize-symbol",
	"-G", "--keep-global-symbol",
	"-W", "--weaken-symbol",
	"-i", "--interleave",
	"-b", "--byte",
	"--change-start", "--adjust-start",
	"--change-addresses", "--adjust-vma",
	"--change-section-address", "--adjust-section-vma",
	"--change-section-lma",
	"--change-section-vma",
	"--set-section-flags",
	"--set-section-alignment",
	"--add-section",
	"--update-section",
	"--dump-section",
	"--rename-section",
	"--long-section-names",
	"--reverse-bytes",
	"--redefine-sym",
	"--redefine-syms",
	"--srec-len",
	"--strip-symbols",
	"--strip-unneeded-symbols",
	"--keep-symbols",
	"--localize-symbols",
	"--globalize-symbols",
	"--keep-global-symbols",
	"--weaken-symbols",
	"--add-symbol",
	"--alt-machine-code",
	"--prefix-symbols",
	"--prefix-sections",
	"--prefix-alloc-sections",
	"--file-alignment",
	"--heap",
	"--image-base",
	"--section-alignment",
	"--stack",
	"--subsystem",
	"--compress-debug-sections",
	"--elf-stt-common",
	"--verilog-data-width"
};

static int argument_expects_value(const char* const name) {
	
	size_t index = 0;
	const char* item = NULL;
	
	if (name == NULL) {
		return 0;
	}
	
	for (index = 0; index < sizeof(ARGUMENT_EXPECTS_VALUE) / sizeof(*ARGUMENT_EXPECTS_VALUE); index++) {
		item = ARGUMENT_EXPECTS_VALUE[index];
		
		if (strcmp(name, item) == 0) {
			return 1;
		}
	}
	
	return 0;
	
}
 
int main(int argc, char* argv[], char* envp[]) {
	
	hquery_t query = {0};
	
	pid_t pid = 0;
	int wstatus = 0;
	
	int verbose = 0;
	
	int err = ERR_SUCCESS;
	
	const struct binfmt* fmt = NULL;
	
	const char* triplet = NULL;
	const char* output = NULL;
	char* input = NULL;
	
	const char* file_name = NULL;
	char* parent_directory = NULL;
	char* app_filename = NULL;
	
	const char* prev = NULL;
	char* cur = NULL;
	char* executable = NULL;
	
	size_t index = 0;
	
	char** args = NULL;
	
	size_t kargc = 0;
	char** kargv = NULL;
	
	size_t inputsc = 0;
	char** inputs = NULL;
	
	kargv = malloc(
		sizeof(*argv) * (
			argc + 
			1 /* NULL */
		)
	);
	
	if (kargv == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	inputs = malloc(sizeof(*argv) * (argc + 1));
	
	if (inputs == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	query_load_environ(&query);
	
	verbose = query_get_bool(&query, ENV_VERBOSE) == 1;
	
	for (index = 1; index < (size_t) argc; index++) {
		cur = argv[index];
		
		if (ends_with(cur, ".sym") && file_exists(cur) != 1) {
			continue;
		}
		
		if (prev != NULL && strcmp(prev, "-o") == 0) {
			output = cur;
		}
		
		if (ends_with(cur, ".sym") || cur[0] == '-' || argument_expects_value(prev)) {
			kargv[kargc++] = cur;
		} else {
			inputs[inputsc++] = cur;
		}
		
		prev = cur;
	}
	
	if (inputsc == 0) {
		fprintf(stderr, "nothing here but crickets\n");
		goto end;
	}
	
	kargv[kargc++] = NULL;
	
	app_filename = get_app_filename();
	
	if (app_filename == NULL) {
		err = ERR_GET_APP_FILENAME_FAILURE;
		goto end;
	}
	
	file_name = basename(app_filename);
	
	if (!(strcmp(file_name, LLVM_STRIP) == 0 || strcmp(file_name, LLVM_OBJCOPY) == 0)) {
		err = ERR_UNKNOWN_BINUTILS_WRAPPER;
		goto end;
	}
	
	file_name += strlen(LLVM_COMMAND_PREFIX);
	
	parent_directory = malloc(strlen(app_filename) + 1);
	
	if (parent_directory == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	/* Get the directory where our executable is (i.e., <prefix>/bin) */
	get_parent_path(app_filename, parent_directory, 1);
	
	args = malloc(
		sizeof(*kargv) * (
			kargc + 
			1 + /* <triplet>-<strip> */
			1 + /* <input> */
			1 /* NULL */
		)
	);
	
	if (args == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	memcpy(&args[1], kargv, kargc * sizeof(*kargv));
	
	for (index = 0; index < inputsc; index++) {
		input = inputs[index];
		
		fmt = binfmt_guess(input);
		
		if (fmt == NULL) {
			err = ERR_BINFMT_GUESS_FAILURE;
			goto end;
		}
		
		triplet = binfmt_get_triplet(fmt);
		
		if (triplet == NULL) {
			err = ERR_BINFMT_GUESS_FAILURE;
			goto end;
		}
		
		free(executable);
		
		executable = malloc(strlen(parent_directory) + strlen(PATHSEP_S) + strlen(triplet) + strlen(HYPHEN) + strlen(file_name) + 1);
		
		if (executable == NULL) {
			err = ERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(executable, parent_directory);
		strcat(executable, PATHSEP_S);
		strcat(executable, triplet);
		strcat(executable, HYPHEN);
		strcat(executable, file_name);
		
		args[0] = executable;
		args[kargc] = input;
		args[kargc + 1] = NULL;
		
		if (verbose) {
			obggcc_print_args(args);
		}
		
		pid = fork();
		
		if (pid == 0) {
			if (execve(executable, args, environ) == -1) {
				fprintf(stderr, "fatal error: %s: %s\n", obggcc_strerror(ERR_EXECVE_FAILURE), strerror(errno));
				_exit(EXIT_FAILURE);
			}
			
			_exit(EXIT_SUCCESS);
		} else if (pid > 0) {
			wait(&wstatus);
			wstatus = WIFSIGNALED(wstatus) ? 128 + WTERMSIG(wstatus) : WEXITSTATUS(wstatus);
			
			if (wstatus != EXIT_SUCCESS) {
				err = ERR_EXECVE_SUBPROCESS_FAILURE;
				goto end;
			}
		} else {
			err = ERR_FORK_FAILURE;
			goto end;
		}
		
		#if defined(PINO)
			if (output == NULL) {
				output = input;
			}
			
			cur = basename(output);
			
			if (strcmp(cur, LIBCXX_SHARED) == 0) {
				if (verbose) {
					fprintf(stderr, "- Removing '%s'\n", output);
				}
				remove_file(output);
			}
		#endif
	}
	
	end:;
	
	free(executable);
	free(kargv);
	free(args);
	free(parent_directory);
	free(app_filename);
	
	query_free(&query);
	
	if (err != ERR_SUCCESS) {
		cur = strerror(errno);
		
		fprintf(stderr, "fatal error: %s", obggcc_strerror(err));
		
		switch (err) {
			case ERR_EXECVE_FAILURE:
			case ERR_COPY_FILE_FAILURE:
			case ERR_GET_APP_FILENAME_FAILURE:
			case ERR_MEM_ALLOC_FAILURE:
				fprintf(stderr, ": %s", cur);
				break;
			case ERR_BINFMT_GUESS_FAILURE:
				fprintf(stderr, ": %s", input);
				break;
		}
		
		fprintf(stderr, "\n");
		
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
	
}