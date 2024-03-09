#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "binary.h"

bool create_binary(
const char* restrict path,
Binary* restrict binary) {
	binary->path = NULL;

	binary->file = fopen(
		path,
		"w");

	if(binary->file == NULL) {
		printf("Cannot create the binary file.");
		destroy_binary(binary);
		return false;
	}

	binary->path = path;
	return true;
}

bool destroy_binary(Binary* binary) {
	if(binary->file != NULL
	&& fclose(binary->file) == EOF) {
		printf("Cannot close the binary.");
		return false;
	}

	binary->path = NULL;
	binary->file = NULL;
}

bool binary_append_byte(
Binary* restrict binary,
uint8_t byte) {
	return fwrite(
		&byte,
		1,
		1,
		binary->file) == 1;
}

bool binary_append_little_endian_word(
Binary* restrict binary,
uint16_t byte) {
	return fwrite(
		&byte,
		2,
		1,
		binary->file) == 1;
}

bool binary_append_big_endian_word(
Binary* restrict binary,
uint16_t byte) {
	return fwrite(
		&byte,
		1,
		2,
		binary->file) == 1;
}

bool binary_x64(
Binary* restrict binary,
const Parser* restrict parser) {
	for(long int i = 0;
	i < parser->count - 1;
	++i) {
#define APPEND_BYTE(byte) binary_append_byte( \
	binary, \
	byte)
#define APPEND_WORD(word) binary_append_big_endian_word( \
	binary, \
	word)
		const Node* restrict const node = &parser->nodes[i];

		if(node->type == NodeType_CORE_B) {
			APPEND_BYTE(node->value);
		}
#undef APPEND_WORD
#undef APPEND_BYTE
	}
}
