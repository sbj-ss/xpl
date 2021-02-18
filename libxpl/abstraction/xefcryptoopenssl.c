#include <libxpl/abstraction/xef.h>
#include <libxpl/xplmessages.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/ripemd.h>
#include <openssl/sha.h>
#include <openssl/whrlpool.h>
#include <openssl/rand.h>

bool xefStartupCrypto(xefStartupParamsPtr params)
{
	/* TODO: theoretically we can meet an outdated system with OpenSSL 0.9.x or 1.0.x
	   which requires THREADID and locking functions - but leave this for later. */
    UNUSED_PARAM(params);
    return true;
}

void xefShutdownCrypto(void)
{

}

typedef unsigned char* (*digestFunc)(const unsigned char *d, size_t n, unsigned char *md);

typedef struct _DigestDesc
{
	digestFunc fn;
	size_t size;
} DigestDesc, *DigestDescPtr;

static DigestDesc digest_methods[] =
{
	[XEF_CRYPTO_DIGEST_METHOD_MD4] = { MD4, MD4_DIGEST_LENGTH },
	[XEF_CRYPTO_DIGEST_METHOD_MD5] = { MD5, MD5_DIGEST_LENGTH },
	[XEF_CRYPTO_DIGEST_METHOD_RIPEMD160] = { RIPEMD160, RIPEMD160_DIGEST_LENGTH },
	[XEF_CRYPTO_DIGEST_METHOD_SHA1] = { SHA1, SHA_DIGEST_LENGTH },
	[XEF_CRYPTO_DIGEST_METHOD_SHA224] = { SHA224, SHA224_DIGEST_LENGTH },
	[XEF_CRYPTO_DIGEST_METHOD_SHA256] = { SHA256, SHA256_DIGEST_LENGTH },
	[XEF_CRYPTO_DIGEST_METHOD_SHA384] = { SHA384, SHA384_DIGEST_LENGTH },
	[XEF_CRYPTO_DIGEST_METHOD_SHA512] = { SHA512, SHA512_DIGEST_LENGTH },
	[XEF_CRYPTO_DIGEST_METHOD_WHIRLPOOL] = { (digestFunc) WHIRLPOOL, WHIRLPOOL_DIGEST_LENGTH }
};

bool xefCryptoDigest(xefCryptoDigestParamsPtr params)
{
	if (!params)
		return false;
	if (((int) params->digest_method) < 0 || ((int) params->digest_method) > XEF_CRYPTO_DIGEST_METHOD_MAX)
	{
		params->error = xplFormatMessage(BAD_CAST "unknown digest method '%d'", params->digest_method);
		return false;
	}
	if (!(params->digest = (xmlChar*) XPL_MALLOC(digest_methods[params->digest_method].size)))
	{
		params->error = BAD_CAST XPL_STRDUP("out of memory");
		return false;
	}
	params->digest_size = digest_methods[params->digest_method].size;
	memcpy(params->digest, digest_methods[params->digest_method].fn(params->input, params->input_size, NULL), params->digest_size);
	params->error = NULL;
	return true;
}

bool xefCryptoRandom(xefCryptoRandomParamsPtr params)
{
	if (!params)
		return false;
	if (!params->size)
	{
		params->error = BAD_CAST XPL_STRDUP("params->size must be non-zero");
		return false;
	}
	if (params->alloc_bytes && params->bytes)
	{
		params->error = BAD_CAST XPL_STRDUP("alloc_bytes is set but bytes is not NULL");
		return false;
	}
	if (!params->alloc_bytes && !params->bytes)
	{
		params->error = BAD_CAST XPL_STRDUP("alloc_bytes is not set but bytes is NULL");
		return false;
	}
	if (params->alloc_bytes && !(params->bytes = (unsigned char*) XPL_MALLOC(params->size)))
	{
		params->error = BAD_CAST XPL_STRDUP("out of memory");
		return false;
	}
	// ignore params->secure: RAND_pseudo_bytes deprecated in OpenSSL 1.1
	if (RAND_bytes(params->bytes, params->size) != 1)
	{
		XPL_FREE(params->bytes);
		params->bytes = NULL;
		params->error = BAD_CAST XPL_STRDUP(ERR_error_string(ERR_get_error(), NULL));
		return false;
	}
	params->error = NULL;
	return true;
}
