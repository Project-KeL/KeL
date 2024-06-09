#ifndef SOURCE_H
#define SOURCE_H

typedef struct {
	const char* path;
	char* content;
	long int length;
} Source;

void initialize_source(Source* source);
bool create_source(
	const char* path,
	Source* source);
void destroy_source(Source* source);

#endif
