#include "Common.h"
#include "ReszBuf.h"

typedef struct _ReszBuf
{
	size_t size;
	ReszBufGrowStrategy grow_strategy;
	size_t increment;
	void *start;
	void *current;
	ReszBufFlushCallback flush_callback;
} ReszBuf, *ReszBufPtr;

#define DEFAULT_RESZ_BUF_INITIAL_SIZE 1024
#define DEFAULT_RESZ_BUF_GROW_STRATEGY RESZ_BUF_GROW_DOUBLE
#define DEFAULT_RESZ_BUF_GROW_INCREMENT 1024

ReszBufPtr createReszBuf()
{
	return createReszBufParams(DEFAULT_RESZ_BUF_INITIAL_SIZE, DEFAULT_RESZ_BUF_GROW_STRATEGY, DEFAULT_RESZ_BUF_GROW_INCREMENT);
}

ReszBufPtr createReszBufSize(size_t initialSize)
{
	return createReszBufParams(initialSize, DEFAULT_RESZ_BUF_GROW_STRATEGY, DEFAULT_RESZ_BUF_GROW_INCREMENT);
}

ReszBufPtr createReszBufParams(size_t initialSize, ReszBufGrowStrategy strategy, size_t increment)
{
	ReszBufPtr ret = (ReszBufPtr) xmlMalloc(sizeof(ReszBuf));
	if (!ret)
		return NULL;
	if ((ret->start = ret->current = xmlMalloc(initialSize)))
		ret->size = initialSize;
	else {
		xmlFree(ret);
		return NULL;
	}
	ret->grow_strategy = strategy;
	ret->increment = increment;
	ret->flush_callback = NULL;
	return ret;
}

ReszBufGrowStrategy getReszBufGrowStrategy(ReszBufPtr buf)
{
	if (buf)
		return buf->grow_strategy;
	return RESZ_BUF_GROW_UNKNOWN;
}

void setReszBufGrowStrategy(ReszBufPtr buf, ReszBufGrowStrategy strategy)
{
	if (buf)
		buf->grow_strategy = strategy;
}

size_t getReszBufGrowIncrement(ReszBufPtr buf)
{
	if (buf)
		return buf->increment;
	return -1;
}

void setReszBufGrowIncrement(ReszBufPtr buf, size_t increment)
{
	if (!buf)
		return;
	buf->increment = increment;
}

size_t getReszBufContentSize(ReszBufPtr buf)
{
	if (!buf)
		return 0;
	if (!buf->start)
		return 0;
	return (char*) buf->current - (char*) buf->start;
}

ReszBufFlushCallback getReszBufFlushCallback(ReszBufPtr buf)
{
	if (!buf)
		return NULL;
	return buf->flush_callback;
}
ReszBufFlushCallback setReszBufFlushCallback(ReszBufPtr buf, ReszBufFlushCallback cb)
{
	ReszBufFlushCallback old;
	if (!buf)
		return NULL;
	old = buf->flush_callback;
	buf->flush_callback = cb;
	return old;
}

void *getReszBufContent(ReszBufPtr buf)
{
	if (!buf)
		return NULL;
	return buf->start;
}

void *detachReszBufContent(ReszBufPtr buf)
{
	void *ret;
	if (!buf)
		return NULL;
	ret = buf->start;
	buf->start = buf->current = NULL;
	buf->size = 0;
	return ret;
}

ReszBufOpResult ensureReszBufFreeSize(ReszBufPtr buf, size_t minfree)
{
	size_t offset;
	void *start;

	if (!buf)
		return RESZ_BUF_RESULT_INVALID;
	offset = (char*) buf->current - (char*) buf->start;
	if (buf->size - offset >= minfree)
		return RESZ_BUF_RESULT_OK;

	switch (buf->grow_strategy)
	{
		case RESZ_BUF_GROW_FIXED:
			return RESZ_BUF_RESULT_NO_MEMORY;
			break;
		case RESZ_BUF_GROW_EXACT:
			buf->size += minfree - (buf->size - offset);
			break;
		case RESZ_BUF_GROW_INCREMENT:
			/* round to the increment */
			buf->size += ((size_t) (((minfree - 1.0*(buf->size - offset)) / buf->increment) + .5))*buf->increment;
			break;
		case RESZ_BUF_GROW_DOUBLE:
			while ((buf->size - offset) < minfree)
				buf->size *= 2;
			break;
		case RESZ_BUF_GROW_FLUSH:
			/* we can try processing this here if minfree <= buf->size */
			if (buf->size < minfree)
				return RESZ_BUF_RESULT_NO_MEMORY;
			if (!buf->flush_callback)
				return RESZ_BUF_RESULT_INVALID;
			if (buf->flush_callback(buf->start, offset) < 0)
				return RESZ_BUF_RESULT_FLUSH_FAILED;
			buf->current = buf->start;
			return RESZ_BUF_RESULT_OK;
			break;
		default: /* RESZ_BUF_GROW_UNKNOWN, impossible */
			return RESZ_BUF_RESULT_INVALID;
			break;
	}
	start = xmlRealloc(buf->start, buf->size);
	if (!start)
		return RESZ_BUF_RESULT_NO_MEMORY;
	buf->start = start;
	buf->current = (char*) buf->start + offset;

	return RESZ_BUF_RESULT_OK;
}

ReszBufOpResult addDataToReszBuf(ReszBufPtr buf, void* content, size_t size)
{
	ReszBufOpResult rslt;

	if (!buf)
		return RESZ_BUF_RESULT_INVALID;
	if (!buf->size || !buf->start)
		return RESZ_BUF_RESULT_NO_MEMORY;

	if (buf->grow_strategy == RESZ_BUF_GROW_FLUSH)
	{ /* special case. we need to flush the buffer instead of growing it */
		if (size > (buf->size - ((char*) buf->current - (char*) buf->start))) /* doesn't fit */
		{
			if (!buf->flush_callback)
				return RESZ_BUF_RESULT_INVALID;
			/* first flush current contents */
			/* TODO: maybe fill the buffer fully first? */
			if (buf->flush_callback(buf->start, (char*) buf->current - (char*) buf->start) < 0)
				return RESZ_BUF_RESULT_FLUSH_FAILED;
			/* then flush incoming data in batches */
			while (size > buf->size)
			{
				if (buf->flush_callback(content, buf->size) < 0)
					return RESZ_BUF_RESULT_FLUSH_FAILED;
				size -= buf->size;
				content = (char*) content + buf->size;
			}
			/* keep the remainder */
			if (size)
				memcpy(buf->start, content, size);
			buf->current = (char*) buf->start + size;
		} else {
			memcpy(buf->current, content, size);
			buf->current = (char*) buf->current + size;
		}
	} else {
		rslt = ensureReszBufFreeSize(buf, size);
		if (rslt != RESZ_BUF_RESULT_OK)
			return rslt;
		memcpy(buf->current, content, size);
		buf->current = (char*) buf->current + size;
	}
	return RESZ_BUF_RESULT_OK;
}

void rewindReszBuf(ReszBufPtr buf)
{
	if (!buf)
		return;
	buf->current = buf->start;
}

void* getReszBufPosition(ReszBufPtr buf)
{
	if (buf)
		return buf->current;
	return NULL;
}

ReszBufOpResult advanceReszBufferPosition(ReszBufPtr buf, size_t delta)
{
	if (!buf)
		return RESZ_BUF_RESULT_INVALID;
	if ((size_t)((char*) buf->current + delta - (char*) buf->start) > buf->size)
		return RESZ_BUF_RESULT_NO_MEMORY;
	buf->current = (char*) buf->current + delta;
	return RESZ_BUF_RESULT_OK;
}

ReszBufOpResult flushReszBuf(ReszBufPtr buf)
{
	if (!buf)
		return RESZ_BUF_RESULT_INVALID;
	if (buf->grow_strategy != RESZ_BUF_GROW_FLUSH)
		return RESZ_BUF_RESULT_INVALID;
	if (!buf->flush_callback)
		return RESZ_BUF_RESULT_INVALID;
	if (buf->flush_callback(buf->start, (char*) buf->current - (char*) buf->start) < 0)
		return RESZ_BUF_RESULT_FLUSH_FAILED;
	buf->current = buf->start;
	return RESZ_BUF_RESULT_OK;
}

void freeReszBuf(ReszBufPtr buf)
{
	if (!buf)
		return;
	if (buf->start)
		xmlFree(buf->start);
	xmlFree(buf);
}
