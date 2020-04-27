#include "Common.h"
#include "ReszBuf.h"

typedef struct _ReszBuf
{
	size_t size; /* maximal, not current */
	ReszBufGrowStrategy grow_strategy;
	size_t increment;
	void *start;
	void *current;
	ReszBufFlushCallback flush_callback;
	size_t original_size; /* if content is detached, allocate the same amount on next write call */
} ReszBuf, *ReszBufPtr;

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
	ReszBufPtr ret;

	if (strategy < RESZ_BUF_GROW_MIN || strategy > RESZ_BUF_GROW_MAX)
		return NULL;
	if (strategy == RESZ_BUF_GROW_INCREMENT && !increment)
		return NULL;
	ret = (ReszBufPtr) xmlMalloc(sizeof(ReszBuf));
	if (!ret)
		return NULL;
	if (initialSize)
	{
		if ((ret->start = ret->current = xmlMalloc(initialSize)))
			ret->original_size = ret->size = initialSize;
		else {
			xmlFree(ret);
			return NULL;
		}
	} else {
		ret->original_size = ret->size = 0;
		ret->current = ret->start = NULL;
	}
	ret->grow_strategy = strategy;
	ret->increment = increment;
	ret->flush_callback = NULL;
	return ret;
}

ReszBufPtr createFlushableReszBuf(size_t size, ReszBufFlushCallback cb)
{
	ReszBufPtr result;

	if (!cb)
		return NULL;
	result = createReszBufParams(size, RESZ_BUF_GROW_FLUSH, 0);
	if (!result)
		return NULL;
	result->flush_callback = cb;
	return result;
}

ReszBufGrowStrategy getReszBufGrowStrategy(ReszBufPtr buf)
{
	if (buf)
		return buf->grow_strategy;
	return RESZ_BUF_GROW_UNKNOWN;
}

ReszBufOpResult setReszBufGrowStrategy(ReszBufPtr buf, ReszBufGrowStrategy strategy)
{
	if (!buf)
		return RESZ_BUF_RESULT_INVALID;
	if (strategy < RESZ_BUF_GROW_MIN || strategy > RESZ_BUF_GROW_MAX)
		return RESZ_BUF_RESULT_INVALID;
	if (strategy == RESZ_BUF_GROW_INCREMENT && !buf->increment)
		return RESZ_BUF_RESULT_INVALID;
	buf->grow_strategy = strategy;
	return RESZ_BUF_RESULT_OK;
}

size_t getReszBufGrowIncrement(ReszBufPtr buf)
{
	if (buf)
		return buf->increment;
	return 0;
}

ReszBufOpResult setReszBufGrowIncrement(ReszBufPtr buf, size_t increment)
{
	if (!buf)
		return RESZ_BUF_RESULT_INVALID;
	if (buf->grow_strategy == RESZ_BUF_GROW_INCREMENT && !increment)
		return RESZ_BUF_RESULT_INVALID;
	buf->increment = increment;
	return RESZ_BUF_RESULT_OK;
}

size_t getReszBufContentSize(ReszBufPtr buf)
{
	if (!buf)
		return 0;
	if (!buf->start)
		return 0;
	return (char*) buf->current - (char*) buf->start;
}

size_t getReszBufFreeSpace(ReszBufPtr buf)
{
	if (!buf || !buf->start)
		return 0;
	return buf->size - ((char*) buf->current - (char*) buf->start);
}

size_t getReszBufTotalSize(ReszBufPtr buf)
{
	if (!buf)
		return 0;
	return buf->size;
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
	buf->original_size = buf->size;
	buf->size = 0;
	return ret;
}

ReszBufOpResult ensureReszBufFreeSize(ReszBufPtr buf, size_t minfree)
{
	size_t offset, required;
	void *start;

	if (!buf)
		return RESZ_BUF_RESULT_INVALID;
	if (buf->start)
	{
		offset = (char*) buf->current - (char*) buf->start;
		if (buf->size - offset >= minfree)
			return RESZ_BUF_RESULT_OK;
	} else
		offset = 0;
	switch (buf->grow_strategy)
	{
		case RESZ_BUF_GROW_FIXED:
			if (!buf->size)
			{
				if (!(buf->start = buf->current = xmlMalloc(buf->original_size)))
					return RESZ_BUF_RESULT_NO_MEMORY;
				buf->size = buf->original_size;
				return RESZ_BUF_RESULT_OK;
			}
			return RESZ_BUF_RESULT_NO_MEMORY;
			break;
		case RESZ_BUF_GROW_EXACT:
			buf->size += minfree - (buf->size - offset);
			break;
		case RESZ_BUF_GROW_INCREMENT:
			/* round to the increment */
			required = minfree - (buf->size - offset);
			buf->size += buf->increment*(required / buf->increment + ((required % buf->increment)? 1: 0));
			break;
		case RESZ_BUF_GROW_DOUBLE:
			if (!buf->size)
				buf->size = 1;
			while ((buf->size - offset) < minfree)
				buf->size *= 2;
			break;
		case RESZ_BUF_GROW_FLUSH:
			/* we can try processing this here if minfree <= buf->size */
			if (buf->size < minfree)
				return RESZ_BUF_RESULT_NO_MEMORY;
			if (!buf->flush_callback)
				return RESZ_BUF_RESULT_INVALID;
			if (buf->start)
			{
				if (buf->flush_callback(buf->start, offset) < 0)
					return RESZ_BUF_RESULT_FLUSH_FAILED;
				buf->current = buf->start;
			} else
				buf->size = buf->original_size;
			return RESZ_BUF_RESULT_OK;
		default: /* something is wrong here */
			return RESZ_BUF_RESULT_INVALID;
	}
	if (!buf->start)
		start = xmlMalloc(buf->size);
	else
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
	size_t free_space;

	if (!buf)
		return RESZ_BUF_RESULT_INVALID;

	if (buf->grow_strategy == RESZ_BUF_GROW_FLUSH)
	{ /* special case. we need to flush the buffer instead of growing it */
		if (!buf->start)
		{
			if (!(buf->start = buf->current = xmlMalloc(buf->original_size)))
				return RESZ_BUF_RESULT_NO_MEMORY;
			buf->size = buf->original_size;
		}
		free_space = buf->size - ((char*) buf->current - (char*) buf->start);
		if (size > free_space) /* doesn't fit */
		{
			if (!buf->flush_callback)
				return RESZ_BUF_RESULT_INVALID;
			if (free_space < buf->size) /* something is already in, let's combine then flush buf->size bytes */
			{
				memcpy(buf->current, content, free_space);
				buf->current = (char*) buf->start + buf->size;
				if (buf->flush_callback(buf->start, buf->size) < 0)
					return RESZ_BUF_RESULT_FLUSH_FAILED;
				size -= free_space;
				content = ((char*) content) + free_space;
				buf->current = buf->start;
			}
			while (size > buf->size)
			{
				if (buf->flush_callback(content, buf->size) < 0)
					return RESZ_BUF_RESULT_FLUSH_FAILED;
				size -= buf->size;
				content = ((char*) content) + buf->size;
			}
		}
	} else {
		rslt = ensureReszBufFreeSize(buf, size);
		if (rslt != RESZ_BUF_RESULT_OK)
			return rslt;
	}
	memcpy(buf->current, content, size);
	buf->current = (char*) buf->current + size;
	return RESZ_BUF_RESULT_OK;
}

ReszBufOpResult rewindReszBuf(ReszBufPtr buf)
{
	if (!buf)
		return RESZ_BUF_RESULT_INVALID;
	buf->current = buf->start;
	return RESZ_BUF_RESULT_OK;
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
