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

void xplCmdDigestPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdDigestEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define METHOD_ATTR (BAD_CAST "method")

	xmlChar *method_attr = NULL;
	xmlChar *content = NULL;
	xmlNodePtr ret;
	unsigned char *digest;
	size_t digest_size;

	method_attr = xmlGetNoNsProp(commandInfo->element, METHOD_ATTR);
	if (!method_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "no digest method specified"), true, true);
		goto done;
	}
	if (!checkNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "non-text nodes inside"), true, true);
		goto done;
	}
	content = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, 1);
	if (!xmlStrcasecmp(method_attr, BAD_CAST "md4"))
	{
		digest = MD4(content, xmlStrlen(content), NULL);
		digest_size = MD4_DIGEST_LENGTH;
	} else if (!xmlStrcasecmp(method_attr, BAD_CAST "md5")) {
		digest = MD5(content, xmlStrlen(content), NULL);
		digest_size = MD5_DIGEST_LENGTH;
	} else if (!xmlStrcasecmp(method_attr, BAD_CAST "ripemd-160")) {
		digest = RIPEMD160(content, xmlStrlen(content), NULL);
		digest_size = RIPEMD160_DIGEST_LENGTH;
	} else if (!xmlStrcasecmp(method_attr, BAD_CAST "sha1")) {
		digest = SHA1(content, xmlStrlen(content), NULL);
		digest_size = SHA_DIGEST_LENGTH;
	} else if (!xmlStrcasecmp(method_attr, BAD_CAST "sha-224")) {
		digest = SHA224(content, xmlStrlen(content), NULL);
		digest_size = SHA224_DIGEST_LENGTH;
	} else if (!xmlStrcasecmp(method_attr, BAD_CAST "sha-256")) {
		digest = SHA256(content, xmlStrlen(content), NULL);
		digest_size = SHA256_DIGEST_LENGTH;
	} else if (!xmlStrcasecmp(method_attr, BAD_CAST "sha-384")) {
		digest = SHA384(content, xmlStrlen(content), NULL);
		digest_size = SHA384_DIGEST_LENGTH;
	} else if (!xmlStrcasecmp(method_attr, BAD_CAST "sha-512")) {
		digest = SHA512(content, xmlStrlen(content), NULL);
		digest_size = SHA512_DIGEST_LENGTH;
	} else if (!xmlStrcasecmp(method_attr, BAD_CAST "whirlpool")) {
		digest = WHIRLPOOL(content, xmlStrlen(content), NULL);
		digest_size = WHIRLPOOL_DIGEST_LENGTH;
	} else {
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "unknown method: %s", method_attr), true, true);
		goto done;
	}
	ret = xmlNewDocText(commandInfo->document->document, NULL);
	ret->content = bufferToHex(digest, digest_size, false);
	ASSIGN_RESULT(ret, false, true);
done:
	if (method_attr)
		XPL_FREE(method_attr);
	if (content)
		XPL_FREE(content);
}

xplCommand xplDigestCommand = { xplCmdDigestPrologue, xplCmdDigestEpilogue };
