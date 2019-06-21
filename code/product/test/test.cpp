/*
	  风继续吹

	夜色如此放肆，
	从不知：
	风继续吹。
	瑟缩街中落泪，
	只有你，
	可细说，
	可倾诉。

            By Jungle Wei.

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
	
#ifdef _LINUX_
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <termios.h>
#include <assert.h>
#endif
	
#ifdef _WIN32_
#include <conio.h>
#include <io.h>
#include <winsock2.h>

#endif

#include "kernel.h"

#include "product/thirdpart32/openssl/rsa.h"
#include "product/thirdpart32/openssl/pem.h"
#include "product/thirdpart32/openssl/ssl.h"
#include "product/thirdpart32/openssl/engine.h"

SSL_CTX *g_pstSslCtx = NULL;

/****************************************************************************
 *			 Constants used when creating the ENGINE						*
 ***************************************************************************/
static const char *engine_p11_id = "pkcs11";
static const char *engine_p11_name = "pkcs11 engine support(by eboy)";
 
 
/****************************************************************************
 *			Functions to handle the engine									*
 ***************************************************************************/
 
/* Initiator which is only present to make sure this engine looks available */
static int p11_init(ENGINE *e)
{
    printf("p11_init  OK.[FILE:%s,LINE:%d]\n",__FILE__,__LINE__);
    return 1;
}
 
/* Finisher which is only present to make sure this engine looks available */
static int p11_finish(ENGINE *e)
{
	printf("p11_finish  OK.[FILE:%s,LINE:%d]\n",__FILE__,__LINE__);
    
    return 1;
}
 
/* Destructor (complements the "ENGINE_ncipher()" constructor) */
static int p11_destroy(ENGINE *e)
{
    
    printf("p11_destroy  OK.[FILE:%s,LINE:%d]\n",__FILE__,__LINE__);
 
    
    return 1;
}
 
/****************************************************************************
 *			Engine commands													*
*****************************************************************************/
static const ENGINE_CMD_DEFN p11_cmd_defns[] = 
{
	{0, NULL, NULL, 0}
};
 
 
/****************************************************************************
 *			RSA functions													*
*****************************************************************************/
static int p11_public_encrypt(int len, const unsigned char *from,
                                 unsigned char *to, RSA *rsa, int padding)
{
    
    printf("\n**************public_encrypt, my function called, success!***********\n\n");
    return 0;
}
 
static int p11_private_decrypt(int len, const unsigned char *from,
                               unsigned char *to, RSA *rsa, int padding)
{
    return 0;
}
 
static RSA_METHOD p11_rsa =
{
    "eboy's pkcs11 PKCS#1 RSA",
        p11_public_encrypt,
        NULL,
        NULL,
        p11_private_decrypt,
        NULL,
        NULL,
        NULL,
        NULL,
        0,
        NULL,
        NULL,
        NULL
};
 
 
/****************************************************************************
 *			Symetric cipher and digest function registrars					*
*****************************************************************************/
 
static int p11_ciphers(ENGINE *e, const EVP_CIPHER **cipher,
						  const int **nids, int nid);
 
static int p11_digests(ENGINE *e, const EVP_MD **digest,
						  const int **nids, int nid);
 
 
static int p11_cipher_nids[] ={ NID_des_cbc, NID_des_ede3_cbc, NID_desx_cbc, 0 };
static int p11_digest_nids[] ={ NID_md2, NID_md5, 0 };
 
 
 
 
/****************************************************************************
 *			Functions to handle the engine									*
*****************************************************************************/
 
static void ERR_load_P11_strings()
{
	printf("ERR_load_P11_strings  OK.[FILE:%s,LINE:%d]\n",__FILE__,__LINE__);
	return;
	
}
 
static int bind_p11(ENGINE *e)
{
	//const RSA_METHOD *meth1;
	if(!ENGINE_set_id(e, engine_p11_id)
		|| !ENGINE_set_name(e, engine_p11_name)
		|| !ENGINE_set_RSA(e, &p11_rsa)
		//|| !ENGINE_set_ciphers(e, p11_ciphers)
		//|| !ENGINE_set_digests(e, p11_digests)
		|| !ENGINE_set_destroy_function(e, p11_destroy)
		|| !ENGINE_set_init_function(e, p11_init)
		|| !ENGINE_set_finish_function(e, p11_finish)
		/* || !ENGINE_set_ctrl_function(e, p11_ctrl) */
		/* || !ENGINE_set_cmd_defns(e, p11_cmd_defns) */)
		return 0;
	
	/* Ensure the p11 error handling is set up */
	ERR_load_P11_strings();
	return 1;
	}
 
 
#ifdef ENGINE_DYNAMIC_SUPPORT
static int bind_helper(ENGINE *e, const char *id)
{
	if(id && (strcmp(id, engine_p11_id) != 0))
		return 0;
	if(!bind_p11(e))
		return 0;
	return 1;
}       
IMPLEMENT_DYNAMIC_CHECK_FN()
IMPLEMENT_DYNAMIC_BIND_FN(bind_helper)
#else
static ENGINE *engine_p11(void)
{
	ENGINE *ret = ENGINE_new();
	if(!ret)
		return NULL;
	if(!bind_p11(ret))
	{
		ENGINE_free(ret);
		return NULL;
	}
	return ret;
}
 
void ENGINE_load_pkcs11(void)
{
	/* Copied from eng_[openssl|dyn].c */
	ENGINE *toadd = engine_p11();
	if(!toadd) return;
	ENGINE_add(toadd);
	ENGINE_free(toadd);
	ERR_clear_error();
}
#endif


void ssl_info_callback(const SSL *s, int where, int ret)
{
    const char *str;
    int w;

    w = where & ~SSL_ST_MASK;

    if (w & SSL_ST_CONNECT)
        str = "SSL_connect";
    else if (w & SSL_ST_ACCEPT)
        str = "SSL_accept";
    else
        str = "undefined";

    if (where & SSL_CB_LOOP) {
        printf("%s:%s\n", str, SSL_state_string_long(s));
    } else if (where & SSL_CB_ALERT) {
        str = (where & SSL_CB_READ) ? "read" : "write";
        printf("SSL3 alert %s:%s:%s\n",
                   str,
                   SSL_alert_type_string_long(ret),
                   SSL_alert_desc_string_long(ret));
    } else if (where & SSL_CB_EXIT) {
        if (ret == 0)
            printf("%s:failed in %s\n",
                       str, SSL_state_string_long(s));
        else if (ret < 0) {
            printf("%s:error in %s\n",
                       str, SSL_state_string_long(s));
        }
    }
}
void ssl_msg_callback(int write_p, int version, int content_type,
                        const void *buf, size_t len, SSL *ssl, void *arg)
{
    const char *str_write_p, *str_version, *str_content_type =
        "", *str_details1 = "", *str_details2 = "";

    str_write_p = write_p ? ">>>" : "<<<";

    switch (version) {
    case SSL2_VERSION:
        str_version = "SSL 2.0";
        break;
    case SSL3_VERSION:
        str_version = "SSL 3.0 ";
        break;
    case TLS1_VERSION:
        str_version = "TLS 1.0 ";
        break;
    case TLS1_1_VERSION:
        str_version = "TLS 1.1 ";
        break;
    case TLS1_2_VERSION:
        str_version = "TLS 1.2 ";
        break;
    case DTLS1_VERSION:
        str_version = "DTLS 1.0 ";
        break;
    case DTLS1_BAD_VER:
        str_version = "DTLS 1.0 (bad) ";
        break;
    default:
        str_version = "???";
    }

    if (version == SSL2_VERSION) {
        str_details1 = "???";

        if (len > 0) {
            switch (((const unsigned char *)buf)[0]) {
            case 0:
                str_details1 = ", ERROR:";
                str_details2 = " ???";
                if (len >= 3) {
                    unsigned err =
                        (((const unsigned char *)buf)[1] << 8) +
                        ((const unsigned char *)buf)[2];

                    switch (err) {
                    case 0x0001:
                        str_details2 = " NO-CIPHER-ERROR";
                        break;
                    case 0x0002:
                        str_details2 = " NO-CERTIFICATE-ERROR";
                        break;
                    case 0x0004:
                        str_details2 = " BAD-CERTIFICATE-ERROR";
                        break;
                    case 0x0006:
                        str_details2 = " UNSUPPORTED-CERTIFICATE-TYPE-ERROR";
                        break;
                    }
                }

                break;
            case 1:
                str_details1 = ", CLIENT-HELLO";
                break;
            case 2:
                str_details1 = ", CLIENT-MASTER-KEY";
                break;
            case 3:
                str_details1 = ", CLIENT-FINISHED";
                break;
            case 4:
                str_details1 = ", SERVER-HELLO";
                break;
            case 5:
                str_details1 = ", SERVER-VERIFY";
                break;
            case 6:
                str_details1 = ", SERVER-FINISHED";
                break;
            case 7:
                str_details1 = ", REQUEST-CERTIFICATE";
                break;
            case 8:
                str_details1 = ", CLIENT-CERTIFICATE";
                break;
            }
        }
    }

    if (version == SSL3_VERSION ||
        version == TLS1_VERSION ||
        version == TLS1_1_VERSION ||
        version == TLS1_2_VERSION ||
        version == DTLS1_VERSION || version == DTLS1_BAD_VER) {
        switch (content_type) {
        case 20:
            str_content_type = "ChangeCipherSpec";
            break;
        case 21:
            str_content_type = "Alert";
            break;
        case 22:
            str_content_type = "Handshake";
            break;
        }

        if (content_type == 21) { /* Alert */
            str_details1 = ", ???";

            if (len == 2) {
                switch (((const unsigned char *)buf)[0]) {
                case 1:
                    str_details1 = ", warning";
                    break;
                case 2:
                    str_details1 = ", fatal";
                    break;
                }

                str_details2 = " ???";
                switch (((const unsigned char *)buf)[1]) {
                case 0:
                    str_details2 = " close_notify";
                    break;
                case 10:
                    str_details2 = " unexpected_message";
                    break;
                case 20:
                    str_details2 = " bad_record_mac";
                    break;
                case 21:
                    str_details2 = " decryption_failed";
                    break;
                case 22:
                    str_details2 = " record_overflow";
                    break;
                case 30:
                    str_details2 = " decompression_failure";
                    break;
                case 40:
                    str_details2 = " handshake_failure";
                    break;
                case 42:
                    str_details2 = " bad_certificate";
                    break;
                case 43:
                    str_details2 = " unsupported_certificate";
                    break;
                case 44:
                    str_details2 = " certificate_revoked";
                    break;
                case 45:
                    str_details2 = " certificate_expired";
                    break;
                case 46:
                    str_details2 = " certificate_unknown";
                    break;
                case 47:
                    str_details2 = " illegal_parameter";
                    break;
                case 48:
                    str_details2 = " unknown_ca";
                    break;
                case 49:
                    str_details2 = " access_denied";
                    break;
                case 50:
                    str_details2 = " decode_error";
                    break;
                case 51:
                    str_details2 = " decrypt_error";
                    break;
                case 60:
                    str_details2 = " export_restriction";
                    break;
                case 70:
                    str_details2 = " protocol_version";
                    break;
                case 71:
                    str_details2 = " insufficient_security";
                    break;
                case 80:
                    str_details2 = " internal_error";
                    break;
                case 90:
                    str_details2 = " user_canceled";
                    break;
                case 100:
                    str_details2 = " no_renegotiation";
                    break;
                case 110:
                    str_details2 = " unsupported_extension";
                    break;
                case 111:
                    str_details2 = " certificate_unobtainable";
                    break;
                case 112:
                    str_details2 = " unrecognized_name";
                    break;
                case 113:
                    str_details2 = " bad_certificate_status_response";
                    break;
                case 114:
                    str_details2 = " bad_certificate_hash_value";
                    break;
                case 115:
                    str_details2 = " unknown_psk_identity";
                    break;
                }
            }
        }

        if (content_type == 22) { /* Handshake */
            str_details1 = "???";

            if (len > 0) {
                switch (((const unsigned char *)buf)[0]) {
                case 0:
                    str_details1 = ", HelloRequest";
                    break;
                case 1:
                    str_details1 = ", ClientHello";
                    break;
                case 2:
                    str_details1 = ", ServerHello";
                    break;
                case 3:
                    str_details1 = ", HelloVerifyRequest";
                    break;
                case 11:
                    str_details1 = ", Certificate";
                    break;
                case 12:
                    str_details1 = ", ServerKeyExchange";
                    break;
                case 13:
                    str_details1 = ", CertificateRequest";
                    break;
                case 14:
                    str_details1 = ", ServerHelloDone";
                    break;
                case 15:
                    str_details1 = ", CertificateVerify";
                    break;
                case 16:
                    str_details1 = ", ClientKeyExchange";
                    break;
                case 20:
                    str_details1 = ", Finished";
                    break;
                }
            }
        }
#ifndef OPENSSL_NO_HEARTBEATS
        if (content_type == 24) { /* Heartbeat */
            str_details1 = ", Heartbeat";

            if (len > 0) {
                switch (((const unsigned char *)buf)[0]) {
                case 1:
                    str_details1 = ", HeartbeatRequest";
                    break;
                case 2:
                    str_details1 = ", HeartbeatResponse";
                    break;
                }
            }
        }
#endif
    }

    printf("%s %s%s [length %04lx]%s%s\n", str_write_p, str_version,
               str_content_type, (unsigned long)len, str_details1,
               str_details2);

#if 0
    if (len > 0) {
        size_t num, i;

        printf("   ");
        num = len;

        for (i = 0; i < num; i++) {
            if (i % 16 == 0 && i > 0)
                printf("\n   ");
            printf(" %02x", ((const unsigned char *)buf)[i]);
        }
        if (i < len)
            printf(" ...");
        printf("\n");
    }
#endif

}

#define SSLA_NO_ERR 			0
#define SSLA_WANT_READ			1
#define SSLA_WANT_WRITE			2
#define SSLA_ERR_CREATE_CONNECT 3

int ssl_accept_state(SSL *ssl, int ret)
{
	int r = 0;

	r = SSL_get_error(ssl, ret);

	switch(r)
	{
        case SSL_ERROR_NONE:
            return SSLA_NO_ERR;
			
		case SSL_ERROR_WANT_READ:
			if (SSL_ST_OK == SSL_get_state(ssl))
			{
				return SSLA_NO_ERR;
			}
			return SSLA_WANT_READ;
			
        case SSL_ERROR_WANT_WRITE:
			if (SSL_ST_OK == SSL_get_state(ssl))
			{
				return SSLA_NO_ERR;
			}
			return SSLA_WANT_WRITE;

        case SSL_ERROR_WANT_X509_LOOKUP:
        case SSL_ERROR_SYSCALL:
        case SSL_ERROR_SSL:
        case SSL_ERROR_ZERO_RETURN:
            return SSLA_ERR_CREATE_CONNECT;
        default:
			return SSLA_ERR_CREATE_CONNECT;			
	}
}

int create_self_signed_cert(X509 **ppX509_Cert, EVP_PKEY **ppEvpKey)
{
	int ret = 0;
	RSA *rsa = NULL;
	BIGNUM *bne = NULL;
	X509_NAME *x509_name = NULL;
	EVP_PKEY *pEvpKey = NULL;
	X509 *x509 = NULL;
	int iCertLen = 0;
	int iLenTmp = 0;
	int iPkeyLen = 0;

	if (NULL == ppX509_Cert || NULL == ppEvpKey)
	{
		return 1;
	}

	/* 1: generate rsa key */
	bne = BN_new();
	ret = BN_set_word(bne, RSA_F4);
	if (ret != 1)
	{
		printf("\r\n BN_set_word failed.");
		goto free_all;
	}
	
	rsa = RSA_new();
	ret = RSA_generate_key_ex(rsa, 2048, bne, NULL);
	if (ret != 1)
	{
		printf("\r\n RSA_generate_key_ex failed.");
		goto free_all;
	}

	pEvpKey = EVP_PKEY_new();
	EVP_PKEY_assign_RSA(pEvpKey, rsa);

	/* 2: create x509 cert */
	x509 = X509_new();
	if (NULL == x509)
	{
		printf("\r\n X509_new failed.");
		goto free_all;
	}

	/* 3: set public key of x509 */
	if (!X509_set_pubkey(x509, pEvpKey))
	{
		printf("\r\n X509_set_pubkey failed.");
		goto free_all;
	}

	/* 4: set version of x509 */
	if (!X509_set_version(x509, 2))
	{
		printf("\r\n X509_set_version failed.");
		goto free_all;
	}

	/* 5: set subject of x509 */
	x509_name = X509_get_subject_name(x509);
	if (NULL == x509_name)
	{
		printf("\r\n X509_get_subject_name failed.");
		goto free_all;
	}

	(VOID)X509_NAME_add_entry_by_txt(x509_name, "C",  MBSTRING_ASC, (const unsigned char *)"CN", -1, -1, 0);
	(VOID)X509_NAME_add_entry_by_txt(x509_name, "ST", MBSTRING_ASC, (const unsigned char *)"JS", -1, -1, 0);
	(VOID)X509_NAME_add_entry_by_txt(x509_name, "L",  MBSTRING_ASC, (const unsigned char *)"NJ", -1, -1, 0);
	(VOID)X509_NAME_add_entry_by_txt(x509_name, "O",  MBSTRING_ASC, (const unsigned char *)"Jungle", -1, -1, 0);
	(VOID)X509_NAME_add_entry_by_txt(x509_name, "OU", MBSTRING_ASC, (const unsigned char *)"VPN", -1, -1, 0);
	(VOID)X509_NAME_add_entry_by_txt(x509_name, "CN", MBSTRING_ASC, (const unsigned char *)"CA", -1, -1, 0);

	if (!X509_set_issuer_name(x509, x509_name))
	{
		printf("\r\n X509_set_issuer_name failed.");
		goto free_all;
	}

	/* 6: set validity of x509, 10years */
	X509_gmtime_adj(X509_get_notBefore(x509), 0);
	X509_gmtime_adj(X509_get_notAfter(x509), (long)60 * 60 * 24 * 365 * 10);

	/* 7: sign for x509 */
	if (!X509_sign(x509, pEvpKey, EVP_sha1()))
	{
		printf("\r\n X509_sign failed.");
		goto free_all;
	}

	BN_free(bne);

	/* test print cert&key */
	RSA_print_fp(stdout, pEvpKey->pkey.rsa, 0);
	PEM_write_PrivateKey(stdout, pEvpKey, NULL, NULL, 0, NULL, NULL);	
	X509_print_fp(stdout, x509);
	PEM_write_X509(stdout, x509);

	*ppX509_Cert = x509;
	*ppEvpKey = pEvpKey;

	return 0;
free_all:

	if (NULL != bne)
	{
		BN_free(bne);
	}

	if (NULL != x509)
	{
		X509_free(x509);
	}

	if (NULL != pEvpKey)
	{
		EVP_PKEY_free(pEvpKey);
	}

	if (NULL != x509)
	{
		X509_free(x509);
	}
	
	return 1;
}

SSL_CTX * create_ssl_ctx()
{
	SSL_CTX* ssl_ctx = NULL;

	SSL_library_init();
	OpenSSL_add_all_algorithms();

	#if 0
	{
		ENGINE *p11_engine = NULL;

		ENGINE_load_pkcs11();

		
		p11_engine = ENGINE_by_id("pkcs11");
		
		if(p11_engine == NULL)
		{
			printf("\r\n p11_engine is null");
			return NULL;
		}

		printf("get pkcs11 engine OK.name:%s\n",ENGINE_get_name(p11_engine));

		ENGINE_register_RSA(p11_engine);

		ENGINE_set_default(p11_engine,ENGINE_METHOD_ALL);

	}
	#endif
	
	SSL_load_error_strings();

	ssl_ctx = SSL_CTX_new(TLSv1_2_server_method());
	if (NULL == ssl_ctx)
	{
		printf("\r\n SSL_CTX_new failed.");
		return NULL;
	}

	(VOID)SSL_CTX_set_options(ssl_ctx, (long)SSL_OP_ALL|SSL_OP_NO_COMPRESSION);

	SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_NONE, NULL);

	return ssl_ctx;
}

int ssl_ctx_init()
{	
	X509 *x509 = NULL;
	EVP_PKEY *pEvpKey = NULL;
	SSL_CTX* ssl_ctx = NULL;
	
	if (NULL != g_pstSslCtx)
	{
		return 0;
	}

	if (0 != create_self_signed_cert(&x509, &pEvpKey))
	{
		printf("\r\n create_self_signed_cert failed.");
		return 1;
	}

	ssl_ctx = create_ssl_ctx();
	if (NULL == ssl_ctx)
	{
		printf("\r\n create_ssl_ctx failed.");

		X509_free(x509);
		EVP_PKEY_free(pEvpKey);
		
		return 1;
	}

	if (1 != SSL_CTX_use_certificate(ssl_ctx, x509))
	{
		printf("\r\n SSL_CTX_use_certificate failed.");
		
		X509_free(x509);
		EVP_PKEY_free(pEvpKey);
		SSL_CTX_free(ssl_ctx);
		
		return 1;
	}

	if (1 != SSL_CTX_use_PrivateKey(ssl_ctx, pEvpKey))
	{
		printf("\r\n SSL_CTX_use_certificate failed.");
		
		X509_free(x509);
		EVP_PKEY_free(pEvpKey);
		SSL_CTX_free(ssl_ctx);
		
		return 1;
	}

	if (SSL_CTX_check_private_key(ssl_ctx) < 0)
	{
		printf("\r\n SSL_CTX_check_private_key failed.");
		
		X509_free(x509);
		EVP_PKEY_free(pEvpKey);
		SSL_CTX_free(ssl_ctx);
		
		return 1;
	}

	g_pstSslCtx = ssl_ctx;

	X509_free(x509);
	EVP_PKEY_free(pEvpKey);
	
	return 0;
}

socket_t g_ssl_socket;

int ssl_socket_init()
{
#ifdef _WIN32_
	WSADATA wsaData;
    WORD sockVersion = MAKEWORD(2, 2);
	if(WSAStartup(sockVersion, &wsaData) != 0)
	{
		printf("\r\n ssl_socket_init, WSAStartup failed.");
		return 0;
	}
#endif

	g_ssl_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(g_ssl_socket == INVALID_SOCKET)
	{
		printf("\r\n ssl_socket_init, socket failed.");
		return 0;
	}

	const char yes = 1;
    setsockopt(g_ssl_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	//unsigned long mode=1; /* FIONBIO */
	//ioctlsocket(g_ssl_socket, FIOASYNC, &mode);

#if 0
	unsigned long mode=1; /* FIONBIO */
	if (ioctlsocket(g_ssl_socket, FIONBIO, &mode) < 0)
	{
		printf("\r\n ssl_socket_init, ioctlsocket failed.");
		closesocket(g_ssl_socket);
		return 0;
	}
#endif

	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(443);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(g_ssl_socket,(const sockaddr*)&sin,sizeof(sin)) < 0)
	{
		printf("\r\n ssl_socket_init, bind failed.");
		closesocket(g_ssl_socket);
		return 0; 
	}

	if(SOCKET_ERROR == listen(g_ssl_socket,20))
	{
		printf("\r\n ssl_socket_init, listen failed.");
		
		closesocket(g_ssl_socket);
		return 0;
	}

	return 1;
}

int ssl_user_thread(void *pEntry)
{
	int ret = 0;
	int lErr = 0;
	char buff[4096] = {0};
	socket_t sock = 0;
	SSL *ssl = (SSL*)pEntry;

	sock = SSL_get_fd(ssl);

	ret = SSL_accept(ssl);

	printf("### SSL connection using %s. ret=%d\r\n", SSL_get_cipher(ssl), ret);
	
	lErr = ssl_accept_state(ssl, ret);
	if (SSLA_ERR_CREATE_CONNECT == lErr)
	{
		printf("### ssl_user_thread, error=%d, ret=%d\r\n", lErr, ret);
		SSL_free(ssl);
		closesocket(sock);
		return 0;
	}
	
	while (1)
	{
		memset(buff, 0, sizeof(buff));

		ret = SSL_read(ssl, buff, sizeof(buff) - 1);
		if (ret < 0)
		{
			printf("### ssl_read error , close 1. ret=%d\r\n", ret);
			break;
		}
		else if (ret == 0)
		{
			printf("### ssl_read error, close 2.\r\n");
			break;
		}
		else
		{
			char buf[1024] = {0};

			char *context = "hello world";

			//printf("\r\n <<< ssl_user_thread, SSL_read. ret=%d, sock=%d, buff=%s", ret, sock, buff);
			
			sprintf(buf, "HTTP/1.1 200 OK\r\n"
				"Content-Type:text/html\r\n"
				"Content-Length:%d\r\n"
				"\r\n%s", strlen(context), context);

			ret = SSL_write(ssl, buf,  strlen(buf));
			lErr = ssl_accept_state(ssl, ret);
			#if 1
			printf("#### ssl_user_thread, SSL_write. ret=%d\r\n", ret);
			#endif
			
		}
		
	}

	SSL_free(ssl);
	
	closesocket(sock);
	
	return 0;
}


int ssl_listen_thread(void *pEntry)
{
	int ret = 0;
	sockaddr_in remoteAddr;
	socket_t sClient;
	socklen_t nAddrLen = sizeof(remoteAddr);
	char buff[1024] = {0};

	SSL *ssl = NULL;

	if (0 == ssl_socket_init())
	{
		printf("\r\n ssl_listen_thread, ssl_socket_init failed.");
		return 0;
	}
	
	while(TRUE)
	{
		sClient = accept(g_ssl_socket, (SOCKADDR*)&remoteAddr, &nAddrLen);
		if(sClient == INVALID_SOCKET)
		{
			continue;
		}

		//printf("\r\n <<< ssl_listen_thread, new accept. socket=%d", sClient);
		
		ssl = SSL_new(g_pstSslCtx);

		SSL_set_info_callback(ssl, ssl_info_callback);
		SSL_set_msg_callback(ssl, ssl_msg_callback);
		SSL_set_msg_callback_arg(ssl,NULL);
		
		ret = SSL_set_fd(ssl, sClient);
		
		thread_create(ssl_user_thread, (void*)ssl);
		
		Sleep(1);
	}

	closesocket(sClient);

	return 0;
}

static void display_engine_list(){
	ENGINE *h;
	int loop;
	
	h = ENGINE_get_first();
	loop = 0;
	
	printf("\r\n >>>>> listing available engine types\n");
	
	while(h)
	{		
		printf("engine %i, id = \"%s\", name = \"%s\"\n",
			loop++, 
			ENGINE_get_id(h),
			ENGINE_get_name(h));
		
		h = ENGINE_get_next(h);	
	}	

	printf("\r\n >>>>> end of list\n");

	/* ENGINE_get_first() increases the struct_ref counter, so we must call ENGINE_free() to decrease it again */	

	ENGINE_free(h);

	return;
}


void ssl_test()
{
	
	ssl_ctx_init();

	thread_create(ssl_listen_thread, NULL);

	display_engine_list();
	
	return ;
}

