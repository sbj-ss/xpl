/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __xplbuffer_H
#define __xplbuffer_H

#include "Configuration.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Grow strategy in case new content doesn't fit */
typedef enum _rbGrowStrategy
{
	RB_GROW_UNKNOWN = -1, /* can only be returned by getrbGrowStrategy */
	RB_GROW_EXACT = 0,    /* add exactly as many bytes as needed */
	RB_GROW_MIN = RB_GROW_EXACT,
	RB_GROW_DOUBLE,		/* 2x grow every time */
	RB_GROW_INCREMENT,	/* linear increment */
	RB_GROW_FIXED,		/* don't grow, return an error */
	RB_GROW_FLUSH,		/* call user-provided flush function (a remainder can be left in buffer) */
	RB_GROW_MAX = RB_GROW_FLUSH
} rbGrowStrategy;

typedef enum _rbOpResult
{
	RB_RESULT_OK = 0,
	RB_RESULT_NO_MEMORY = -1,
	RB_RESULT_FLUSH_FAILED = -2,
	RB_RESULT_INVALID = -3
} rbOpResult;

typedef struct _rbBuf *rbBufPtr;

#define RB_DEFAULT_INITIAL_SIZE 1024
#define RB_DEFAULT_GROW_STRATEGY RB_GROW_DOUBLE
#define RB_DEFAULT_GROW_INCREMENT 1024

/* User function to flush a buffer. Return values:
   0 = ОК
   < 0 = failure
   The function should be able to handle blocks up to the buffer size.
 */
typedef int (*rbFlushCallback)(void* data, size_t size);

/* Creates a buffer with default parameters */
XPLPUBFUN rbBufPtr XPLCALL
	rbCreateBuf(void);
/* Creates a buffer with default parameters and explicit initial size */
XPLPUBFUN rbBufPtr XPLCALL
	rbCreateBufSize(size_t initialSize);
/* Creates a buffer with provided initial size, grow strategy and increment */
XPLPUBFUN rbBufPtr XPLCALL
	rbCreateBufParams(size_t initialSize, rbGrowStrategy strategy, size_t increment);
/* Creates a flushable buffer */
XPLPUBFUN rbBufPtr XPLCALL
	rbCreateFlushableBuf(size_t size, rbFlushCallback cb);
/* Gets buffer grow strategy */
XPLPUBFUN rbGrowStrategy XPLCALL
	rbGetBufGrowStrategy(rbBufPtr buf);
/* Sets buffer grow strategy */
XPLPUBFUN rbOpResult XPLCALL
	rbSetBufGrowStrategy(rbBufPtr buf, rbGrowStrategy strategy);
/* Gets buffer linear growth increment */
XPLPUBFUN size_t XPLCALL
	rbGetBufGrowIncrement(rbBufPtr buf);
/* Sets buffer linear growth increment */
XPLPUBFUN rbOpResult XPLCALL
	rbSetBufGrowIncrement(rbBufPtr buf, size_t increment);
/* Gets buffer content size */
XPLPUBFUN size_t XPLCALL
	rbGetBufContentSize(rbBufPtr buf);
/* Gets buffer free space */
XPLPUBFUN size_t XPLCALL
	rbGetBufFreeSpace(rbBufPtr buf);
/* Gets buffer total allocated size */
XPLPUBFUN size_t XPLCALL
	rbGetBufTotalSize(rbBufPtr buf);
/* Gets buffer flush function */
XPLPUBFUN rbFlushCallback XPLCALL
	rbGetBufFlushCallback(rbBufPtr buf);
/* Sets buffer flush function */
XPLPUBFUN rbFlushCallback XPLCALL
	rbSetBufFlushCallback(rbBufPtr buf, rbFlushCallback cb);
/* Gets a pointer to buffer content */
XPLPUBFUN void* XPLCALL
	rbGetBufContent(rbBufPtr buf);
/* Gets a pointer to buffer content detaching content from buffer. Buffer is still usable for writing */
XPLPUBFUN void* XPLCALL
	rbDetachBufContent(rbBufPtr buf);
/* Writes data to buffer */
XPLPUBFUN rbOpResult XPLCALL
	rbAddDataToBuf(rbBufPtr buf, void* content, size_t size);
/* Rewinds the content pointer. Memory isn't freed. */
XPLPUBFUN rbOpResult XPLCALL
	rbRewindBuf(rbBufPtr buf);
/* Gets current buffer position */
XPLPUBFUN void* XPLCALL
	rbGetBufPosition(rbBufPtr buf);
/* Moves buffer position forward (within allocated size) */
XPLPUBFUN rbOpResult XPLCALL
	rbAdvanceBufPosition(rbBufPtr buf, size_t delta);
/* Ensures there's enough place in buffer for direct writing */
XPLPUBFUN rbOpResult XPLCALL
	rbEnsureBufFreeSize(rbBufPtr buf, size_t minfree);
/* Flushes a buffer forcefully */
XPLPUBFUN rbOpResult XPLCALL
	rbFlushBuf(rbBufPtr buf);
/* Deletes a buffer. Content is freed, too. */
XPLPUBFUN void XPLCALL
	rbFreeBuf(rbBufPtr buf);

#ifdef __cplusplus
}
#endif
#endif
