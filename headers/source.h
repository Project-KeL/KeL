#ifndef SOURCE_H
#define SOURCE_H

typedef struct {
	const char* path;
	char* content;
	long int length;
} Source;

void initialize_source(Source* restrict source);
bool create_source(
	const char* restrict path,
	Source* restrict source);
void destroy_source(Source* restrict source);

#endif
