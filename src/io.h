#pragma once


typedef struct File_S {
	char *data;
	unsigned int size;
} File_S;


File_S readFile(const char *file_name);
