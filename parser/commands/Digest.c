#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>
#include "commands/Digest.h"
#include "openssl/md4.h"
#include "openssl/md5.h"
#include "openssl/ripemd.h"
#include "openssl/sha.h"
#include "openssl/whrlpool.h"

typedef unsigned char* (*xplCmdDigestFunc)(const unsigned char *d, size_t n, unsigned char *md);

typedef struct _xplCmdDigestMethodDesc
{
	xmlChar *name;
	xplCmdDigestFunc fn;
	size_t size;
} xplCmdDigestMethodDesc, *xplCmdDigestMethodDescPtr;

static xplCmdDigestMethodDesc digest_methods[] =
{
	{ BAD_CAST "md4", MD4, MD4_DIGEST_LENGTH },
	{ BAD_CAST "md5", MD5, MD5_DIGEST_LENGTH },
	{ BAD_CAST "ripemd-160", RIPEMD160, RIPEMD160_DIGEST_LENGTH },
	{ BAD_CAST "sha1", SHA1, SHA_DIGEST_LENGTH },
	{ BAD_CAST "sha-224", SHA224, SHA224_DIGEST_LENGTH },
	{ BAD_CAST "sha-256", SHA256, SHA256_DIGEST_LENGTH },
	{ BAD_CAST "sha-384", SHA384, SHA384_DIGEST_LENGTH },
	{ BAD_CAST "sha-512", SHA512, SHA512_DIGEST_LENGTH },
	{ BAD_CAST "whirlpool", (xplCmdDigestFunc) WHIRLPOOL, WHIRLPOOL_DIGEST_LENGTH },
	{ NULL, NULL, 0 }
};

typedef struct _xplCmdDigestParams
{
	xplCmdDigestMethodDescPtr method;
} xplCmdDigestParams, *xplCmdDigestParamsPtr;

static const xplCmdDigestParams params_stencil =
{
	.method = NULL
};

static xmlChar* _getMethod(const xmlChar* raw_value, void **result)
{
	xplCmdDigestMethodDescPtr desc = digest_methods;

	while (desc->name)
	{
		if (!xmlStrcasecmp(desc->name, raw_value))
		{
			*result = desc;
			return NULL;
		}
		desc++;
	}
	return xplFormatMessage(BAD_CAST "unknown digest method '%s'", raw_value);
}

xplCommand xplDigestCommand =
{
	.prologue = xplCmdDigestPrologue,
	.epilogue = xplCmdDigestEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE | XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdDigestParams),
	.parameters = {
		{
			.name = BAD_CAST "method",
			.type = XPL_CMD_PARAM_TYPE_PTR_CUSTOM_GETTER,
			.extra = {
				.ptr_fn = {
					.getter = _getMethod,
					.deallocator = NULL
				}
			},
			.value_stencil = &params_stencil.method
		}, {
			.name = NULL
		}
	}
};

void xplCmdDigestPrologue(xplCommandInfoPtr commandInfo)
{
	UNUSED_PARAM(commandInfo);
}

void xplCmdDigestEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdDigestParamsPtr params = (xplCmdDigestParamsPtr) commandInfo->params;
	xmlNodePtr ret;
	unsigned char *digest;

	digest = params->method->fn(commandInfo->content, xmlStrlen(commandInfo->content), NULL);
	ret = xmlNewDocText(commandInfo->document->document, NULL);
	ret->content = xstrBufferToHex(digest, params->method->size, false);
	ASSIGN_RESULT(ret, false, true);
}
