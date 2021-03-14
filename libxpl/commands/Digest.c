#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>
#include <libxpl/abstraction/xef.h>

void xplCmdDigestEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdDigestParams
{
	xefCryptoDigestMethod method;
} xplCmdDigestParams, *xplCmdDigestParamsPtr;

static const xplCmdDigestParams params_stencil =
{
	.method = 0
};

static xplCmdParamDictValue method_dict[] = {
	{ BAD_CAST "md4", XEF_CRYPTO_DIGEST_METHOD_MD4 },
	{ BAD_CAST "md5", XEF_CRYPTO_DIGEST_METHOD_MD5 },
	{ BAD_CAST "ripemd-160", XEF_CRYPTO_DIGEST_METHOD_RIPEMD160 },
	{ BAD_CAST "sha1", XEF_CRYPTO_DIGEST_METHOD_SHA1 },
	{ BAD_CAST "sha-224", XEF_CRYPTO_DIGEST_METHOD_SHA224 },
	{ BAD_CAST "sha-256", XEF_CRYPTO_DIGEST_METHOD_SHA256 },
	{ BAD_CAST "sha-384", XEF_CRYPTO_DIGEST_METHOD_SHA384 },
	{ BAD_CAST "sha-512", XEF_CRYPTO_DIGEST_METHOD_SHA512 },
	{ BAD_CAST "whirlpool", XEF_CRYPTO_DIGEST_METHOD_WHIRLPOOL },
	{ NULL, 0 }
};

xplCommand xplDigestCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdDigestEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE | XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdDigestParams),
	.parameters = {
		{
			.name = BAD_CAST "method",
			.type = XPL_CMD_PARAM_TYPE_DICT,
			.extra.dict_values = method_dict,
			.required = true,
			.value_stencil = &params_stencil.method
		}, {
			.name = NULL
		}
	}
};

void xplCmdDigestEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdDigestParamsPtr params = (xplCmdDigestParamsPtr) commandInfo->params;
	xmlNodePtr ret;
	xefCryptoDigestParams dp;

	if (!(dp.input = commandInfo->content))
	{
		ASSIGN_RESULT(NULL, false, true);
		return;
	}
	dp.input_size = xmlStrlen(dp.input);
	dp.digest_method = params->method;
	if (!xefCryptoDigest(&dp))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "digest error: %s", dp.error? dp.error: BAD_CAST "unknown error"), true, true);
		return;
	}
	ret = xmlNewDocText(commandInfo->document->document, NULL);
	ret->content = xstrBufferToHexAlloc(dp.digest, dp.digest_size, false);
	XPL_FREE(dp.digest);
	ASSIGN_RESULT(ret, false, true);
}
