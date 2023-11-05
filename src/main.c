#include "blowfish.h"
#include "io.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Usage: %s path_to_key_file key_len file_to_encrpyt", argv[0]);
    return 1;
  }

  char *key_file = argv[1];
  char key_size = atoi(argv[2]);
  char *file_to_encrpyt = argv[3];

  double init_time = clock();
  uint8_t *key = (uint8_t *)read_file_chunk(key_file, key_size);

  // Initialize Blowfish with the key
  key_expansion(key, 16);

  char *plaintext = NULL;
  size_t file_size = read_entire_file(file_to_encrpyt, &plaintext);

  char *encrypted = malloc(sizeof(char) * file_size);
  char *decrypted = malloc(sizeof(char) * file_size);
  int err = 0;

  double encryption_start = clock();
  err = blowfish_encrypt_string(plaintext, encrypted);
  double encryption_end = clock();
  if (err != 0) {
    fprintf(stderr, "Failed to encrypt message\n");
    return -1;
  }

  err = save_to_file("encrypted.txt", encrypted, file_size);
  if (err != 0) {
    fprintf(stderr, "Failed to save encrypted message to the file\n");
    return -1;
  }

  double decryption_start = clock();
  err = blowfish_decrypt_string(encrypted, file_size, decrypted);
  double decryption_end = clock();
  if (err != 0) {
    fprintf(stderr, "Failed to decrypt message\n");
    return -1;
  }

  err = save_to_file("decrypted.txt", decrypted, file_size);
  if (err != 0) {
    fprintf(stderr, "Failed to save decrypted message to the file\n");
    return -1;
  }

  free(encrypted);
  free(decrypted);

  double finalize_time = clock();
  double total_time = (finalize_time - init_time) / CLOCKS_PER_SEC;
  double encryption_time = (encryption_end - encryption_start) / CLOCKS_PER_SEC;
  double decryption_time = (decryption_end - decryption_start) / CLOCKS_PER_SEC;

  printf("Encryption time time: %f\n", encryption_time);
  printf("Decryption time: %f\n", decryption_time);
  printf("Total execution time: %f\n", total_time);

  return 0;
}
