#include "blowfish.h"
#include "io.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#define ENCRYPTED_FILE_NAME "tmp/encrypted.txt"
#define DECRYPTED_FILE_NAME "tmp/decrypted.txt"
#define ENCRYPTED_FILE_NAME_OMP "tmp/encrypted_omp.txt"
#define DECRYPTED_FILE_NAME_OMP "tmp/decrypted_omp.txt"

int test_impl(char * plaintext, size_t file_size, const char * encrypt_filename, const char * decrypt_filename, void (*encrypt)(const char*, int, char*), void (*decrypt)(const char*, int, char*), double (*get_time)(void), double (*calc_duration)(double, double)) {
  int len = file_size;
  if (is_padding_needed(file_size)) {
    plaintext = add_padding(plaintext, file_size, &len);
  }

  char *encrypted = malloc(sizeof(char) * len);
  char *decrypted = malloc(sizeof(char) * len);
  int err = 0;

  double encryption_start = get_time();
  encrypt(plaintext, len, encrypted);
  double encryption_end = get_time();

  err = save_to_file(encrypt_filename, encrypted, file_size);
  if (err != 0) {
    fprintf(stderr, "Failed to save encrypted message to the file\n");
    return -1;
  }

  double decryption_start = get_time();
  decrypt(encrypted, len, decrypted);
  double decryption_end = get_time();

  err = save_to_file(decrypt_filename, decrypted, file_size);
  if (err != 0) {
    fprintf(stderr, "Failed to save decrypted message to the file\n");
    return -1;
  }

  free(encrypted);
  free(decrypted);

  printf("Encryption time: %f seconds\n", calc_duration(encryption_start, encryption_end));
  printf("Decryption time: %f seconds\n", calc_duration(decryption_start, decryption_end));

  return 0;
}

double get_time() {
  return clock();
}

double calc_duration_omp(double start, double end) {
  return end - start;
}

double calc_duration(double start, double end) {
  return (end - start) / CLOCKS_PER_SEC;
}

int test_impl_seq(char *plaintext, size_t file_size) {
  return test_impl(plaintext, file_size, ENCRYPTED_FILE_NAME, DECRYPTED_FILE_NAME,
    blowfish_encrypt_string, blowfish_decrypt_string, get_time, calc_duration);
}

int test_impl_omp(char *plaintext, size_t file_size) {
  return test_impl(plaintext, file_size, ENCRYPTED_FILE_NAME_OMP, DECRYPTED_FILE_NAME_OMP,
    blowfish_encrypt_string_openmp, blowfish_decrypt_string_openmp, omp_get_wtime, calc_duration_omp);
}

int main(int argc, char *argv[]) {
  if (argc != 6) {
    printf("Usage: %s key_file key_len file_to_encrpyt mode omp_num_threads\n", argv[0]);
    return 1;
  }

  char *key_file = argv[1];
  char key_size = atoi(argv[2]);
  char *file_to_encrpyt = argv[3];
  int mode = atoi(argv[4]);
  int omp_num_threads = atoi(argv[5]);

  uint8_t *key = (uint8_t *)read_file_chunk(key_file, key_size);

  // Initialize Blowfish with the key
  key_expansion(key, 16);

  char *plaintext = NULL;
  size_t file_size = read_entire_file(file_to_encrpyt, &plaintext);

  switch (mode) {
    case 0:
      printf("--- Sequential implementation ---\n");
      test_impl_seq(plaintext, file_size);
      break;
    case 1:
      omp_set_dynamic(0);
      omp_set_num_threads(omp_num_threads);
      printf("--- OpenMP implementation ---\n");
      test_impl_omp(plaintext, file_size);
      break;
    
    default:
      printf("--- OpenMP implementation ---\n");
      test_impl_omp(plaintext, file_size);
      printf("--- Sequential implementation ---\n");
      test_impl_seq(plaintext, file_size);
      break;
  }

  free(plaintext);
  return 0;
}
