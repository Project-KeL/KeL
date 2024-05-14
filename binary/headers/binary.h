#ifndef BINARY_H
#define BINARY_H

#include <stdint.h>
#include "parser.h"

typedef struct {
	const char* path;
	FILE* file;
} Binary;

bool create_binary(
	const char* restrict path,
	Binary* restrict binary);
bool destroy_binary(Binary* restrict binary);
bool binary_append_byte(
	Binary* restrict binary,
	uint8_t byte);
bool binary_x64(
	Binary* restrict binary,
	const Parser* restrict parser);

#endif
