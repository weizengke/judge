#ifndef _BASE64_H_
#define _BASE64_H_

enum {BASE64_OK = 0, BASE64_INVALID};

#define BASE64_ENCODE_OUT_SIZE(s)	(((s) + 2) / 3 * 4)
#define BASE64_DECODE_OUT_SIZE(s)	(((s)) / 4 * 3)

int base64_encode(const unsigned char *in,  unsigned long len, unsigned char *out);
int base64_decode(const unsigned char *in, unsigned char *out);


#endif
