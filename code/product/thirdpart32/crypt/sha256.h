#ifndef _SHA256_H_
#define _SHA256_H_

typedef struct {
	uint32_t buf[16];
	uint32_t hash[8];
	uint32_t len[2];
} sha256_context;

void sha256_init(sha256_context *);
void sha256_hash(sha256_context *, uint8_t * /* data */, uint32_t /* len */);
void sha256_done(sha256_context *, uint8_t * /* hash */);



#endif

