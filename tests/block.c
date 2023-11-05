#include "assert.h"
#include "../src/blowfish.h"
#include "../src/io.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  uint8_t *key = read_file_chunk("key.txt", 16);
  key_expansion(key, 16);

  const uint32_t BASE_L = 0x11223344;
  const uint32_t BASE_R = 0x55667788;

  uint32_t left = 0x11223344;
  uint32_t right = 0x55667788;

  blowfish_encrypt_block(&left, &right);
  blowfish_decrypt_block(&left, &right);

  assert(left == BASE_L);
  assert(right == BASE_R);

  printf("[TEST] Block operation tested successfully");

  return 0;
}
