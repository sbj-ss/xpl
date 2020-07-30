#include <libxpl/xplstring.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <iconv.h>
#include <libxml/chvalid.h>
#include <libxml/entities.h>
#ifdef _USE_LIBIDN
#include <idna.h>
#include <idn-free.h>
#endif

xmlChar* strTrim(xmlChar* str)
{
	size_t out_size;
	xmlChar *start, *end, *out;

	start = str;
	end = str + xmlStrlen(str) - 1;
	while (start <= end)
	{
		if (xmlIsBlank_ch(*start))
			start++;
		else break;
	}
	while (start < end)
	{
		if (xmlIsBlank_ch(*end))
			end--;
		else break;
	}
	if (start > end)
		return NULL;
	out_size = end - start + 1;
	out = (xmlChar*) XPL_MALLOC(out_size + 1);
	strncpy((char*) out, (const char*) start, out_size + 1);
	return out;
}

bool strNonblank(xmlChar* str)
{
	if (!str)
		return false;
	while (*str)
	{
		if (!xmlIsBlank_ch(*str))
			return true;
		str++;
	}
	return false;
}

bool isNumber(xmlChar *str)
{
	xmlChar *p;
	bool dot = false;

	if (!str)
		return false;
	p = str;
	while (*p)
	{
		if (isdigit(*p))
			NOOP();
		else if ((*p == '.') && !dot)
			dot = true;
		else
			return false;
		p++;
	}
	return true;
}

xmlChar* getLastLibxmlError()
{
	xmlChar *error, *encError;
	xmlErrorPtr err;
	size_t max_err_len;

	err = xmlGetLastError();
	if (!err)
		return BAD_CAST XPL_STRDUP("unknown error");
	max_err_len = (err->message?strlen(err->message):0) + (err->file?strlen(err->file):0) + 127;
	if (err->str1) max_err_len += strlen(err->str1);
	if (err->str2) max_err_len += strlen(err->str2);
	if (err->str3) max_err_len += strlen(err->str3);
	error = (xmlChar*) XPL_MALLOC(max_err_len + 1);
	if (!error)
		return NULL;
	snprintf((char*) error, max_err_len, "file %s, %d:%d, problem: %s, extra info [%s, %s, %s]", err->file, err->line, err->int2, err->message, err->str1, err->str2, err->str3);
	encError = xmlEncodeSpecialChars(NULL, error);
	XPL_FREE(error);
	return encError;
}

/* previous implementations could result in false positives/negatives
 due to incomplete sequences in sample. let's use a state machine instead. */

typedef enum _utf8CheckStep
{
	U8CS_DECISION = 0,
	U8CS_2_1,
	U8CS_3_1,
	U8CS_3_2,
	U8CS_4_1,
	U8CS_4_2,
	U8CS_4_3
} utf8CheckStep;

typedef struct _utf8CheckState
{
	uint8_t mask;
	uint8_t result;
	utf8CheckStep next;
} utf8CheckState, *utf8CheckStatePtr;

static utf8CheckState U8DecisionList[] =
{
	{ 0x80, 0x00, U8CS_DECISION },
	{ 0xE0, 0xC0, U8CS_2_1 },
	{ 0xF0, 0xE0, U8CS_3_1 },
	{ 0xF8, 0xF0, U8CS_4_1 }
};
#define U8_DECISION_LIST_SIZE (sizeof(U8DecisionList)/sizeof(U8DecisionList[0]))

static utf8CheckState U8TransitionList[] =
{
	{ 0xFF, 0x00, U8CS_DECISION },	/* stub */
	{ 0xC0, 0x80, U8CS_DECISION },	/* U8CS_2_1 */
	{ 0xC0, 0x80, U8CS_3_2 },		/* U8CS_3_1 */
	{ 0xC0, 0x80, U8CS_DECISION },	/* U8CS_3_2 */
	{ 0xC0, 0x80, U8CS_4_2 },		/* U8CS_4_1 */
	{ 0xC0, 0x80, U8CS_4_3 },		/* U8CS_4_2 */
	{ 0xC0, 0x80, U8CS_DECISION }	/* U8CS_4_3 */
};

bool isValidUtf8Sample(xmlChar *s, size_t len, bool isCompleteString)
{
	xmlChar *end = s + len, *p = s;
	utf8CheckStep step = U8CS_DECISION;
	int i;

	while (p < end)
	{
		if (step == U8CS_DECISION) /* beginning of a char */
		{
			for (i = 0; i < U8_DECISION_LIST_SIZE; i++)
				if ((*p & U8DecisionList[i].mask) == U8DecisionList[i].result)
				{
					step = U8DecisionList[i].next;
					break;
				}
			if (i == U8_DECISION_LIST_SIZE) /* sorry, nothing matches */
				return false;
		} else { /* transition */
			if ((*p & U8TransitionList[step].mask) == U8TransitionList[step].result)
				step = U8TransitionList[step].next;
			else
				return false;
		}
		p++;
	}
	if (isCompleteString)
		return (step == U8CS_DECISION);
	return true;
}

size_t getOffsetToNextUTF8Char(xmlChar *cur)
{
	if (!cur || !*cur)
		return 0;
	if (!(*cur & 0x80))
		return 1;
	else if ((*cur & 0xE0) == 0xC0)
		return 2;
	else if ((*cur & 0xF0) == 0xE0)
		return 3;
	else if ((*cur & 0xF8) == 0xF0)
		return 4;
	/* we're in the middle of a char, let's advance to the beginning of a valid one */
	/* TODO suspicious code */
	return getOffsetToNextUTF8Char(cur + 1) + 1;
}

xmlChar* encodeUriIdn(xmlChar *uri)
{
#ifdef _USE_LIBIDN
	xmlChar *ret = NULL, *slash_pos;
	char *enc_domain;
	slash_pos = BAD_CAST xmlStrchr(uri, '/');
	if (!slash_pos || (slash_pos == uri)) /* uri doesn't contain '/' or starts with '/' */
		return NULL;
	if (*(slash_pos - 1) != ':')
		return NULL; /* not ':/' */
	if (*(++slash_pos) != '/')
		return NULL; /* not '//' */
	idna_to_ascii_8z((char*) slash_pos+1, &enc_domain, 0);
	if (!enc_domain)
		return NULL;
	/* we probably don't have 4Gb long URIs :) */
	ret = xmlStrndup(uri, (int)(slash_pos - uri + 1));
	ret = xmlStrcat(ret, BAD_CAST enc_domain);
#ifdef _WIN32
	idn_free(enc_domain);
#else
	free(enc_domain);
#endif
	return ret;
#else
	return XPL_STRDUP(uri);
#endif
}

int detectEncoding(char* str, size_t sampleLen)
{
	size_t small_count, caps_count;
	size_t a0af_count, f0ff_count;
	size_t i;
	size_t real_sample_len; /* only higher ASCII half */
	unsigned char c;

	/* look for zeros. if there're some - this is UTF-16 */
	for (i = 0; i < sampleLen; i++)
	{
		if (!str[i])
		{
			if (i % 2)
				return DETECTED_ENC_UTF16LE;
			else
				return DETECTED_ENC_UTF16BE;
		}
	}
	/* check for utf-8 */
	if (isValidUtf8Sample(BAD_CAST str, sampleLen, false))
		return DETECTED_ENC_UTF8;
	/* if there's too much capital letters - this is probably KOI-8 */
	small_count = caps_count = 0;
	for (i = 0; i < sampleLen; i++)
	{
		c = (unsigned char) str[i];
		if ((c == 0xB8) || (c >= 0xE0))
			small_count++;
		else if ((c == 0xA8) || ((c >= 0xC0) && (c <= 0xDF)))
			caps_count++;
	}
	if ((caps_count > small_count) && small_count) /* entire text could be in caps */
		return DETECTED_ENC_KOI8;
	/* symbol frequency tests, courtesy of E. Kochergov */
	a0af_count = f0ff_count = real_sample_len = 0;
    for (i = 0; i < sampleLen; i++)
    {
		c = (unsigned char) str[i];
		if ((c >= 0xA0) && (c <= 0xAF)) a0af_count++;
        else if (c >= 0xF0) f0ff_count++;
		if (c & 0x80) real_sample_len++;
    }
    if ((((float) a0af_count) / real_sample_len < 0.33985) && (((float) f0ff_count) / real_sample_len > 0.15105))
		return DETECTED_ENC_1251;
	else if (a0af_count || f0ff_count)
		return DETECTED_ENC_866;
	return DETECTED_ENC_UTF8; /* last resort */
}

int iconv_string (const char* tocode, const char* fromcode,
				  const char* start, const char* end,
				  char** resultp, size_t* lengthp)
{
	size_t insize, outsize, res;
	char *result, *outptr;
	const char *inptr;
	iconv_t cd;

	insize = end - start;
	if (!strcmp(tocode, fromcode)) /* just copy */
	{
		result = *resultp = (char*) (*resultp == NULL? XPL_MALLOC(insize+2): XPL_REALLOC(*resultp, insize+2));
		memcpy(*resultp, start, insize);
		result[insize] = result[insize+1] = 0; /* in case of utf-16 */
		if (lengthp)
			*lengthp = insize;//+ 1;
		return 0;
	}
	cd = iconv_open(tocode, fromcode);
	if (cd == (iconv_t) (-1))
		return -1;
	result = (char*) XPL_MALLOC((end - start + 1)*sizeof(uint32_t)); /* up to 4 bytes per char */
	if (!result)
	{
		iconv_close(cd);
		errno = ENOMEM;
		return -1;
	}
	inptr = start;
	outptr = result;
	outsize = insize*2;
	while (insize)
	{
		res = iconv(cd, (char**) &inptr, &insize, &outptr, &outsize);
		if (res == (size_t) - 1)
		{
			iconv_close(cd);
			if (resultp)
				*resultp = NULL;
			return -1;
		}
	}
	iconv_close(cd);
	/* cut the block */
	*outptr = *(outptr + 1) = 0;
	if (lengthp)
		*lengthp = outptr - result;
	result = (char*) XPL_REALLOC(result, outptr - result + 2);
	if (resultp)
		*resultp = result;
	return 0;
}

static xmlChar hex_digits[] = "0123456789ABCDEF";

xmlChar* bufferToHex(void* buf, size_t len, bool prefix)
{
	xmlChar *ret, *ret_start;
	size_t i;

	ret = ret_start = (xmlChar*) XPL_MALLOC(len*2 + (prefix? 3: 1));
	if (!ret)
		return NULL;
	if (prefix)
	{
		*ret++ = '0';
		*ret++ = 'x';
	}
	for (i = 0; i < len; i++)
	{
		*ret++ = hex_digits[((xmlChar*) buf)[i] >> 4];
		*ret++ = hex_digits[((xmlChar*) buf)[i] & 0x0F];
	}
	*ret = 0;
	return ret_start;
}

/* BASE64 */
/* Borrowed from http://en.wikibooks.org/wiki/Algorithm_Implementation/Miscellaneous/Base64 */
int base64encode(const void* data_buf, size_t dataLength, char* result, size_t resultSize)
{
	const char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	const uint8_t *data = (const uint8_t*) data_buf;
	size_t result_index = 0;
	size_t x;
	uint32_t n = 0;
	int padCount = (int) (dataLength % 3);
	uint8_t n0, n1, n2, n3;

	/* increment over the length of the string, three characters at a time */
	for (x = 0; x < dataLength; x += 3)
	{
		/* these three 8-bit (ASCII) characters become one 24-bit number */
		n = data[x] << 16;
		if ((x+1) < dataLength)
			n += data[x+1] << 8;
		if ((x+2) < dataLength)
			n += data[x+2];
		/* this 24-bit number gets separated into four 6-bit numbers */
		n0 = (uint8_t)(n >> 18) & 0x3F;
		n1 = (uint8_t)(n >> 12) & 0x3F;
		n2 = (uint8_t)(n >> 6) & 0x3F;
		n3 = (uint8_t) n & 0x3F;
		/*
		 * if we have one byte available, then its encoding is spread
		 * out over two characters
		 */
		if (result_index >= resultSize) return 0;   /* indicate failure: buffer too small */
		result[result_index++] = base64chars[n0];
		if (result_index >= resultSize) return 0;   /* indicate failure: buffer too small */
		result[result_index++] = base64chars[n1];
		/*
		 * if we have only two bytes available, then their encoding is
		 * spread out over three chars
		 */
		if ((x+1) < dataLength)
		{
			if (result_index >= resultSize) return 0;   /* indicate failure: buffer too small */
			result[result_index++] = base64chars[n2];
		}
		/*
		 * if we have all three bytes available, then their encoding is spread
		 * out over four characters
		 */
		if ((x+2) < dataLength)
		{
			if (result_index >= resultSize) return 0;   /* indicate failure: buffer too small */
			result[result_index++] = base64chars[n3];
		}
	}
 	/*
	 * create and add padding that is required if we did not have a multiple of 3
	 * number of characters available
	 */
	if (padCount > 0)
	{
		for (; padCount < 3; padCount++)
		{
			if(result_index >= resultSize) return 0;   /* indicate failure: buffer too small */
			result[result_index++] = '=';
		}
	}
	if (result_index >= resultSize) return 0;   /* indicate failure: buffer too small */
	result[result_index] = 0;
	return 1;   /* indicate success */
}

#define WHITESPACE 64
#define EQUALS     65
#define INVALID    66

static const unsigned char base64_table[] =
{
	66, 66, 66, 66, 66, 66, 66, 66, /* 0x00 - 0x07 */
	66, 64, 66, 66, 66, 66, 66, 66, /* 0x08 - 0x0F */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0x10 - 0x17 */
	66,	66, 66, 66, 66, 66, 66, 66, /* 0x18 - 0x1F */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0x20 - 0x27 */
	66, 66, 66, 62, 66, 66, 66, 63, /* 0x28 - 0x2F ...+.../ */
	52, 53,	54, 55, 56, 57, 58, 59, /* 0x30 - 0x37 01234567 */
	60, 61, 66, 66, 66, 65, 66, 66, /* 0x38 - 0x3F 89...=.. */
	66,  0,  1,  2,  3,  4,  5,  6, /* 0x40 - 0x47 .ABCDEFG */
	 7,  8,  9, 10, 11, 12, 13, 14, /* 0x48 - 0x4F HIJKLMNO */
	15, 16, 17, 18, 19, 20, 21, 22, /* 0x50 - 0x57 PQRSTUVW */
	23, 24, 25, 66, 66, 66, 66, 66, /* 0x58 - 0x5F XYZ..... */
	66, 26, 27, 28,	29, 30, 31, 32, /* 0x60 - 0x67 .abcdefg */
	33, 34, 35, 36, 37, 38, 39, 40, /* 0x68 - 0x6F hijklmno */
	41, 42, 43, 44, 45, 46, 47, 48, /* 0x70 - 0x77 pqrstuvw */
	49, 50, 51, 66, 66,	66, 66, 66, /* 0x78 - 0x7F xyz..... */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0x80 - 0x87 */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0x88 - 0x8F */
	66, 66, 66, 66, 66, 66,	66, 66, /* 0x90 - 0x97 */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0x98 - 0x9F */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0xA0 - 0xA7 */
	66, 66, 66, 66, 66, 66, 66,	66, /* 0xA8 - 0xAF */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0xB0 - 0xB7 */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0xB8 - 0xBF */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0xC0 - 0xC7 */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0xC8 - 0xCF */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0xD0 - 0xD7 */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0xD8 - 0xDF */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0xE0 - 0xE7 */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0xE8 - 0xEF */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0xF0 - 0xF7 */
	66, 66,	66, 66, 66, 66, 66, 66  /* 0xF8 - 0xFF */
};

size_t base64decode (const char *data_buf, size_t dataLength, char *result, size_t resultSize)
{
	const char *end = data_buf + dataLength;
	size_t buf = 1, len = 0;

	while (data_buf < end)
	{
		unsigned char c = base64_table[(unsigned char) *data_buf++];

		switch (c)
		{
		case WHITESPACE: continue;   /* skip whitespace */
		case INVALID:    return (size_t) -1;   /* invalid input, return error */
		case EQUALS:                 /* pad character, end of data */
			data_buf = end;
			continue;
		default:
			buf = buf << 6 | c;
			/* If the buffer is full, split it into bytes */
			if (buf & 0x1000000)
			{
				if ((len += 3) > resultSize)
					return (size_t) -1; /* buffer overflow */
				*result++ = (buf >> 16) & 0xFF;
				*result++ = (buf >> 8)  & 0xFF;
				*result++ = buf & 0xFF;
				buf = 1;
			}
		}
	}

	if (buf & 0x40000)
	{
		if ((len += 2) > resultSize)
			return (size_t) -1; /* buffer overflow */
		*result++ = (buf >> 10) & 0xFF;
		*result++ = (buf >> 2)  & 0xFF;
	} else if (buf & 0x1000) {
		if (++len > resultSize)
			return (size_t) -1; /* buffer overflow */
		*result++ = (buf >> 4) & 0xFF;
	}
	return len;
}

void composeAndSplitPath(xmlChar *basePath, xmlChar *relativePath, xmlChar **normalizedPath, xmlChar **normalizedFilename)
{
	size_t base_path_len = xmlStrlen(basePath);

	*normalizedFilename = BAD_CAST strrchr((const char*) relativePath, '/');
	if (*normalizedFilename)
	{
		(*normalizedFilename)++;
		*normalizedPath = (xmlChar*) XPL_MALLOC(xmlStrlen(basePath) + (*normalizedFilename - relativePath) + 1);
		if ((basePath[base_path_len - 1] != XPR_PATH_DELIM) && (basePath[base_path_len - 1] != XPR_PATH_INVERSE_DELIM))
			strcpy((char*) *normalizedPath, (char*) basePath);
		else {
			strncpy((char*) *normalizedPath, (char*) basePath, base_path_len - 1);
			(*normalizedPath)[base_path_len - 1] = 0;
		}
		strncat((char*) *normalizedPath, (char*) relativePath, *normalizedFilename - relativePath);
	} else {
		*normalizedFilename = relativePath;
		*normalizedPath = BAD_CAST XPL_STRDUP((char*) basePath);
	}
	if (*normalizedPath)
		xprConvertSlashes(*normalizedPath);
}

xmlChar* appendThreadIdToString(xmlChar *str, XPR_THREAD_ID id)
{
	size_t len;
	char buf[17];

	len = xmlStrlen(str);
	str = (xmlChar*) XPL_REALLOC(str, len + sizeof(buf));
	if (!str)
		return NULL;
	/* TODO x64 */
	snprintf(buf, 17, XPR_THREAD_ID_FORMAT, id);
	strcpy((char*) (str+len), buf);
	return str;
}
