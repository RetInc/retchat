#define _GNU_SOURCE

#include "dh.h"
#include <string.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>

static BIGNUM* dh_prime = NULL;
static BIGNUM* dh_generator = NULL;

void init_dh_params(void) {
    const char* prime_hex =
        "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD1"
        "29024E088A67CC74020BBEA63B139B22514A08798E3404DD"
        "EF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245"
        "E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED"
        "EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3D"
        "C2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F"
        "83655D23DCA3AD961C62F356208552BB9ED529077096966D"
        "670C354E4ABC9804F1746C08CA18217C32905E462E36CE3B"
        "E39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9"
        "DE2BCBF6955817183995497CEA956AE515D2261898FA0510"
        "15728E5A8AACAA68FFFFFFFFFFFFFFFF";

    dh_prime = BN_new();
    if (!dh_prime) {
        fprintf(stderr, "error: BN_new failed for dh_prime\n");
        exit(EXIT_FAILURE);
    }
    if (BN_hex2bn(&dh_prime, prime_hex) == 0) {
        fprintf(stderr, "error: invalid DH prime hex\n");
        exit(EXIT_FAILURE);
    }

    dh_generator = BN_new();
    if (!dh_generator) {
        fprintf(stderr, "error: BN_new failed for dh_generator\n");
        exit(EXIT_FAILURE);
    }
    BN_set_word(dh_generator, 2);
}

void free_dh_params(void) {
    BN_free(dh_prime);
    BN_free(dh_generator);
}

void generate_private_key(BIGNUM* priv) {
    do {
        BN_rand(priv, 256, BN_RAND_TOP_ONE, BN_RAND_BOTTOM_ANY);
    } while (BN_is_zero(priv));
}

void compute_public_key(const BIGNUM* priv, BIGNUM* pub) {
    BN_CTX* ctx = BN_CTX_new();
    if (!ctx) {
        fprintf(stderr, "error: BN_CTX_new failed\n");
        exit(EXIT_FAILURE);
    }
    BN_mod_exp(pub, dh_generator, priv, dh_prime, ctx);
    BN_CTX_free(ctx);
}

void compute_shared_secret(const BIGNUM* peer_pub, const BIGNUM* priv, BIGNUM* secret) {
    BN_CTX* ctx = BN_CTX_new();
    if (!ctx) {
        fprintf(stderr, "error: BN_CTX_new failed\n");
        exit(EXIT_FAILURE);
    }
    BN_mod_exp(secret, peer_pub, priv, dh_prime, ctx);
    BN_CTX_free(ctx);
}

void derive_enc_key(const BIGNUM* shared_secret, uint8_t out_key[KEY_LENGTH]) {
    int secret_len = BN_num_bytes(shared_secret);
    uint8_t* secret_bin = malloc(secret_len);
    if (!secret_bin) {
        fprintf(stderr, "error: malloc failed\n");
        exit(EXIT_FAILURE);
    }
    BN_bn2bin(shared_secret, secret_bin);
    
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, secret_bin, secret_len);
    SHA256_Final(out_key, &ctx);
    
    free(secret_bin);
}

void derive_keystream(uint8_t* keystream, size_t len, const uint8_t* base_key, uint64_t counter) {
    uint8_t counter_bytes[8];
    for (int i = 0; i < 8; i++) {
        counter_bytes[i] = (counter >> (i * 8)) & 0xFF;
    }

    uint8_t digest[32];
    unsigned int digest_len;
    HMAC_CTX* hmac = HMAC_CTX_new();
    HMAC_Init_ex(hmac, base_key, KEY_LENGTH, EVP_sha256(), NULL);
    HMAC_Update(hmac, counter_bytes, 8);
    HMAC_Final(hmac, digest, &digest_len);
    HMAC_CTX_free(hmac);

    size_t copied = 0;
    while (copied < len) {
        size_t chunk = (len - copied) < 32 ? (len - copied) : 32;
        memcpy(keystream + copied, digest, chunk);
        copied += chunk;
        if (copied < len) {
            HMAC_CTX* hmac2 = HMAC_CTX_new();
            HMAC_Init_ex(hmac2, base_key, KEY_LENGTH, EVP_sha256(), NULL);
            HMAC_Update(hmac2, digest, 32);
            HMAC_Final(hmac2, digest, &digest_len);
            HMAC_CTX_free(hmac2);
        }
    }
}

void xor_crypt(uint8_t* data, size_t len, const uint8_t* key, uint64_t counter) {
    uint8_t keystream[MAX_MSG_LEN];
    derive_keystream(keystream, len, key, counter);
    for (size_t i = 0; i < len; i++) {
        data[i] ^= keystream[i];
    }
}