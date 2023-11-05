#include "io.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int read_file(FILE *file, size_t bytes_to_read, uint8_t *output) {
  size_t bytes_read = fread(output, sizeof(char), bytes_to_read, file);
  if (bytes_read != bytes_to_read) {
    // fread sets global variable errno so we can access it later via perror()
    // saying so there is no need for printing error message here
    return -1;
  }

  output[bytes_to_read] = '\0';
  return 0;
}

uint8_t *read_entire_file(const char *file_path) {
  FILE *file = fopen(file_path, "r");

  if (file == NULL) {
    perror("Failed to read file");
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  uint8_t *file_content = (uint8_t *)malloc(file_size + 1);

  if (file_content == NULL) {
    perror("Failed to allocate memory for file content");
    fclose(file);
    return NULL;
  }

  if (read_file(file, file_size, file_content) != 0) {
    perror("Failed to read requested amount of bytes from the file");
    free(file_content);
    fclose(file);
    return NULL;
  }

  fclose(file);

  return file_content;
}

uint8_t *read_file_chunk(const char *file_path, size_t bytes_to_read) {
  FILE *file = fopen(file_path, "r");

  if (file == NULL) {
    perror("Failed to read file");
    return NULL;
  }

  uint8_t *file_content = (uint8_t *)malloc(bytes_to_read + 1);

  if (file_content == NULL) {
    perror("Failed to allocate memory for file content");
    fclose(file);
    return NULL;
  }

  if (read_file(file, bytes_to_read, file_content) != 0) {
    perror("Failed to read requested amount of bytes from the file");
    free(file_content);
    fclose(file);
    return NULL;
  }

  fclose(file);

  return file_content;
}
