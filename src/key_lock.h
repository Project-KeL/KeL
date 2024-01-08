#ifndef KEY_LOCK_H
#define KEY_LOCK_H

#include <stdint.h>

typedef enum: uint8_t {
#define KEY_LOCK_CHECK_STAGE(stage) KeyLockCheckStage_ ## stage
	KEY_LOCK_CHECK_STAGE(NO),
	KEY_LOCK_CHECK_STAGE(KEY),
	KEY_LOCK_CHECK_STAGE(LOCK),
#undef KEY_LOCK_CHECK_STAGE
} KeyLockCheckStage;

bool is_valid_key(
	const char* restrict try_key,
	int try_key_length);
bool is_the_right_lock(
	const char* restrict try_lock,
	size_t length);

#endif
