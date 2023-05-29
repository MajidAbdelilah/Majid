#include "io.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


File_S readFile(const char *file_name) {
	File_S file = {0};
	FILE *fp = fopen(file_name, "r");
	if (fp == NULL) {
		fprintf(stderr, "ERROR: file doesnt exist\n");
	}
	fseek(fp, 0L, SEEK_END);
	unsigned int size = ftell(fp);
	fprintf(stderr, "%s file size = %u\n", file_name, size);
	fseek(fp, 0L, SEEK_SET);
	char *result = malloc(size + 1);
	memset(result, 0, size + 1);
	fread(result, size, 1, fp);
	result[size] = '\0';
	file.data = result;
	file.size = size;
	return file;
}

