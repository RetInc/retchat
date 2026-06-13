#pragma once

#include <stdint.h>
#include <stddef.h>
#include <openssl/bn.h>

#define KEY_LENGTH  32
#define MAX_MSG_LEN 4096


void init_dh_params(void);
void free_dh_params(void);

void generate_private_key(BIGNUM* priv);
void compute_public_key(const BIGNUM* priv, BIGNUM* pub);
void compute_shared_secret(const BIGNUM* peer_pub, const BIGNUM* priv, BIGNUM* secret);

void derive_enc_key(const BIGNUM* shared_secret, uint8_t out_key[KEY_LENGTH]);

void derive_keystream(uint8_t* keystream, size_t len, const uint8_t* base_key, uint64_t counter);
void xor_crypt(uint8_t* data, size_t len, const uint8_t* key, uint64_t counter);