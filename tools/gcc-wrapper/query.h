#if !defined(QUERY_H)
#define QUERY_H

#include "biggestint.h"

struct hquery_param {
	char* key;
	char* value;
};

typedef struct hquery_param hquery_param_t;

struct hquery {
	size_t size;
	size_t offset;
	hquery_param_t* parameters;
	char sep;
	const char* subsep;
};

typedef struct hquery hquery_t;

void query_free(hquery_t* const query);

int query_add_string(
	hquery_t* const query,
	const char* const key,
	const char* const value
);

int query_add_int(
	hquery_t* const query,
	const char* const key,
	const bigint_t value
);

int query_add_uint(
	hquery_t* const query,
	const char* const key,
	const biguint_t value
);

int query_add_float(
	hquery_t* const query,
	const char* const key,
	const bigfloat_t value
);

char* query_get_string(
	hquery_t* const query,
	const char* const key
);

const char* param_get_string(const hquery_param_t* const param);

bigint_t query_get_int(
	hquery_t* const query,
	const char* const key
);

bigint_t param_get_int(const hquery_param_t* const param);

biguint_t query_get_uint(
	hquery_t* const query,
	const char* const key
);

biguint_t param_get_uint(const hquery_param_t* const param);

bigfloat_t query_get_float(
	hquery_t* const query,
	const char* const key
);

bigfloat_t param_get_float(const hquery_param_t* const param);

int query_get_bool(
	hquery_t* const query,
	const char* const key
);

int param_get_bool(const hquery_param_t* const param);

void query_init(
	hquery_t* const query,
	const char sep,
	const char* subsep
);

int query_load_string(
	hquery_t* const query,
	const char* const string
);

int query_load_file(
	hquery_t* const query,
	const char* const filename
);

int query_load_environ(hquery_t* const query);

void param_free(hquery_param_t* const param);

size_t query_dump_string(
	const hquery_t* const query,
	char* const destination
);

int query_dump_file(
	hquery_t* const query,
	const char* const filename
);

#endif
