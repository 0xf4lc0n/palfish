#include "blowfish.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
  // Define and initialize a 128-bit key
  uint8_t key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

  // Initialize Blowfish with the key
  key_expansion(key, 16);

  // Encrypt a plaintext block
  const char *plaintext = "H";
  char encrypted[strlen(plaintext)];
  char decrypted[strlen(plaintext)];
  int err;

  memset(encrypted, 0, strlen(plaintext));
  memset(decrypted, 0, strlen(plaintext));

  uint32_t left = 0x11223344;
  uint32_t right = 0x55667788;

  printf("Plaintext: 0x%08X%08X\n", left, right);
  blowfish_encrypt_block(&left, &right);
  printf("Ciphertext: 0x%08X%08X\n", left, right);
  blowfish_decrypt_block(&left, &right);
  printf("Decrypted: 0x%08X%08X\n", left, right);

  err = blowfish_encrypt_string((const char *)plaintext, encrypted);
  if (err != 0) {
    fprintf(stderr, "Failed to encrypt string");
    return -1;
  }

  printf("Encrypted: ");
  print_in_hex(encrypted);

  err = blowfish_decrypt_string((const char *)encrypted, decrypted);
  if (err != 0) {
    fprintf(stderr, "Failed to decryop string");
    return -1;
  }

  printf("Decrypted: ");
  print_in_hex(decrypted);
  printf("%s", decrypted);

  return 0;
}
