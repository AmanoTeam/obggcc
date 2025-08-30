#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "query.h"
#include "biggestint.h"
#include "urlencode.h"
#include "urldecode.h"
#include "strsplit.h"
#include "fstream.h"

static const char AND = '&';
static const char EQUAL[] = "=";

static const char* const BOOL_VAL_TRUE[] = {
	"TRUE",
	"True",
	"true",
	"YES",
	"Yes",
	"yes",
	"1",
};

static const char* const BOOL_VAL_FALSE[] = {
	"FALSE",
	"False",
	"false",
	"NO",
	"No",
	"no",
	"0"
};

extern char** environ;

enum hquery_num {
	HQUERY_INT,
	HQUERY_UINT,
	HQUERY_FLOAT
};

static int put_parameter(
	hquery_t* const query,
	hquery_param_t* const parameter
) {
	
	size_t size = 0;
	hquery_param_t* parameters = NULL;
	
	if (sizeof(*query->parameters) * (query->offset + 1) > query->size) {
		size = query->size + sizeof(*query->parameters) * (query->offset + 1);
		parameters = realloc(query->parameters, size);
		
		if (parameters == NULL) {
			return -1;
		}
		
		query->size = size;
		query->parameters = parameters;
	}
	
	query->parameters[query->offset++] = *parameter;
	
	return 0;
	
}

hquery_param_t* query_get_param(
	hquery_t* const query,
	const char* const key
) {
	
	size_t index = 0;
	
	for (index = 0; index < query->offset; index++) {
		hquery_param_t* const parameter = &query->parameters[index];
		
		if (strcmp(parameter->key, key) != 0) {
			continue;
		}
		
		return parameter;
	}
	
	return NULL;
	
}

char* query_get_string(
	hquery_t* const query,
	const char* const key
) {
	
	const hquery_param_t* const parameter = query_get_param(query, key);
	
	if (parameter == NULL || parameter->value == NULL) {
		return NULL;
	}
	
	return parameter->value;
	
}

static int get_numeric_value(
	const enum hquery_num type,
	const char* const source,
	bigint_storage_t* destination
) {
	
	int err = 0;
	
	switch (type) {
		case HQUERY_INT: {
			const bigint_t val = strtobi(source, NULL, 10);
			
			if (errno == ERANGE) {
				err = -1;
				break;
			}
			
			memcpy(destination, &val, sizeof(val));
			
			break;
		}
		case HQUERY_UINT: {
			const biguint_t val = strtobui(source, NULL, 10);
			
			if (errno == ERANGE) {
				err = -1;
				break;
			}
			
			memcpy(destination, &val, sizeof(val));
			
			break;
		}
		case HQUERY_FLOAT: {
			const bigfloat_t val = strtobf(source, NULL);
			
			if (errno == ERANGE) {
				err = -1;
				break;
			}
			
			memcpy(destination, &val, sizeof(val));
			
			break;
		}
		default:
			break;
	}
	
	return err;
	
}

static int query_get_numeric(
	hquery_t* const query,
	const enum hquery_num type,
	const char* const key,
	bigint_storage_t* destination
) {
	
	int err = 0;
	
	const char* const source = query_get_string(query, key);
	
	if (source == NULL) {
		return -1;
	}
	
	err = get_numeric_value(type, source, destination);
	
	return err;
	
}

const char* param_get_string(const hquery_param_t* const param) {
	return param->value;
}

int param_get_bool(const hquery_param_t* const param) {
	
	size_t index = 0;
	const char* value = NULL;
	
	for (index = 0; index < sizeof(BOOL_VAL_TRUE) / sizeof(*BOOL_VAL_TRUE); index++) {
		value = BOOL_VAL_TRUE[index];
		
		if (strcmp(param->value, value) != 0) {
			continue;
		}
		
		return 1;
	}
	
	
	for (index = 0; index < sizeof(BOOL_VAL_FALSE) / sizeof(*BOOL_VAL_FALSE); index++) {
		value = BOOL_VAL_FALSE[index];
		
		if (strcmp(param->value, value) != 0) {
			continue;
		}
		
		return 0;
	}
	
	return -1;
	
}

int query_get_bool(
	hquery_t* const query,
	const char* const key
) {
	
	const hquery_param_t* const parameter = query_get_param(query, key);
	
	if (parameter == NULL || parameter->value == NULL) {
		return -1;
	}
	
	return param_get_bool(parameter);
	
}

bigint_t param_get_int(const hquery_param_t* const param) {
	
	bigint_storage_t value = {0};
	const int err = get_numeric_value(HQUERY_INT, param->value, &value);
	
	if (err != 0) {
		return BIGINT_MAX;
	}
	
	return *(bigint_t*) &value;
	
}

bigint_t query_get_int(
	hquery_t* const query,
	const char* const key
) {
	
	bigint_storage_t value = {0};
	const int err = query_get_numeric(query, HQUERY_INT, key, &value);
	
	if (err != 0) {
		return BIGINT_MAX;
	}
	
	return *(bigint_t*) &value;
	
}

biguint_t param_get_uint(const hquery_param_t* const param) {
	
	bigint_storage_t value = {0};
	const int err = get_numeric_value(HQUERY_UINT, param->value, &value);
	
	if (err != 0) {
		return BIGINT_MAX;
	}
	
	return *(biguint_t*) &value;
	
}

biguint_t query_get_uint(
	hquery_t* const query,
	const char* const key
) {
	
	bigint_storage_t value = {0};
	const int err = query_get_numeric(query, HQUERY_UINT, key, &value);
	
	if (err != 0) {
		return BIGUINT_MAX;
	}
	
	return *(biguint_t*) &value;
	
}

bigfloat_t param_get_float(const hquery_param_t* const param) {
	
	bigint_storage_t value = {0};
	const int err = get_numeric_value(HQUERY_FLOAT, param->value, &value);
	
	if (err != 0) {
		return BIGFLOAT_MAX;
	}
	
	return *(bigfloat_t*) &value;
	
}

bigfloat_t query_get_float(
	hquery_t* const query,
	const char* const key
) {
	
	bigint_storage_t value = {0};
	const int err = query_get_numeric(query, HQUERY_FLOAT, key, &value);
	
	if (err != 0) {
		return BIGFLOAT_MAX;
	}
	
	return *(bigfloat_t*) &value;
	
}

int query_add_string(
	hquery_t* const query,
	const char* const key,
	const char* const value
) {
	
	int err = 0;
	
	size_t size = 0;
	
	hquery_param_t* param = NULL;
	hquery_param_t parameter = {0};
	
	param = query_get_param(query, key);
	
	if (param != NULL) {
		if (strcmp(param->value, value) == 0) {
			goto end;
		}
		
		free(param->value);
		param->value = malloc(strlen(value) + 1);
		
		if (param->value == NULL) {
			err = -1;
			goto end;
		}
		
		strcpy(param->value, value);
		
		goto end;
	}
	
	size = strlen(key);
	
	if (size > 0) {
		parameter.key = malloc(size + 1);
		
		if (parameter.key == NULL) {
			err = -1;
			goto end;
		}
		
		strcpy(parameter.key, key);
	}
	
	size = strlen(value);
	
	if (size > 0) {
		parameter.value = malloc(size + 1);
		
		if (parameter.value == NULL) {
			err = -1;
			goto end;
		}
		
		strcpy(parameter.value, value);
	}
	
	err = put_parameter(query, &parameter);
	
	end:;
	
	if (err != 0) {
		param_free(&parameter);
	}
	
	return err;
	
}

static int query_add_numeric(
	hquery_t* const query,
	const enum hquery_num type,
	const char* const key,
	const void* const value
) {
	
	int err = 0;
	
	char wtmp[32];
	int wsize = 0;
	
	switch (type) {
		case HQUERY_INT:
			wsize = snprintf(wtmp, sizeof(wtmp), "%"FORMAT_BIGGEST_INT_T, *(bigint_t*) value);
			break;
		case HQUERY_UINT:
			wsize = snprintf(wtmp, sizeof(wtmp), "%"FORMAT_BIGGEST_UINT_T, *(biguint_t*) value);
			break;
		case HQUERY_FLOAT:
			wsize = snprintf(wtmp, sizeof(wtmp), "%"FORMAT_BIGGEST_FLOAT_T, *(bigfloat_t*) value);
			break;
		default:
			break;
	}
	
	if (wsize < 1) {
		err = -1;
		goto end;
	}
	
	err = query_add_string(query, key, wtmp);
	
	end:;
	
	return err;
	
}

int query_add_int(
	hquery_t* const query,
	const char* const key,
	const bigint_t value
) {
	
	const int err = query_add_numeric(query, HQUERY_INT, key, &value);
	return err;
	
}

int query_add_uint(
	hquery_t* const query,
	const char* const key,
	const biguint_t value
) {
	
	const int err = query_add_numeric(query, HQUERY_UINT, key, &value);
	return err;
	
}

int query_add_float(
	hquery_t* const query,
	const char* const key,
	const bigfloat_t value
) {
	
	const int err = query_add_numeric(query, HQUERY_FLOAT, key, &value);
	return err;
	
}

size_t query_dump_string(
	const hquery_t* const query,
	char* const destination
) {
	
	ssize_t size = 0;
	size_t index = 0;
	
	const hquery_param_t* parameter = NULL;
	
	char* end = destination;
	
	const char separator[] = {query->sep, '\0'};
	
	if (destination != NULL) {
		*destination = '\0';
	}
	
	size++;
	
	for (index = 0; index < query->offset; index++) {
		parameter = &query->parameters[index];
		
		if (index != 0) {
			if (destination != NULL) {
				strcat(destination, separator);
			}
			
			size += strlen(separator);
		}
		
		if (parameter->key != NULL) {
			if (destination != NULL) {
				end = strchr(end, '\0');
				urlencode(parameter->key, end);
			}
			
			size += urlencode(parameter->key, NULL) - 1;
		}
		
		if (destination != NULL) {
			strcat(destination, query->subsep);
		}
		
		size += strlen(query->subsep);
		
		if (parameter->value != NULL) {
			if (destination != NULL) {
				end = strchr(end, '\0');
				urlencode(parameter->value, end);
			}
			
			size += urlencode(parameter->value, NULL) - 1;
		}
	}
	
	return size;
	
}

void param_free(hquery_param_t* const parameter) {
	
	free(parameter->key);
	parameter->key = NULL;
	
	free(parameter->value);
	parameter->value = NULL;
	
}

void query_free(hquery_t* const query) {
	
	size_t index = 0;
	
	for (index = 0; index < query->offset; index++) {
		hquery_param_t* const parameter = &query->parameters[index];
		param_free(parameter);
	}
	
	query->size = 0;
	query->offset = 0;
	
	free(query->parameters);
	query->parameters = NULL;
	
}

int query_load_string(
	hquery_t* const query,
	const char* const string
) {
	
	int err = 0;
	
	strsplit_t split = {0};
	strsplit_part_t part = {0};
	
	strsplit_t subsplit = {0};
	strsplit_part_t subpart = {0};
	
	const char* end = NULL;
	const char* match = NULL;
	
	const char separator[] = {query->sep, '\0'};
	
	hquery_param_t param = {0};
	
	strsplit_init(&split, &part, string, separator);
	
	while (strsplit_next(&split, &part) != NULL) {
		if (part.size == 0) {
			continue;
		}
		
		strsplit_init(&subsplit, &subpart, part.begin, query->subsep);
		
		/* Parse parameter name */
		strsplit_next(&subsplit, &subpart);
		
		if (subpart.begin == NULL) {
			continue;
		}
		
		strsplit_resize(&split, &subpart);
		
		if (subpart.size == 0) {
			continue;
		}
		
		param.key = NULL;
		param.value = NULL;
		
		param.key = malloc(subpart.size + 1);
		
		if (param.key == NULL) {
			err = -1;
			break;
		}
		
		memcpy(param.key, subpart.begin, subpart.size);
		param.key[subpart.size] = '\0';
		
		urldecode(param.key, param.key);
		
		/* Parse parameter value */
		strsplit_next(&subsplit, &subpart);
		
		if (subpart.begin == NULL) {
			param_free(&param);
			continue;
		}
		
		strsplit_resize(&split, &subpart);
		
		if (subpart.size == 0) {
			param_free(&param);
			continue;
		}
		
		end = subpart.begin + subpart.size;
		
		if (strncmp(end, query->subsep, strlen(query->subsep)) == 0) {
			match = strchr(end, query->sep);
			
			if (match != NULL) {
				subpart.size += (size_t) (match - end);
			}
		}
		
		param.value = malloc(subpart.size + 1);
		
		if (param.value == NULL) {
			err = -1;
			break;
		}
		
		memcpy(param.value, subpart.begin, subpart.size);
		param.value[subpart.size] = '\0';
		
		urldecode(param.value, param.value);
		
		err = put_parameter(query, &param);
		
		if (err != 0) {
			break;
		}
	}
	
	if (err != 0) {
		param_free(&param);
		query_free(query);
	}
	
	return err;
	
}

int query_load_environ(hquery_t* const query) {
	
	int err = 0;
	
	char* string = NULL;
	const char* item = NULL;
	
	size_t index = 0;
	size_t size = 0;
	
	char separator[] = {'\0', '\0'};
	
	if (environ == NULL) {
		err = -1;
		goto end;
	}
	
	query_init(query, 0, NULL);
	
	separator[0] = query->sep;
	
	while ((item = environ[index++]) != NULL) {
		size += strlen(item) + strlen(separator);
	}
	
	string = malloc(size + 1);
	
	if (string == NULL) {
		err = -1;
		goto end;
	}
	
	string[0] = '\0';
	
	index = 0;
	
	while ((item = environ[index++]) != NULL) {
		if (string[0] != '\0') {
			strcat(string, separator);
		}
		
		strcat(string, item);
	}
	
	err = query_load_string(query, string);
	
	end:;
	
	free(string);
	
	return err;
	
}

void query_init(
	hquery_t* const query,
	const char sep,
	const char* subsep
) {
	
	query_free(query);
	
	query->sep = sep;
	
	if (query->sep == '\0') {
		query->sep = AND;
	}
	
	query->subsep = subsep;
	
	if (query->subsep == NULL) {
		query->subsep = EQUAL;
	}
	
}

int query_load_file(
	hquery_t* const query,
	const char* const filename
) {
	
	int err = 0;
	
	fstream_t* stream = NULL;
	char* buffer = NULL;
	
	long int file_size = 0;
	ssize_t rsize = 0;
	
	stream = fstream_open(filename, FSTREAM_READ);
	
	if (stream == NULL) {
		err = -1;
		goto end;
	}
	
	err = fstream_seek(stream, 0, FSTREAM_SEEK_END);
	
	if (err == -1) {
		err = -1;
		goto end;
	}
	
	file_size = fstream_tell(stream);
	
	if (file_size == -1) {
		err = -1;
		goto end;
	}
	
	if (file_size == 0) {
		err = -1;
		goto end;
	}
	
	err = fstream_seek(stream, 0, FSTREAM_SEEK_BEGIN);
	
	if (err == -1) {
		err = -1;
		goto end;
	}
	
	buffer = malloc((size_t) file_size + 1);
	
	if (buffer == NULL) {
		err = -1;
		goto end;
	}
	
	rsize = fstream_read(stream, buffer, (size_t) file_size);
	
	if (rsize == -1) {
		err = -1;
		goto end;
	}
	
	buffer[(size_t) file_size] = '\0';
	
	err = query_load_string(query, buffer);
	
	if (err != 0) {
		err = -1;
		goto end;
	}
	
	end:;
	
	free(buffer);
	
	if (err != 0) {
		query_free(query);
	}
	
	fstream_close(stream);
	
	return err;
	
}

int query_dump_file(
	hquery_t* const query,
	const char* const filename
) {
	
	int err = 0;
	
	size_t size = 0;
	
	fstream_t* stream = NULL;
	char* buffer = NULL;
	
	stream = fstream_open(filename, FSTREAM_WRITE);
	
	if (stream == NULL) {
		err = -1;
		goto end;
	}
	
	size = query_dump_string(query, NULL);
	
	buffer = malloc(size);
	
	if (buffer == NULL) {
		err = -1;
		goto end;
	}
	
	query_dump_string(query, buffer);
	
	err = fstream_write(stream, buffer, strlen(buffer));
	
	if (err != FSTREAM_SUCCESS) {
		err = -1;
		goto end;
	}
	
	end:;
	
	fstream_close(stream);
	free(buffer);
	
	return err;
	
}