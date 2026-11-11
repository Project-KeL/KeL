#ifndef SOURCE_H
#define SOURCE_H

typedef struct {
	const char* path;
	char* content;
	long int length;
} Source;

static inline Source setup_source(void) {
	return (Source) {};
}
bool create_source(
	const char* path,
	Source* source);
void destroy_source(Source* source);

#endif
