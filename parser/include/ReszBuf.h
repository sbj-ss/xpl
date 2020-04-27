/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __reszbuf_H
#define __reszbuf_H

#include "Configuration.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Grow strategy in case new content doesn't fit */
typedef enum _ReszBufGrowStrategy 
{
	RESZ_BUF_GROW_UNKNOWN = -1, /* can only be returned by getReszBufGrowStrategy */
	RESZ_BUF_GROW_EXACT = 0,    /* add exactly as many bytes as needed */
	RESZ_BUF_GROW_MIN = RESZ_BUF_GROW_EXACT,
	RESZ_BUF_GROW_DOUBLE,		/* 2x grow every time */
	RESZ_BUF_GROW_INCREMENT,	/* linear increment */
	RESZ_BUF_GROW_FIXED,		/* don't grow, return an error */
	RESZ_BUF_GROW_FLUSH,		/* call user-provided flush function (a remainder can be left in buffer) */
	RESZ_BUF_GROW_MAX = RESZ_BUF_GROW_FLUSH
} ReszBufGrowStrategy;

typedef enum _ReszBufOpResult
{
	RESZ_BUF_RESULT_OK = 0,
	RESZ_BUF_RESULT_NO_MEMORY = -1,
	RESZ_BUF_RESULT_FLUSH_FAILED = -2,
	RESZ_BUF_RESULT_INVALID = -3
} ReszBufOpResult;

typedef struct _ReszBuf *ReszBufPtr;

#define DEFAULT_RESZ_BUF_INITIAL_SIZE 1024
#define DEFAULT_RESZ_BUF_GROW_STRATEGY RESZ_BUF_GROW_DOUBLE
#define DEFAULT_RESZ_BUF_GROW_INCREMENT 1024

/* User function to flush a buffer. Return values:
   0 = ОК
   < 0 = failure
   The function should be able to handle blocks up to the buffer size.
 */
typedef int (*ReszBufFlushCallback)(void* data, size_t size);

/* Creates a buffer with default params */
XPLPUBFUN ReszBufPtr XPLCALL
	createReszBuf(void);
/* Creates a buffer with default params and explicit initial size */
XPLPUBFUN ReszBufPtr XPLCALL
	createReszBufSize(size_t initialSize);
/* Creates a buffer with provided initial size, grow strategy and increment */
XPLPUBFUN ReszBufPtr XPLCALL
	createReszBufParams(size_t initialSize, ReszBufGrowStrategy strategy, size_t increment);
/* Creates a flushable buffer */
XPLPUBFUN ReszBufPtr XPLCALL
	createFlushableReszBuf(size_t size, ReszBufFlushCallback cb);
/* Gets buffer grow strategy */
XPLPUBFUN ReszBufGrowStrategy XPLCALL
	getReszBufGrowStrategy(ReszBufPtr buf);
/* Sets buffer grow strategy */
XPLPUBFUN ReszBufOpResult XPLCALL
	setReszBufGrowStrategy(ReszBufPtr buf, ReszBufGrowStrategy strategy);
/* Gets buffer linear growth increment */
XPLPUBFUN size_t XPLCALL
	getReszBufGrowIncrement(ReszBufPtr buf);
/* Sets buffer linear growth increment */
XPLPUBFUN ReszBufOpResult XPLCALL
	setReszBufGrowIncrement(ReszBufPtr buf, size_t increment);
/* Gets buffer content size */
XPLPUBFUN size_t XPLCALL
	getReszBufContentSize(ReszBufPtr buf);
/* Gets buffer free space */
XPLPUBFUN size_t XPLCALL
	getReszBufFreeSpace(ReszBufPtr buf);
/* Gets buffer total allocated size */
XPLPUBFUN size_t XPLCALL
	getReszBufTotalSize(ReszBufPtr buf);
/* Gets buffer flush function */
XPLPUBFUN ReszBufFlushCallback XPLCALL
	getReszBufFlushCallback(ReszBufPtr buf);
/* Sets buffer flush function */
XPLPUBFUN ReszBufFlushCallback XPLCALL
	setReszBufFlushCallback(ReszBufPtr buf, ReszBufFlushCallback cb);
/* Gets a pointer to buffer content */
XPLPUBFUN void* XPLCALL
	getReszBufContent(ReszBufPtr buf);
/* Gets a pointer to buffer content detaching content from buffer. Buffer is still usable for writing */
XPLPUBFUN void* XPLCALL
	detachReszBufContent(ReszBufPtr buf);
/* Writes data to buffer */
XPLPUBFUN ReszBufOpResult XPLCALL
	addDataToReszBuf(ReszBufPtr buf, void* content, size_t size);
/* Rewinds the content pointer. Memory isn't freed. */
XPLPUBFUN ReszBufOpResult XPLCALL
	rewindReszBuf(ReszBufPtr buf);
/* Gets current buffer position */
XPLPUBFUN void* XPLCALL
	getReszBufPosition(ReszBufPtr buf);
/* Moves buffer position forward (within allocated size) */
XPLPUBFUN ReszBufOpResult XPLCALL
	advanceReszBufferPosition(ReszBufPtr buf, size_t delta);
/* Ensures there's enough place in buffer for direct writing */
XPLPUBFUN ReszBufOpResult XPLCALL
	ensureReszBufFreeSize(ReszBufPtr buf, size_t minfree);
/* Flushes a buffer forcefully */
XPLPUBFUN ReszBufOpResult XPLCALL
	flushReszBuf(ReszBufPtr buf);
/* Deletes a buffer. Content is freed, too. */
XPLPUBFUN void XPLCALL
	freeReszBuf(ReszBufPtr buf);

#ifdef __cplusplus
}
#endif
#endif
