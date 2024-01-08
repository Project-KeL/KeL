#include "exl.h"

const char* keys[] = {
	"byte",
	"mov",
	"syscall"};
const char* const* locks[] = {
	// byte
	(const char*[]) {},
	// mov
	(const char*[]) {
		"B",
		"W",
		"D",
		"Q"},
	// syscall
	(const char*[]) {}};
const uint16_t locks_count[] = {
	// byte
	0,
	// mov
	4,
	// syscall
	0};
Initializer _initializer = {
	.key_lock = {
		.check_stage = KeyLockCheckStage_NO,
		.keys = keys,
		.key_count = sizeof(keys) / sizeof(const char*),
		.key_index = 0,
		.locks = locks,
		.locks_count = sizeof(locks_count) / sizeof(const uint16_t)}
};
