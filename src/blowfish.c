#include "blowfish.h"
#include "math.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

// Define P-boxes and S-boxes
uint32_t P[18];
uint32_t S[4][256];

// Function to initialize P-boxes and S-boxes
void init_blowfish(void) {
  // Initialize P-boxes on the hexadecimal expansion of Pi
  for (int i = 0; i < 18; i++) {
    double pi_hex = M_PI * pow(2, 32);
    P[i] = (uint32_t)(pi_hex + i);
  }

  // Initialize S-boxes based on the hexadecimal expansion of e
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 256; j++) {
      double e_hex = M_E * pow(2, 32);
      S[i][j] = (uint32_t)(e_hex + (i * 256) + j);
    }
  }
}

// Key Expansion function
void key_expansion(uint8_t *key, int keylen) {
  // Initialize P-boxes and S-boxes with pi and e
  init_blowfish();

  // XOR the P-boxes with the key
  int keyIndex = 0;
  for (int i = 0; i < 18; i++) {
    uint32_t data = 0;
    for (int j = 0; j < 4; j++) {
      data = (data << 8) | key[keyIndex];
      keyIndex = (keyIndex + 1) % keylen;
    }
    P[i] ^= data;
  }

  // Initialize L and R to zero
  uint32_t L = 0, R = 0;

  // Apply the encryption process to L and R several times
  for (int i = 0; i < 18; i += 2) {
    blowfish_encrypt_block(&L, &R);
    P[i] = L;
    P[i + 1] = R;
  }

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 256; j += 2) {
      blowfish_encrypt_block(&L, &R);
      S[i][j] = L;
      S[i][j + 1] = R;
    }
  }
}

// F function used in the Feistel network
uint32_t F(uint32_t x) {
  uint8_t b0 = (x >> 24) & 0xFF;
  uint8_t b1 = (x >> 16) & 0xFF;
  uint8_t b2 = (x >> 8) & 0xFF;
  uint8_t b3 = x & 0xFF;

  uint32_t result = ((S[0][b0] + S[1][b1]) ^ S[2][b2]) + S[3][b3];

  return result;
}

void swap(uint32_t *l, uint32_t *r) {
  uint32_t tmp = *l;
  *l = *r;
  *r = tmp;
}

// Encryption function
void blowfish_encrypt_block(uint32_t *L, uint32_t *R) {
  for (int i = 0; i < 16; i++) {
    *L ^= P[i];
    *R ^= F(*L);
    swap(L, R);
  }

  swap(L, R);
  *R ^= P[16];
  *L ^= P[17];
}

// Decryption function
void blowfish_decrypt_block(uint32_t *L, uint32_t *R) {
  // The decryption process is essentially the same as encryption, but
  // you use the subkeys in reverse order.

  for (int i = 17; i > 1; i--) {
    *L ^= P[i];
    *R ^= F(*L);
    swap(L, R);
  }

  swap(L, R);
  *R ^= P[1];
  *L ^= P[0];
}

void print_in_hex(const char *str) {
  for (unsigned long i = 0; i < strlen(str); i++) {
    printf("%02X ", (unsigned char)str[i]);
  }
  printf("\n");
}

char *add_pkcs7_padding(const char *input, int block_size) {
  // printf("Padding was used\n");
  int len = strlen(input);
  int padding_len = block_size - (len % block_size);
  int padded_input_len = len + padding_len;

  char *padded_input = (char *)malloc(padded_input_len + 1);

  if (padded_input == NULL) {
    fprintf(stderr, "Failed to allocate memory for padded input");
    return NULL;
  }

  strcpy(padded_input, input);

  for (int i = len; i < padded_input_len; i++) {
    padded_input[i] = (char)padding_len;
  }

  padded_input[padded_input_len] = '\0';

  return padded_input;
}

char *remove_pkcs7_padding(const char *input) {
  int len = strlen(input);
  int padding_len = (int)input[len - 1];

  if (padding_len > 0 && padding_len <= len) {
    for (int i = len - padding_len; i < len; i++) {
      if (input[i] != (char)padding_len) {
        return NULL;
      }
    }

    char *unpadded_input = (char *)malloc(len - padding_len + 1);

    if (unpadded_input == NULL) {
      fprintf(stderr, "Failed to allocate memory for unpadded input");
      return NULL;
    }

    int plain_text_len = len - padding_len;
    strncpy(unpadded_input, input, plain_text_len);
    memset(unpadded_input + plain_text_len, 0, padding_len);
    return unpadded_input;
  }

  return NULL;
}

int blowfish_encrypt_string(const char *input, char *output) {
  int len = strlen(input);

  if (len % BLOWFISH_BLOCK_SIZE != 0) {
    input = add_pkcs7_padding(input, BLOWFISH_BLOCK_SIZE);
    if (input == NULL) {
      fprintf(stderr, "Failed to pad input message");
      return -1;
    }
    output[strlen(input)] = '\0';
  } else {
    output[len] = '\0';
  }

  for (int i = 0; i < len; i += BLOWFISH_BLOCK_SIZE) {
    uint32_t L = 0, R = 0;
    memcpy(&L, input + i, sizeof(uint32_t));
    memcpy(&R, input + i + sizeof(uint32_t), sizeof(uint32_t));
    blowfish_encrypt_block(&L, &R);
    memcpy(output + i, &L, sizeof(uint32_t));
    memcpy(output + i + sizeof(uint32_t), &R, sizeof(uint32_t));
  }

  return 0;
}

int is_padding_present(const char *ciphertext) {
  int len = strlen(ciphertext);
  int padding_len = ciphertext[strlen(ciphertext) - 1];

  if (padding_len > BLOWFISH_BLOCK_SIZE)
    return 0;

  for (int i = len - padding_len; i < len; i++) {
    if (ciphertext[i] != padding_len) {
      return 0;
    }
  }

  return 1;
}

int blowfish_decrypt_string(const char *input, size_t input_len, char *output) {
  int len = input_len;

  for (int i = 0; i < len; i += BLOWFISH_BLOCK_SIZE) {
    uint32_t L = 0, R = 0;
    memcpy(&L, input + i, sizeof(uint32_t));
    memcpy(&R, input + i + sizeof(uint32_t), sizeof(uint32_t));
    blowfish_decrypt_block(&L, &R);
    memcpy(output + i, &L, sizeof(uint32_t));
    memcpy(output + i + sizeof(uint32_t), &R, sizeof(uint32_t));
  }

  output[len] = '\0';

  if (is_padding_present(output)) {
    char *unpadded = remove_pkcs7_padding(output);
    memcpy(output, unpadded, len);
    if (output == NULL) {
      fprintf(stderr, "Failed to unpad input message");
      return -1;
    }
  }

  return 0;
}

int blowfish_decrypt_string_openmp(const char *input, size_t input_len, char *output) {
  int len = input_len;

  #pragma omp parallel for
  for (int i = 0; i < len; i += BLOWFISH_BLOCK_SIZE) {
    uint32_t L = 0, R = 0;
    memcpy(&L, input + i, sizeof(uint32_t));
    memcpy(&R, input + i + sizeof(uint32_t), sizeof(uint32_t));
    blowfish_decrypt_block(&L, &R);
    memcpy(output + i, &L, sizeof(uint32_t));
    memcpy(output + i + sizeof(uint32_t), &R, sizeof(uint32_t));
  }

  output[len] = '\0';

  if (is_padding_present(output)) {
    char *unpadded = remove_pkcs7_padding(output);
    memcpy(output, unpadded, len);
    if (output == NULL) {
      fprintf(stderr, "Failed to unpad input message");
      return -1;
    }
  }

  return 0;
}

// int blowfish_encrypt_string_openmp(const char *input, char *output) {
//     int n = strlen(input);
//     // printf("Size: %d\n", n);

//    //int chunksize = n/2;

//     // #pragma omp parallel num_threads(2) default(none) shared(n,chunksize,input,output)
//     // {
//     //   int nthread = omp_get_thread_num();
//     //   #pragma omp for schedule(static,chunksize) 
//     //   for (int i= chunksize * nthread; i<n; i+=8) {
//     //       printf("Iter %d being done by thread %d\n", i, nthread);
//     //       blowfish_encrypt_string((char*)(input + i), (char*)(output + i));
//     //   }
//     // }
//  omp_set_num_threads(8);
//     int thread_num, num_threads, start, end, i;
//     #pragma omp parallel private(i,thread_num,num_threads,start,end)
//     {
//       thread_num = omp_get_thread_num();
//       num_threads = omp_get_num_threads();
//       start = thread_num * n / num_threads;
//       end = (thread_num + 1) * n / num_threads;

//       for (i = start; i != end; i += 8) {
//         //printf("%d %d\n", thread_num, i);
//         blowfish_encrypt_string((char*)(input + i), (char*)(output + i));
//       }
//     }

//     return 0;
// }

int blowfish_encrypt_string_openmp(const char *input, char *output) {
  int len = strlen(input);

  if (len % BLOWFISH_BLOCK_SIZE != 0) {
    input = add_pkcs7_padding(input, BLOWFISH_BLOCK_SIZE);
    if (input == NULL) {
      fprintf(stderr, "Failed to pad input message");
      return -1;
    }
    output[strlen(input)] = '\0';
  } else {
    output[len] = '\0';
  }

  #pragma omp parallel for
  for (int i = 0; i < len; i += BLOWFISH_BLOCK_SIZE) {
    uint32_t L = 0, R = 0;
    memcpy(&L, input + i, sizeof(uint32_t));
    memcpy(&R, input + i + sizeof(uint32_t), sizeof(uint32_t));

    blowfish_encrypt_block(&L, &R);

    memcpy(output + i, &L, sizeof(uint32_t));
    memcpy(output + i + sizeof(uint32_t), &R, sizeof(uint32_t));
  }
  return 0;
}
