#include <stddef.h>
#include <stdint.h>

// Define the Blowfish block size
#define BLOWFISH_BLOCK_SIZE 8

// Function to initialize P-boxes and S-boxes
void init_blowfish(void);

// Key Expansion function
void key_expansion(uint8_t *key, int keylen);

// F function used in the Feistel network
uint32_t F(uint32_t x);

// Encryption function
void blowfish_encrypt_block(uint32_t *L, uint32_t *R);

// Decryption function
void blowfish_decrypt_block(uint32_t *L, uint32_t *R);

// Ecrypt string
int blowfish_encrypt_string(const char *input, char *output);

// Ecrypt string using OpenMP
int blowfish_encrypt_string_openmp(const char *input, char *output);

// Decrypt string
int blowfish_decrypt_string(const char *input, size_t input_len, char *output);

// Decrypt string using OpenMP
int blowfish_decrypt_string_openmp(const char *input, size_t input_len, char *output);

void print_in_hex(const char *str);
