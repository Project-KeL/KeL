#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "exl.h"
#include "key_lock.h"

extern Initializer _initializer;

bool is_valid_key(
const char* restrict try_key,
int try_key_length) {
	assert(_initializer.key_lock.check_stage == KeyLockCheckStage_NO);
	_initializer.key_lock.check_stage = KeyLockCheckStage_KEY;

	if(try_key_length == 0)
		return false;

	const char* const key_list[] = {
		"byte",
		"mov",
		"syscall"
		};

	for(size_t i = 0;
	i < sizeof(key_list) / sizeof(const char*);
	++i) {
		if(strncmp(
			try_key,
			key_list[i],
			try_key_length)
		== 0) {
			_initializer.key_lock.key_index = i;
			return true;
		}
	}

	return false;
}

bool is_the_right_lock(
const char* restrict try_lock,
size_t length) {
	assert(_initializer.key_lock.check_stage == KeyLockCheckStage_KEY);
	_initializer.key_lock.check_stage = KeyLockCheckStage_NO;
	const size_t _index = _initializer.key_lock.key_index;
	// the first lock must be "" if it is part of (see next condition)
	const char* const byte_locks[] = {
		};
	const char* const mov_locks[] = {
		"B",
		"W",
		"D",
		"Q"};
	const char* const syscall_locks[] = {
		};
	const char* const* const lock_list[] = {
		byte_locks,
		mov_locks,
		syscall_locks};
	const size_t const lock_lists_size[] = {
		0, // byte
		4, // mov
		0}; // syscall

	if(length == 0
	&& lock_lists_size[_index] != 0
	&& lock_list[_index][0] != "")
		return false;

	if(lock_lists_size[_index] == 0)
		return true;

	for(size_t i = 0;
	i < lock_lists_size[_index];
	++i) {
		if(strncmp(
			try_lock,
			lock_list[_index][i],
			length)
		== 0)
			return true;
	}

	return false;
}
