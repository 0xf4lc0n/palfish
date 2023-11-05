#include "blowfish.h"
#include "io.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Usage: %s path_to_key_file key_len path_to_file_with_messages",
           argv[0]);
    return 1;
  }

  char *key_file = argv[1];
  char key_size = atoi(argv[2]);
  char *messages_file = argv[3];

  uint8_t *key = read_file_chunk(key_file, key_size);

  // Initialize Blowfish with the key
  key_expansion(key, 16);

  uint8_t *messages = read_entire_file(messages_file);

  char *eol = strchr((const char *)messages, '\n');
  char *line_start = (char *)messages;
  int err = 0;
  int line_len = 0;
  char *line = NULL;
  char *encrypted = NULL;
  char *decrypted = NULL;

  while (eol != NULL) {
    line_len = eol - line_start;
    line = malloc(sizeof(char) * line_len + 1);
    strncpy(line, line_start, line_len);

    encrypted = malloc(sizeof(char) * line_len + 1);
    decrypted = malloc(sizeof(char) * line_len + 1);

    printf("Plaintext %s\n", line);

    err = blowfish_encrypt_string(line, encrypted);
    if (err != 0) {
      fprintf(stderr, "Failed to encrypt string");
      return -1;
    }

    printf("[HEX] Encrypted: ");
    print_in_hex(encrypted);

    err = blowfish_decrypt_string((const char *)encrypted, decrypted);
    if (err != 0) {
      fprintf(stderr, "Failed to decryop string");
      return -1;
    }

    printf("Decrypted: %s\n", decrypted);
    printf("[HEX] Decrypted: ");
    print_in_hex(decrypted);

    free(line);
    free(encrypted);
    free(decrypted);

    line_start += line_len + 1;
    eol = strchr(line_start, '\n');
  }

  return 0;
}
