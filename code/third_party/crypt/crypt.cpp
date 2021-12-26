#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h> 

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
	//base64_encode((const unsigned char *)hv, sizeof(hv) - 1, ciphertext);
	
	//uint32_t i;
	//for (i = 0; i < 32; i++) printf("%02x%s", hv[i], ((i%4)==3)?" ":"");

	//printf("\r\n output=%s, len=%d", output, strlen(output));
	//unsigned char decode[64] = {0};
	//base64_decode(output, sizeof(output) - 1, decode);
	//printf("\r\n decode=%s\r\n", decode);	

	//for (i = 0; i < 32; i++) printf("%02x%s", decode[i], ((i%4)==3)?" ":"");

	return 0;
}

#include<openssl/aes.h>
#include<openssl/rsa.h>
#include<openssl/pem.h>
#include<openssl/err.h>
#include <openssl/bn.h>

void bytes2HexStr(unsigned char *src, int srcLen, char *des)
{
	char *res;
	int i=0;
 
	res = des;
	while(srcLen > 0)
	{
		sprintf((char*)(res+i*2),"%02X",*(src+i));
		i++;
		srcLen--;
	}
}

int hex2byte(char *src, unsigned char *dst) {
    while(*src) {
        if(' ' == *src) {
            src++;
            continue;
        }
        sscanf(src, "%02X", dst);
        src += 2;
        dst++;
    }
    return 0;
}

void print_show(char *name, unsigned char *src, int len)
{
	char res[1024] = {0};
	bytes2HexStr(src, len, res);
	printf("%s: %s\n", name, res);
}

void rand_iv(unsigned char *iv, int len)
{
	int i = 0;

	BIGNUM *bn;
	bn = BN_new();
	int bits = 8 * len;
	int top = -1;
	int bottom = 1;

	BN_rand(bn, bits, top, bottom);

	char *a = BN_bn2hex(bn);
	hex2byte(a, iv);

	BN_free(bn);
}

#if 1
int openssl_aes_test()
{
	unsigned char key[16] = "1234567890";
	unsigned char iv[16] = {0};
	unsigned char iv_copy[16];

	rand_iv(iv, sizeof(iv));
	print_show("IV", iv, sizeof(iv));

	unsigned char iv_encrypt_base64[640] = {0};
	unsigned char iv_decrypt_base64[640] = {0};
	int len_iv_iv_decrypt_base64 = 640;

	base64_encode(iv, sizeof(iv), iv_encrypt_base64);
	len_iv_iv_decrypt_base64 = base64_decode(iv_encrypt_base64, iv_decrypt_base64);
	//base64_encode((const unsigned char *)iv, sizeof(iv), iv_encrypt_base64);
	//base64_decode(iv_encrypt_base64, sizeof(iv_encrypt_base64), iv_decrypt_base64);
	printf("iv_encrypt_base64:%s, %d, %d\n", iv_encrypt_base64, strlen((char*)iv_encrypt_base64));
	printf("iv_decrypt_base64:%s, %d\n",  iv_decrypt_base64, len_iv_iv_decrypt_base64);
	print_show("IV", iv_decrypt_base64, len_iv_iv_decrypt_base64);

	/* base64: 123456,          MTIzNDU2AAAAAAAAAAAA 
		       123456789012345, MTIzNDU2Nzg5MDEyMzQ1

YkDv+Ogf5q9fhtKrt+RS1NppmEt01g4oWm4k0D1GlejFwG9io8DMPDK3B3BcgH7jevD3p7cvAlSJT2OVgxtE
	*/

	char buf_normal[32] = "123456789012345678901234567890";
	unsigned char buf_encrypt[256] = "";
	unsigned char buf_encrypt_base64[256] = "";
	unsigned char buf_decrypt_base64[256] = "";
	int len_buf_decrypt_base64 = 32;
	AES_KEY aesKey;

	//加密，密文长度(明文长度 +1) / 16 * 16
	int len = ceil((strlen(buf_normal) + 1.0)/16) * 16;
	memcpy(iv_copy, iv, 16);
	AES_set_encrypt_key(key, 128, &aesKey);
	AES_cbc_encrypt((unsigned char *)buf_normal, buf_encrypt, sizeof(buf_normal), &aesKey, iv_copy, 1);
	
	printf("plaintext:%s\n", buf_normal);
	printf("ciphertext:%s, %d, %d\n", buf_encrypt, strlen((char *)buf_encrypt), len);
	print_show("buf_encrypt", buf_encrypt, len);
	base64_encode((unsigned char *)buf_encrypt, len, buf_encrypt_base64);
	printf("buf_encrypt_base64:%s, %d\n", buf_encrypt_base64, strlen((char*)buf_encrypt_base64));

	//解密
	len_buf_decrypt_base64 = base64_decode(buf_encrypt_base64, buf_decrypt_base64);
	printf("buf_decrypt_base64:%s, len=%d\n", buf_decrypt_base64, len_buf_decrypt_base64);
	print_show("buf_decrypt_base64", buf_decrypt_base64, len_buf_decrypt_base64);
	memcpy(iv_copy, iv, 16);
	AES_set_decrypt_key(key, 128, &aesKey);

	memset(buf_normal, 0, sizeof(buf_normal));
	AES_cbc_encrypt(buf_decrypt_base64, (unsigned char *)buf_normal, sizeof(buf_decrypt_base64), &aesKey, iv_copy, 0);
	printf("decrypt::%s\n", buf_normal);

	return 0;
}
#endif

int util_crypt_aes128(char *plaintext, char *ciphertext)
{
	AES_KEY aesKey;
	int bits = 16 * 8;
	unsigned char key[16] = "1234567890";
	unsigned char iv[16] = {0};
	unsigned char iv_encrypt_base64[256] = {0};
	unsigned char iv_decrypt_base64[256] = {0};

	rand_iv(iv, sizeof(iv));
	//print_show("iv", iv, 16);
	base64_encode(iv, sizeof(iv), iv_encrypt_base64); /* 编码后长度⌈n/3⌉*4, ⌈⌉ 代表上取整 */

	unsigned char buf_encrypt[256] = {0};
	unsigned char buf_encrypt_base64[256] = {0};
	int len = 0;
	if (strlen(plaintext) <= 16) {
		len = 16;
	} else if (strlen(plaintext) <= 32) {
		len = 32;
	} else {
		return 0;
	}
	 
	print_show("plaintext", (unsigned char *)plaintext, strlen(plaintext));

	AES_set_encrypt_key(key, bits, &aesKey);
	AES_cbc_encrypt((unsigned char *)plaintext, buf_encrypt, strlen(plaintext), &aesKey, iv, 1);

	printf("len=%d\n", len);
	print_show("buf_encrypt", buf_encrypt, len);
	base64_encode(buf_encrypt, len, buf_encrypt_base64);

	//char ciphertext_temp[256] = {0};
	sprintf(ciphertext, "%s%s", iv_encrypt_base64, buf_encrypt_base64);
	//base64_encode((unsigned char*)ciphertext_temp, strlen(ciphertext_temp), (unsigned char*)ciphertext);

	return strlen(ciphertext);
}


int util_decrypt_aes128(char *ciphertext, char *plaintext)
{
	AES_KEY aesKey;
	int bits = 16 * 8;
	unsigned char key[16] = "1234567890";
	unsigned char iv[16] = {0};
	unsigned char iv_encrypt_base64[256] = {0};

	//unsigned char ciphertext_temp[256] = {0};	
	//base64_decode((unsigned char*)ciphertext, ciphertext_temp);

	int i = 0;
	for (i = 0; i < 24; i++) {
		iv_encrypt_base64[i] = ciphertext[i];
	}
	base64_decode(iv_encrypt_base64, iv);

	unsigned char buf_encrypt[256] = {0};
	unsigned char buf_encrypt_base64[256] = {0};
	int j = 0;
	for (i = 24; i < strlen((char*)ciphertext); i++) {
		buf_encrypt_base64[j++] = ciphertext[i];
	}
	int len = base64_decode(buf_encrypt_base64, buf_encrypt);
	printf("len=%d\n", len);
	print_show("buf_encrypt", buf_encrypt, len);
	AES_set_decrypt_key(key, bits, &aesKey);
	AES_cbc_encrypt((unsigned char *)buf_encrypt, (unsigned char *)plaintext, len, &aesKey, iv, 0);
	print_show("plaintext", (unsigned char *)plaintext, strlen(plaintext));

	return strlen(plaintext);
}

int crypt_test()
{
#if 0
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
	
	openssl_aes_test();
#endif

	openssl_aes_test();

	char ans[128] = {0};
	char text[128] = {0};
	int len = 0;

	len = util_crypt_aes128("Root", ans);
	util_decrypt_aes128(ans, text);
	printf("%d, cipher=%s, text=%s\n", len, ans, text);
    memset(ans, 0, sizeof(ans));
	memset(text, 0, sizeof(text));

	len = util_crypt_aes128("Root@123", ans);
	util_decrypt_aes128(ans, text);
	printf("%d, cipher=%s, text=%s\n", len, ans, text);
    memset(ans, 0, sizeof(ans));
	memset(text, 0, sizeof(text));

	len = util_crypt_aes128("123456789012345", ans);
	util_decrypt_aes128(ans, text);
	printf("%d, cipher=%s, text=%s\n", len, ans, text);
    memset(ans, 0, sizeof(ans));
	memset(text, 0, sizeof(text));

	len = util_crypt_aes128("1234567890123456", ans);
	util_decrypt_aes128(ans, text);
	printf("%d, cipher=%s, text=%s\n", len, ans, text);
    memset(ans, 0, sizeof(ans));
	memset(text, 0, sizeof(text));

	len = util_crypt_aes128("12345678901234567", ans);
	util_decrypt_aes128(ans, text);
	printf("%d, cipher=%s, text=%s\n", len, ans, text);
    memset(ans, 0, sizeof(ans));
	memset(text, 0, sizeof(text));

	len = util_crypt_aes128("123456789012345678901234", ans);
	util_decrypt_aes128(ans, text);
	printf("%d, cipher=%s, text=%s\n", len, ans, text);
    memset(ans, 0, sizeof(ans));
	memset(text, 0, sizeof(text));

	len = util_crypt_aes128("123456789012345678901234567890", ans);
	util_decrypt_aes128(ans, text);
	printf("%d, cipher=%s, text=%s\n", len, ans, text);
    memset(ans, 0, sizeof(ans));
	memset(text, 0, sizeof(text));

	len = util_crypt_aes128("123456789012345678901234567890a", ans);
	util_decrypt_aes128(ans, text);
	printf("%d, cipher=%s, text=%s, len=%d\n", len, ans, text, strlen(text));
    memset(ans, 0, sizeof(ans));
	memset(text, 0, sizeof(text));

	len = util_crypt_aes128("123456789012345678901234567890ab", ans);
	util_decrypt_aes128(ans, text);
	printf("%d, cipher=%s, text=%s\n", len, ans, text);
    memset(ans, 0, sizeof(ans));
	memset(text, 0, sizeof(text));

	return 0;
}

