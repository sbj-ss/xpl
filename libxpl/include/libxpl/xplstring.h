/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __xplstring_H
#define __xplstring_H

#include "Configuration.h"
#include <stdbool.h>
#include <libxml/xmlstring.h>
#include <libxpl/abstraction/xpr.h>

#ifdef __cplusplus
extern "C" {
#endif

/* cuts spaces/tabs/newlines from both string sides, returning a cleaned copy */
XPLPUBFUN xmlChar* XPLCALL
	xstrStrTrim(xmlChar* str);

/* checks if there's more than blanks */
XPLPUBFUN bool XPLCALL
	xstrStrNonblank(xmlChar *str);
/* ^[0-9]+(\.[0-9]+)$ */
XPLPUBFUN bool XPLCALL
	xstrIsNumber(xmlChar *str);

/* returns a human-readable copy of last libxml2 error */
XPLPUBFUN xmlChar* XPLCALL
	xstrGetLastLibxmlError(void);

/* Checks if s is in utf-8 encoding. isCompleteString requires the last char to be a complete sequence */
XPLPUBFUN bool XPLCALL
	xstrIsValidUtf8Sample(xmlChar *s, size_t len, bool isCompleteString);
/* Returns offset to the next utf-8 char. 0 if input sequence is incorrect. */
XPLPUBFUN size_t XPLCALL
	xstrGetOffsetToNextUTF8Char(xmlChar *cur);
/* International characters URI encoding (xn--...) */
XPLPUBFUN xmlChar* XPLCALL
	xstrEncodeUriIdn(xmlChar *uri);

#define XSTR_ENC_UNKNOWN (-1)
#define XSTR_ENC_866 1
#define XSTR_ENC_1251 2
#define XSTR_ENC_KOI8 3
#define XSTR_ENC_UTF8 4
#define XSTR_ENC_UTF16LE 5
#define XSTR_ENC_UTF16BE 6

#define XSTR_DEFAULT_ENC_DET_SAMPLE_LEN 256

/* Detects cyrillic encoding automatically */
XPLPUBFUN int XPLCALL
	xstrDetectEncoding(char* str, size_t sampleLen);

/* Recodes start..end. resultp is allocated inside */
XPLPUBFUN int XPLCALL
	xstrIconvString (const char* tocode, const char* fromcode,
				  const char* start, const char* end,
				  char** resultp, size_t* lengthp);

/* Returns a hex representation of a buffer. Result must be freed. */
XPLPUBFUN xmlChar* XPLCALL
	xstrBufferToHex(void* buf, size_t len, bool prefix);
/* Base64 conversion. result must be preallocated and both result and resultSize set! */
/* TODO alloc inside */
XPLPUBFUN int XPLCALL
	xstrBase64Encode(const void* data_buf, size_t dataLength, char* result, size_t resultSize);
/* TODO alloc inside */
XPLPUBFUN size_t XPLCALL
	xstrBase64Decode(const char* data_buf, size_t dataLength, char* result, size_t resultSize);

/* Converts "/a/b"+"c/d" to "/a/b/c"+"d" etc */
XPLPUBFUN void XPLCALL
	xstrComposeAndSplitPath(xmlChar *basePath, xmlChar *relativePath, xmlChar **normalizedPath, xmlChar **normalizedFilename);

/* Reallocates str appending id decimal representation to its end. */
/* TODO remove */
XPLPUBFUN xmlChar* XPLCALL
	xstrAppendThreadIdToString(xmlChar *str, XPR_THREAD_ID id);

#ifdef __cplusplus
}
#endif
#endif