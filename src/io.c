#include "io.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int read_file(FILE *file, size_t bytes_to_read, char *output) {
  size_t bytes_read = fread(output, sizeof(char), bytes_to_read, file);
  if (bytes_read != bytes_to_read) {
    // fread sets global variable errno so we can access it later via perror()
    // saying so there is no need for printing error message here
    return -1;
  }

  output[bytes_to_read] = '\0';
  return 0;
}

size_t read_entire_file(const char *file_path, char **output_buffer) {
  FILE *file = fopen(file_path, "r");

  if (file == NULL) {
    perror("Failed to read file");
    return 0;
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  *output_buffer = (char *)malloc(file_size + 1);

  if (output_buffer == NULL) {
    perror("Failed to allocate memory for file content");
    fclose(file);
    return 0;
  }

  if (read_file(file, file_size, *output_buffer) != 0) {
    perror("Failed to read requested amount of bytes from the file");
    free(output_buffer);
    fclose(file);
    return 0;
  }

  fclose(file);

  return file_size;
}

char *read_file_chunk(const char *file_path, size_t bytes_to_read) {
  FILE *file = fopen(file_path, "r");

  if (file == NULL) {
    perror("Failed to read file");
    return NULL;
  }

  char *file_content = (char *)malloc(bytes_to_read + 1);

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

int save_to_file(const char *file_path, char *data_to_save,
                    size_t data_len) {
  FILE *file = fopen(file_path, "w");

  size_t bytes_write = fwrite(data_to_save, sizeof(char), data_len, file);

  if (bytes_write != data_len) {
    fprintf(stderr, "Failed to write requested amount of bytes to the file\n");
    return -1;
  }

  return 0;
}
