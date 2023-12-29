#include "blowfish.h"
#include "math.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void print_in_hex(const char *str, int size) {
  for (int i = 0; i < size; i++) {
    printf("%02X ", (unsigned char)str[i]);
  }
  printf("\n");
}

void blowfish_encrypt_string(const char *input, int input_len, char *output) {
  for (int i = 0; i < input_len; i += BLOWFISH_BLOCK_SIZE) {
    uint32_t L = 0, R = 0;
    memcpy(&L, input + i, sizeof(uint32_t));
    memcpy(&R, input + i + sizeof(uint32_t), sizeof(uint32_t));
    blowfish_encrypt_block(&L, &R);
    memcpy(output + i, &L, sizeof(uint32_t));
    memcpy(output + i + sizeof(uint32_t), &R, sizeof(uint32_t));
  }
}

void blowfish_decrypt_string(const char *input, int input_len, char *output) {
  for (int i = 0; i < input_len; i += BLOWFISH_BLOCK_SIZE) {
    uint32_t L = 0, R = 0;
    memcpy(&L, input + i, sizeof(uint32_t));
    memcpy(&R, input + i + sizeof(uint32_t), sizeof(uint32_t));
    blowfish_decrypt_block(&L, &R);
    memcpy(output + i, &L, sizeof(uint32_t));
    memcpy(output + i + sizeof(uint32_t), &R, sizeof(uint32_t));
  }
}

void blowfish_encrypt_string_openmp(const char *input, int input_len, char *output) {
  #pragma omp parallel for
  for (int i = 0; i < input_len; i += BLOWFISH_BLOCK_SIZE) {
    uint32_t L = 0, R = 0;
    memcpy(&L, input + i, sizeof(uint32_t));
    memcpy(&R, input + i + sizeof(uint32_t), sizeof(uint32_t));
    blowfish_encrypt_block(&L, &R);
    memcpy(output + i, &L, sizeof(uint32_t));
    memcpy(output + i + sizeof(uint32_t), &R, sizeof(uint32_t));
  }
}

void blowfish_decrypt_string_openmp(const char *input, int input_len, char *output) {
  #pragma omp parallel for
  for (int i = 0; i < input_len; i += BLOWFISH_BLOCK_SIZE) {
    uint32_t L = 0, R = 0;
    memcpy(&L, input + i, sizeof(uint32_t));
    memcpy(&R, input + i + sizeof(uint32_t), sizeof(uint32_t));
    blowfish_decrypt_block(&L, &R);
    memcpy(output + i, &L, sizeof(uint32_t));
    memcpy(output + i + sizeof(uint32_t), &R, sizeof(uint32_t));
  }
}

int is_padding_needed(const char * input) {
  return strlen(input) % BLOWFISH_BLOCK_SIZE != 0;
}

// Pad data
char * add_padding(char * input, int * padded_input_size) {
  int input_size = strlen(input);
  *padded_input_size = input_size + BLOWFISH_BLOCK_SIZE - (input_size % BLOWFISH_BLOCK_SIZE);
  char * padded_input = calloc(*padded_input_size, sizeof(char));
  memcpy(padded_input, input, input_size);
  return padded_input;
}
