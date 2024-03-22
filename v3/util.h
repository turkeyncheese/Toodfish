#pragma once
#include <inttypes.h>
#include <stddef.h>

uint64_t prng(uint64_t *state);
double prngf(uint64_t *state);

void hash_block(uint64_t block, uint64_t *hash);
void hash_blocks(const void *buffer, size_t length, uint64_t *hash);
