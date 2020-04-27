#include "Common.h"
#include "ReszBuf.h"

typedef struct _rbBuf
{
	size_t size; /* maximal, not current */
	rbGrowStrategy grow_strategy;
	size_t increment;
	void *start;
	void *current;
	rbFlushCallback flush_callback;
	size_t original_size; /* if content is detached, allocate the same amount on next write call */
} rbBuf, *rbBufPtr;

rbBufPtr rbCreateBuf()
{
	return rbCreateBufParams(RB_DEFAULT_INITIAL_SIZE, RB_DEFAULT_GROW_STRATEGY, RB_DEFAULT_GROW_INCREMENT);
}

rbBufPtr rbCreateBufSize(size_t initialSize)
{
	return rbCreateBufParams(initialSize, RB_DEFAULT_GROW_STRATEGY, RB_DEFAULT_GROW_INCREMENT);
}

rbBufPtr rbCreateBufParams(size_t initialSize, rbGrowStrategy strategy, size_t increment)
{
	rbBufPtr ret;

	if (strategy < RB_GROW_MIN || strategy > RB_GROW_MAX)
		return NULL;
	if (strategy == RB_GROW_INCREMENT && !increment)
		return NULL;
	ret = (rbBufPtr) xmlMalloc(sizeof(rbBuf));
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

rbBufPtr rbCreateFlushableBuf(size_t size, rbFlushCallback cb)
{
	rbBufPtr result;

	if (!cb)
		return NULL;
	result = rbCreateBufParams(size, RB_GROW_FLUSH, 0);
	if (!result)
		return NULL;
	result->flush_callback = cb;
	return result;
}

rbGrowStrategy rbGetBufGrowStrategy(rbBufPtr buf)
{
	if (buf)
		return buf->grow_strategy;
	return RB_GROW_UNKNOWN;
}

rbOpResult rbSetBufGrowStrategy(rbBufPtr buf, rbGrowStrategy strategy)
{
	if (!buf)
		return RB_RESULT_INVALID;
	if (strategy < RB_GROW_MIN || strategy > RB_GROW_MAX)
		return RB_RESULT_INVALID;
	if (strategy == RB_GROW_INCREMENT && !buf->increment)
		return RB_RESULT_INVALID;
	buf->grow_strategy = strategy;
	return RB_RESULT_OK;
}

size_t rbGetBufGrowIncrement(rbBufPtr buf)
{
	if (buf)
		return buf->increment;
	return 0;
}

rbOpResult rbSetBufGrowIncrement(rbBufPtr buf, size_t increment)
{
	if (!buf)
		return RB_RESULT_INVALID;
	if (buf->grow_strategy == RB_GROW_INCREMENT && !increment)
		return RB_RESULT_INVALID;
	buf->increment = increment;
	return RB_RESULT_OK;
}

size_t rbGetBufContentSize(rbBufPtr buf)
{
	if (!buf)
		return 0;
	if (!buf->start)
		return 0;
	return (char*) buf->current - (char*) buf->start;
}

size_t rbGetBufFreeSpace(rbBufPtr buf)
{
	if (!buf || !buf->start)
		return 0;
	return buf->size - ((char*) buf->current - (char*) buf->start);
}

size_t rbGetBufTotalSize(rbBufPtr buf)
{
	if (!buf)
		return 0;
	return buf->size;
}

rbFlushCallback rbGetBufFlushCallback(rbBufPtr buf)
{
	if (!buf)
		return NULL;
	return buf->flush_callback;
}

rbFlushCallback rbSetBufFlushCallback(rbBufPtr buf, rbFlushCallback cb)
{
	rbFlushCallback old;

	if (!buf)
		return NULL;
	old = buf->flush_callback;
	buf->flush_callback = cb;
	return old;
}

void *rbGetBufContent(rbBufPtr buf)
{
	if (!buf)
		return NULL;
	return buf->start;
}

void *rbDetachBufContent(rbBufPtr buf)
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

rbOpResult rbEnsureBufFreeSize(rbBufPtr buf, size_t minfree)
{
	size_t offset, required;
	void *start;

	if (!buf)
		return RB_RESULT_INVALID;
	if (buf->start)
	{
		offset = (char*) buf->current - (char*) buf->start;
		if (buf->size - offset >= minfree)
			return RB_RESULT_OK;
	} else
		offset = 0;
	switch (buf->grow_strategy)
	{
		case RB_GROW_FIXED:
			if (!buf->size)
			{
				if (!(buf->start = buf->current = xmlMalloc(buf->original_size)))
					return RB_RESULT_NO_MEMORY;
				buf->size = buf->original_size;
				return RB_RESULT_OK;
			}
			return RB_RESULT_NO_MEMORY;
			break;
		case RB_GROW_EXACT:
			buf->size += minfree - (buf->size - offset);
			break;
		case RB_GROW_INCREMENT:
			/* round to the increment */
			required = minfree - (buf->size - offset);
			buf->size += buf->increment*(required / buf->increment + ((required % buf->increment)? 1: 0));
			break;
		case RB_GROW_DOUBLE:
			if (!buf->size)
				buf->size = 1;
			while ((buf->size - offset) < minfree)
				buf->size *= 2;
			break;
		case RB_GROW_FLUSH:
			/* we can try processing this here if minfree <= buf->size */
			if (buf->size < minfree)
				return RB_RESULT_NO_MEMORY;
			if (!buf->flush_callback)
				return RB_RESULT_INVALID;
			if (buf->start)
			{
				if (buf->flush_callback(buf->start, offset) < 0)
					return RB_RESULT_FLUSH_FAILED;
				buf->current = buf->start;
			} else
				buf->size = buf->original_size;
			return RB_RESULT_OK;
		default: /* something is wrong here */
			return RB_RESULT_INVALID;
	}
	if (!buf->start)
		start = xmlMalloc(buf->size);
	else
		start = xmlRealloc(buf->start, buf->size);
	if (!start)
		return RB_RESULT_NO_MEMORY;
	buf->start = start;
	buf->current = (char*) buf->start + offset;

	return RB_RESULT_OK;
}

rbOpResult rbAddDataToBuf(rbBufPtr buf, void* content, size_t size)
{
	rbOpResult rslt;
	size_t free_space;

	if (!buf)
		return RB_RESULT_INVALID;

	if (buf->grow_strategy == RB_GROW_FLUSH)
	{ /* special case. we need to flush the buffer instead of growing it */
		if (!buf->start)
		{
			if (!(buf->start = buf->current = xmlMalloc(buf->original_size)))
				return RB_RESULT_NO_MEMORY;
			buf->size = buf->original_size;
		}
		free_space = buf->size - ((char*) buf->current - (char*) buf->start);
		if (size > free_space) /* doesn't fit */
		{
			if (!buf->flush_callback)
				return RB_RESULT_INVALID;
			if (free_space < buf->size) /* something is already in, let's combine then flush buf->size bytes */
			{
				memcpy(buf->current, content, free_space);
				buf->current = (char*) buf->start + buf->size;
				if (buf->flush_callback(buf->start, buf->size) < 0)
					return RB_RESULT_FLUSH_FAILED;
				size -= free_space;
				content = ((char*) content) + free_space;
				buf->current = buf->start;
			}
			while (size > buf->size)
			{
				if (buf->flush_callback(content, buf->size) < 0)
					return RB_RESULT_FLUSH_FAILED;
				size -= buf->size;
				content = ((char*) content) + buf->size;
			}
		}
	} else {
		rslt = rbEnsureBufFreeSize(buf, size);
		if (rslt != RB_RESULT_OK)
			return rslt;
	}
	memcpy(buf->current, content, size);
	buf->current = (char*) buf->current + size;
	return RB_RESULT_OK;
}

rbOpResult rbRewindBuf(rbBufPtr buf)
{
	if (!buf)
		return RB_RESULT_INVALID;
	buf->current = buf->start;
	return RB_RESULT_OK;
}

void* rbGetBufPosition(rbBufPtr buf)
{
	if (buf)
		return buf->current;
	return NULL;
}

rbOpResult rbAdvanceBufPosition(rbBufPtr buf, size_t delta)
{
	if (!buf)
		return RB_RESULT_INVALID;
	if ((size_t)((char*) buf->current + delta - (char*) buf->start) > buf->size)
		return RB_RESULT_NO_MEMORY;
	buf->current = (char*) buf->current + delta;
	return RB_RESULT_OK;
}

rbOpResult rbFlushBuf(rbBufPtr buf)
{
	if (!buf)
		return RB_RESULT_INVALID;
	if (buf->grow_strategy != RB_GROW_FLUSH)
		return RB_RESULT_INVALID;
	if (!buf->flush_callback)
		return RB_RESULT_INVALID;
	if (buf->flush_callback(buf->start, (char*) buf->current - (char*) buf->start) < 0)
		return RB_RESULT_FLUSH_FAILED;
	buf->current = buf->start;
	return RB_RESULT_OK;
}

void rbFreeBuf(rbBufPtr buf)
{
	if (!buf)
		return;
	if (buf->start)
		xmlFree(buf->start);
	xmlFree(buf);
}
