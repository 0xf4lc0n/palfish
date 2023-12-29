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
void blowfish_encrypt_string(const char *input, int input_len, char *output);

// Ecrypt string using OpenMP
void blowfish_encrypt_string_openmp(const char *input, int input_len, char *output);

// Decrypt string
void blowfish_decrypt_string(const char *input, int input_len, char *output);

// Decrypt string using OpenMP
void blowfish_decrypt_string_openmp(const char *input, int input_len, char *output);

// Check if padding is needed
int is_padding_needed(const char * input);

// Pad data
char * add_padding(char * input, int * padded_input_size);

// Print string in HEX representation
void print_in_hex(const char *str, int size);
