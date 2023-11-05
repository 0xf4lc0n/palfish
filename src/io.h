#include <stdint.h>
#include <stdio.h>

size_t read_entire_file(const char *file_path, char **output_buffer);
char *read_file_chunk(const char *file_path, size_t bytes_to_read);
int save_to_file(const char *file_path, char *data_to_save, size_t data_len);
