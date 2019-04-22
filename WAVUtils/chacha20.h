#pragma once

typedef union _chacha20_ctx_t
{
    unsigned char b[64];
    unsigned int w[16];
    long long q[8];
} chacha20_ctx;


void chacha20_core(void* input, void* output);
void chacha20_setkey(chacha20_ctx* c, void* key, void* nonce);
void chacha20_encrypt(chacha20_ctx* ctx, void* in, unsigned int len);
char* to_hex(unsigned char* bin, size_t len);
void encrypt_file(const char* file_in, const char* file_out, const char key[32]);
void decrypt_file(const char* file_in, const char* file_out, const char key[32]);
