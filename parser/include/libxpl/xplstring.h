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
	strTrim(xmlChar* str);

/* checks if there's more than blanks */
XPLPUBFUN bool XPLCALL
	strNonblank(xmlChar *str);
/* ^[0-9]+(\.[0-9]+)$ */
XPLPUBFUN bool XPLCALL
	isNumber(xmlChar *str);

/* returns a human-readable copy of last libxml2 error */
XPLPUBFUN xmlChar* XPLCALL
	getLastLibxmlError(void);

/* Проверка на корректную utf-8 запись */
XPLPUBFUN bool XPLCALL
	isValidUtf8Sample(xmlChar *s, size_t len, bool isCompleteString);
/* Получение смещения до следующего UTF-8 символа. При неверной записи возвращает 0! */
XPLPUBFUN size_t XPLCALL
	getOffsetToNextUTF8Char(xmlChar *cur);
/* Поддержка кириллических (и не только) URI */
XPLPUBFUN xmlChar* XPLCALL
	encodeUriIdn(xmlChar *uri);

#define DETECTED_ENC_UNKNOWN (-1)
#define DETECTED_ENC_866 1
#define DETECTED_ENC_1251 2
#define DETECTED_ENC_KOI8 3
#define DETECTED_ENC_UTF8 4
#define DETECTED_ENC_UTF16LE 5
#define DETECTED_ENC_UTF16BE 6

#define DEFAULT_ENC_DET_SAMPLE_LEN 256

/* Автоматическое определение кодировки */
XPLPUBFUN int XPLCALL
	detectEncoding(char* str, size_t sampleLen);

/* Перекодировка строки */
XPLPUBFUN int XPLCALL
	iconv_string (const char* tocode, const char* fromcode,
				  const char* start, const char* end,
				  char** resultp, size_t* lengthp);

/* Шестнадцатеричный дамп буфера. Результат необходимо освободить. */
XPLPUBFUN xmlChar* XPLCALL
	bufferToHex(void* buf, size_t len, bool prefix);
/* base-64 запись буфера. Память под результат должна быть выделена до вызова функции, result и resultSize заполнены! */
XPLPUBFUN int XPLCALL
	base64encode(const void* data_buf, size_t dataLength, char* result, size_t resultSize);
XPLPUBFUN size_t XPLCALL
	base64decode(const char* data_buf, size_t dataLength, char* result, size_t resultSize);

XPLPUBFUN void XPLCALL
	composeAndSplitPath(xmlChar *basePath, xmlChar *relativePath, xmlChar **normalizedPath, xmlChar **normalizedFilename);

/* Дописать в конец строки шестнадцатиричную запись id. Возвращает указатель на новую строку (xmlRealloc). */
/* TODO remove */
XPLPUBFUN xmlChar* XPLCALL
	appendThreadIdToString(xmlChar *str, XPR_THREAD_ID id);

#ifdef __cplusplus
}
#endif
#endif
