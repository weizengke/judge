#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "aes256.h"
#include "md5.h"
#include "sha256.h"
#include "base64.h"

int Encrypt_sha256(char *plaintext, char *ciphertext)
{
    sha256_context ctx;
    uint8_t hv[32];	
	//char output[64] = {0};
	
	/* do sha256 encrypt */
    sha256_init(&ctx);
    sha256_hash(&ctx, (uint8_t *)plaintext, (uint32_t)strlen(plaintext));
    sha256_done(&ctx, hv);

	/* do base64 encode */
	base64_encode((const unsigned char *)hv, sizeof(hv) - 1, ciphertext);
	
	//uint32_t i;
	//for (i = 0; i < 32; i++) printf("%02x%s", hv[i], ((i%4)==3)?" ":"");

	//printf("\r\n output=%s, len=%d", output, strlen(output));
	//unsigned char decode[64] = {0};
	//base64_decode(output, sizeof(output) - 1, decode);
	//printf("\r\n decode=%s\r\n", decode);	

	//for (i = 0; i < 32; i++) printf("%02x%s", decode[i], ((i%4)==3)?" ":"");

	return 0;
}


int crypt_test()
{

	extern int aes_test();
	aes_test();

	extern int md5_test();
	md5_test();

	extern int sha256_test();
	sha256_test();

	extern int base64_test();
	base64_test();

	char ciphertext[64] = {0};
	Encrypt_sha256("123456789", ciphertext);	
	printf("\r\nciphertext=%s, len=%u", ciphertext, strlen(ciphertext));

	memset(ciphertext, 0, sizeof(ciphertext));
	Encrypt_sha256("012345678901231111111111111", ciphertext);
	printf("\r\nciphertext=%s, len=%u", ciphertext, strlen(ciphertext));
	
	return 0;
}

