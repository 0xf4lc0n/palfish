#include <stdio.h>
#include <stdint.h>

uint8_t* read_entire_file(const char *file_path);
uint8_t* read_file_chunk(const char *file_path, size_t bytes_to_read);
