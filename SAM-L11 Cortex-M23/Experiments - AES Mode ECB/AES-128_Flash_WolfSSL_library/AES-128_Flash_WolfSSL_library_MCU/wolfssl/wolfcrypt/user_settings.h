#ifndef USER_SETTINGS_H
#define USER_SETTINGS_H

#define NO_FILESYSTEM

#define WOLFSSL_AES_DIRECT
#define WOLFSSL_SP_SMALL      /* use smaller version of code, disable for larger code but slightly more performant option */


#undef NO_RSA
#define NO_RSA

#undef NO_SHA
#define NO_SHA

#undef WOLFCRYPT_ONLY
#define WOLFCRYPT_ONLY

#undef SINGLE_THREADED
#define SINGLE_THREADED

#undef NO_DSA
#define NO_DSA

#undef NO_CODING
#define NO_CODING

#undef NO_RABBIT
#define NO_RABBIT

#undef NO_HMAC
#define NO_HMAC

#undef NO_PWDBASED
#define NO_PWDBASED

#undef NO_SHA256
#define NO_SHA256

#undef WC_NO_HASHDRBG
#define WC_NO_HASHDRBG

#undef WC_NO_RNG
#define WC_NO_RNG

#undef CUSTOM_RAND_GENERATE_BLOCK
#define CUSTOM_RAND_GENERATE_BLOCK

#undef NO_DES3
#define NO_DES3

#undef NO_DES
#define NO_DES

#undef NO_MD5
#define NO_MD5

#undef NO_MD4
#define NO_MD4

#undef NO_RC4
#define NO_RC4

#undef NO_ASN
#define NO_ASN

#undef NO_DH
#define NO_DH

#undef NO_HC128
#define NO_HC128

#undef NO_BIG_INT
#define NO_BIG_INT

#undef WOLFSSL_NO_HASH
#define WOLFSSL_NO_HASH

#undef NO_SIG_WRAPPER
#define NO_SIG_WRAPPER

#undef NO_CERTS
#define NO_CERTS

#endif /* USER_SETTINGS_H */
