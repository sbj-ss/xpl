#include "Test_ReszBuf.h"
#include <assert.h>
#include <libxpl/xplbuffer.h>

#define FAIL(x) do {\
	ctxt->error = BAD_CAST x;\
	goto cleanup;\
} while(0);

static bool xtsTestReszBuf_CreateDefaultParams(xtsContextPtr ctxt)
{
	rbBufPtr buf;
	bool ok = false;

	buf = rbCreateBuf();
	assert(buf);
	if (rbGetBufTotalSize(buf) != RB_DEFAULT_INITIAL_SIZE)
		FAIL("wrong buffer size");
	if (rbGetBufContentSize(buf) != 0)
		FAIL("initial content size must be zero");
	if (rbGetBufGrowStrategy(buf) != RB_DEFAULT_GROW_STRATEGY)
		FAIL("wrong grow strategy");
	if (rbGetBufGrowIncrement(buf) != RB_DEFAULT_GROW_INCREMENT)
		FAIL("wrong increment");
	ok = true;
cleanup:
	rbFreeBuf(buf);
	return ok;
}

static bool xtsTestReszBuf_CreateWithSize(xtsContextPtr ctxt)
{
	rbBufPtr buf;
	bool ok = false;

	buf = rbCreateBufSize(1234);
	assert(buf);
	if (rbGetBufTotalSize(buf) != 1234)
		FAIL("wrong buffer size");
	if (rbGetBufGrowStrategy(buf) != RB_DEFAULT_GROW_STRATEGY)
		FAIL("wrong grow strategy");
	if (rbGetBufGrowIncrement(buf) != RB_DEFAULT_GROW_INCREMENT)
		FAIL("wrong increment");
	ok = true;
cleanup:
	rbFreeBuf(buf);
	return ok;
}

static bool xtsTestReszBuf_CreateExplicit(xtsContextPtr ctxt)
{
	rbBufPtr buf;
	bool ok = false;

	buf = rbCreateBufParams(1234, RB_GROW_INCREMENT, 321);
	assert(buf);
	if (rbGetBufTotalSize(buf) != 1234)
		FAIL("wrong buffer size");
	if (rbGetBufGrowStrategy(buf) != RB_GROW_INCREMENT)
		FAIL("wrong grow strategy");
	if (rbGetBufGrowIncrement(buf) != 321)
		FAIL("wrong increment");
	ok = true;
cleanup:
	rbFreeBuf(buf);
	return ok;
}

static int flush_to_dev_null_and_succeed(void *data, size_t size)
{
	return 0;
}

static bool xtsTestReszBuf_CreateFlushable(xtsContextPtr ctxt)
{
	rbBufPtr buf;
	bool ok = false;

	buf = rbCreateFlushableBuf(1234, NULL);
	if (buf)
		FAIL("creating a flushable buffer without flush callback must fail");
	buf = rbCreateFlushableBuf(2345, flush_to_dev_null_and_succeed);
	assert(buf);
	if (rbGetBufTotalSize(buf) != 2345)
		FAIL("wrong buffer size");
	if (rbGetBufGrowStrategy(buf) != RB_GROW_FLUSH)
		FAIL("wrong grow strategy");
	ok = true;
cleanup:
	rbFreeBuf(buf);
	return ok;
}

static unsigned char src_bytes[RB_DEFAULT_INITIAL_SIZE] = { 0 };

static bool xtsTestReszBuf_GrowExact(xtsContextPtr ctxt)
{
	rbBufPtr buf;
	bool ok = false;

	buf = rbCreateBufParams(16, RB_GROW_EXACT, 0);
	assert(buf);
	if (rbAddDataToBuf(buf, src_bytes, 8) != RB_RESULT_OK)
		FAIL("adding data failed (1)");
	if (rbGetBufContentSize(buf) != 8)
		FAIL("wrong content size (1)");
	if (rbGetBufTotalSize(buf) != 16)
		FAIL("wrong total size (1)");
	if (rbGetBufFreeSpace(buf) != 8)
		FAIL("wrong free space (1)");
	if (rbAddDataToBuf(buf, src_bytes, 16) != RB_RESULT_OK)
		FAIL("adding data failed (2)");
	if (rbGetBufContentSize(buf) != 24)
		FAIL("wrong content size (2)");
	if (rbGetBufTotalSize(buf) != 24)
		FAIL("wrong total size (2)");
	if (rbGetBufFreeSpace(buf) != 0)
		FAIL("wrong free space (2)");
	ok = true;
cleanup:
	rbFreeBuf(buf);
	return ok;
}

static bool xtsTestReszBuf_GrowIncrement(xtsContextPtr ctxt)
{
	rbBufPtr buf;
	bool ok = false;

	buf = rbCreateBufParams(16, RB_GROW_INCREMENT, 24);
	assert(buf);
	if (rbAddDataToBuf(buf, src_bytes, 8) != RB_RESULT_OK)
		FAIL("adding data failed (1)");
	if (rbGetBufContentSize(buf) != 8)
		FAIL("wrong content size (1)");
	if (rbGetBufTotalSize(buf) != 16)
		FAIL("wrong total size (1)");
	if (rbGetBufFreeSpace(buf) != 8)
		FAIL("wrong free space (1)");
	if (rbAddDataToBuf(buf, src_bytes, 16) != RB_RESULT_OK)
		FAIL("adding data failed (2)");
	if (rbGetBufContentSize(buf) != 24)
		FAIL("wrong content size (2)");
	if (rbGetBufTotalSize(buf) != 40)
		FAIL("wrong total size (2)");
	if (rbGetBufFreeSpace(buf) != 16)
		FAIL("wrong free space (2)");
	ok = true;
cleanup:
	rbFreeBuf(buf);
	return ok;
}

static bool xtsTestReszBuf_GrowDouble(xtsContextPtr ctxt)
{
	rbBufPtr buf;
	bool ok = false;

	buf = rbCreateBufParams(16, RB_GROW_DOUBLE, 0);
	assert(buf);
	if (rbAddDataToBuf(buf, src_bytes, 8) != RB_RESULT_OK)
		FAIL("adding data failed (1)");
	if (rbGetBufContentSize(buf) != 8)
		FAIL("wrong content size (1)");
	if (rbGetBufTotalSize(buf) != 16)
		FAIL("wrong total size (1)");
	if (rbGetBufFreeSpace(buf) != 8)
		FAIL("wrong free space (1)");
	if (rbAddDataToBuf(buf, src_bytes, 16) != RB_RESULT_OK)
		FAIL("adding data failed (2)");
	if (rbGetBufContentSize(buf) != 24)
		FAIL("wrong content size (2)");
	if (rbGetBufTotalSize(buf) != 32)
		FAIL("wrong total size (2)");
	if (rbGetBufFreeSpace(buf) != 8)
		FAIL("wrong free space (2)");
	ok = true;
cleanup:
	rbFreeBuf(buf);
	return ok;
}

static bool xtsTestReszBuf_GrowFixed(xtsContextPtr ctxt)
{
	rbBufPtr buf;
	bool ok = false;

	buf = rbCreateBufParams(16, RB_GROW_FIXED, 0);
	assert(buf);
	if (rbAddDataToBuf(buf, src_bytes, 8) != RB_RESULT_OK)
		FAIL("adding data failed (1)");
	if (rbGetBufContentSize(buf) != 8)
		FAIL("wrong content size (1)");
	if (rbGetBufTotalSize(buf) != 16)
		FAIL("wrong total size (1)");
	if (rbGetBufFreeSpace(buf) != 8)
		FAIL("wrong free space (1)");
	if (rbAddDataToBuf(buf, src_bytes, 16) != RB_RESULT_NO_MEMORY)
		FAIL("rbAddDataToBuf() must fail if size is fixed and data don't fit");
	if (rbGetBufContentSize(buf) != 8)
		FAIL("wrong content size (2)");
	if (rbGetBufTotalSize(buf) != 16)
		FAIL("wrong total size (2)");
	if (rbGetBufFreeSpace(buf) != 8)
		FAIL("wrong free space (2)");
	ok = true;
cleanup:
	rbFreeBuf(buf);
	return ok;
}

static int flush_calls = 0;

static int flush_to_dev_null(void *data, size_t size)
{
	flush_calls++;
	return 0;
}

static bool xtsTestReszBuf_GrowFlush(xtsContextPtr ctxt)
{
	rbBufPtr buf;
	bool ok = false;

	buf = rbCreateFlushableBuf(16, flush_to_dev_null);
	assert(buf);

	flush_calls = 0;
	if (rbAddDataToBuf(buf, src_bytes, 8) != RB_RESULT_OK)
		FAIL("adding data failed (1)");
	if (rbGetBufContentSize(buf) != 8)
		FAIL("wrong content size (1)");
	if (rbGetBufTotalSize(buf) != 16)
		FAIL("wrong total size (1)");
	if (rbGetBufFreeSpace(buf) != 8)
		FAIL("wrong free space (1)");
	if (flush_calls != 0)
		FAIL("wrong flush calls count (1)");

	flush_calls = 0;
	if (rbAddDataToBuf(buf, src_bytes, 16) != RB_RESULT_OK)
		FAIL("adding data failed (2)");
	if (rbGetBufContentSize(buf) != 8)
		FAIL("wrong content size (2)");
	if (rbGetBufTotalSize(buf) != 16)
		FAIL("wrong total size (2)");
	if (rbGetBufFreeSpace(buf) != 8)
		FAIL("wrong free space (2)");
	if (flush_calls != 1)
		FAIL("wrong flush calls count (2)");

	flush_calls = 0;
	if (rbAddDataToBuf(buf, src_bytes, 56) != RB_RESULT_OK)
		FAIL("adding data failed (3)");
	if (rbGetBufContentSize(buf) != 16)
		FAIL("wrong content size (3)");
	if (rbGetBufTotalSize(buf) != 16)
		FAIL("wrong total size (3)");
	if (rbGetBufFreeSpace(buf) != 0)
		FAIL("wrong free space (3)");
	if (flush_calls != 3)
		FAIL("wrong flush calls count (3)");
	ok = true;
cleanup:
	rbFreeBuf(buf);
	return ok;
}

static int flush_to_dev_null_and_fail(void *data, size_t size)
{
	return -1;
}

static xmlChar *flush_buf = NULL;
static int flush_to_strcat(void *data, size_t size)
{
	if (flush_buf)
		flush_buf = xmlStrncat(flush_buf, data, size);
	else
		flush_buf = xmlStrndup(data, size);
	return 0;
}

#define ADD_DATA(s) rbAddDataToBuf(buf, s, xmlStrlen(BAD_CAST s))

static bool xtsTestReszBuf_Flush(xtsContextPtr ctxt)
{
	rbBufPtr buf;
	bool ok = false;

	buf = rbCreateFlushableBuf(8, flush_to_dev_null_and_fail);
	assert(buf);
	if (rbGetBufFlushCallback(buf) != flush_to_dev_null_and_fail)
		FAIL("getting flush callback failed");
	if (rbAddDataToBuf(buf, src_bytes, 2) != RB_RESULT_OK)
		FAIL("adding data must succeed if they fit");
	if (rbFlushBuf(buf) != RB_RESULT_FLUSH_FAILED)
		FAIL("flushing must fail if callback reports an error");
	if (rbAddDataToBuf(buf, src_bytes, 10) != RB_RESULT_FLUSH_FAILED)
		FAIL("adding data  must fail if flush callback reports an error");
	if (rbSetBufFlushCallback(buf, flush_to_strcat) != flush_to_dev_null_and_fail)
		FAIL("setting flush callback failed");
	if (rbGetBufFlushCallback(buf) != flush_to_strcat)
		FAIL("getting flush callback failed (2)");
	ADD_DATA("This");
	if (!xmlStrcmp(flush_buf, "This"))
		FAIL("adding data failed (1)");
	ADD_DATA(" is");
	if (!xmlStrcmp(flush_buf, "This is"))
		FAIL("adding data failed (2)");
	ADD_DATA(" a");
	if (!xmlStrcmp(flush_buf, "This is a"))
		FAIL("adding data failed (3)");
	ADD_DATA(" test");
	if (!xmlStrcmp(flush_buf, "This is a test"))
		FAIL("adding data failed (4)");
	ok = true;
cleanup:
	rbFreeBuf(buf);
	if (flush_buf)
	{
		xmlFree(flush_buf);
		flush_buf = NULL;
	}
	return ok;
}

#undef ADD_DATA

static bool xtsTestReszBuf_InvalidParams(xtsContextPtr ctxt)
{
	bool ok = false;
	rbBufPtr buf;

	if ((buf = rbCreateBufParams(10, RB_GROW_MIN - 1, 10)))
		FAIL("rbCreateBufParams() must fail if strategy < RESZ_BUF_GROW_MIN");
	if ((buf = rbCreateBufParams(10, RB_GROW_MAX + 1, 10)))
		FAIL("rbCreateBufParams() must fail if strategy > RESZ_BUF_GROW_MAX");
	if ((buf = rbCreateBufParams(10, RB_GROW_INCREMENT, 0)))
		FAIL("rbCreateBufParams() must fail if grow_strategy = RESZ_BUF_GROW_INCREMENT and increment = 0");
	if ((buf = rbCreateFlushableBuf(10, NULL)))
		FAIL("rbCreateFlashableBuf() must fail if cb is NULL");

	buf = rbCreateBufParams(10, RB_GROW_INCREMENT, 10);
	assert(buf);
	if (rbSetBufGrowStrategy(buf, RB_GROW_MIN - 1) != RB_RESULT_INVALID)
		FAIL("rbSetBufGrowStrategy() must fail if strategy < RESZ_BUF_GROW_MIN");
	if (rbSetBufGrowStrategy(buf, RB_GROW_MAX + 1) != RB_RESULT_INVALID)
		FAIL("rbSetBufGrowStrategy() must fail if strategy > RESZ_BUF_GROW_MAX");
	if (rbSetBufGrowIncrement(buf, 0) != RB_RESULT_INVALID)
		FAIL("rbSetBufGrowIncrement() must fail if strategy = RESZ_BUF_GROW_INCREMENT and increment = 0");
	if (rbSetBufGrowStrategy(buf, RB_GROW_EXACT) != RB_RESULT_OK)
		FAIL("rbSetBufGrowStrategy() failed on valid input");
	if (rbSetBufGrowIncrement(buf, 0) != RB_RESULT_OK)
		FAIL("rbSetBufGrowIncrement() failed on valid input");
	if (rbSetBufGrowStrategy(buf, RB_GROW_INCREMENT) != RB_RESULT_INVALID)
		FAIL("rbSetBufGrowStrategy() must fail if increment is 0 and strategy = RESZ_BUF_GROW_INCREMENT");
	if (rbAdvanceBufPosition(buf, 1000) != RB_RESULT_NO_MEMORY)
		FAIL("rbAdvanceBufPosition() must fail on positions beyond the buffer end");
	ok = true;
cleanup:
	if (buf)
		rbFreeBuf(buf);
	return ok;
}

static bool xtsTestReszBuf_InvalidBuf(xtsContextPtr ctxt)
{
	bool ok = false;

	if (rbGetBufGrowStrategy(NULL) != RB_GROW_UNKNOWN)
		FAIL("rbGetBufGrowStrategy() must fail on NULL buffer");
	if (rbSetBufGrowStrategy(NULL, RB_GROW_EXACT) != RB_RESULT_INVALID)
		FAIL("rbSetBufGrowStrategy() must fail on NULL buffer");
	if (rbGetBufGrowIncrement(NULL) != 0)
		FAIL("rbGetBufGrowIncrement() must return 0 on NULL buffer");
	if (rbSetBufGrowIncrement(NULL, 10) != RB_RESULT_INVALID)
		FAIL("rbSetBufGrowIncrement() must fail on NULL buffer");
	if (rbGetBufContentSize(NULL) != 0)
		FAIL("rbGetBufContentSize() must return 0 on NULL buffer");
	if (rbGetBufFreeSpace(NULL) != 0)
		FAIL("rbGetBufFreeSpace() must return 0 on NULL buffer");
	if (rbGetBufTotalSize(NULL) != 0)
		FAIL("rbGetBufTotalSize() must return 0 on NULL buffer");
	if (rbGetBufFlushCallback(NULL) != NULL)
		FAIL("rbGetBufFlushCallback() must return NULL on NULL buffer");
	if (rbGetBufContent(NULL) != NULL)
		FAIL("rbGetBufContent() must return NULL on NULL buffer");
	if (rbDetachBufContent(NULL) != NULL)
		FAIL("rbDetachBufContent() must return NULL on NULL buffer");
	if (rbAddDataToBuf(NULL, src_bytes, 1) != RB_RESULT_INVALID)
		FAIL("rbAddDataToBuf() must fail on NULL buffer");
	if (rbRewindBuf(NULL) != RB_RESULT_INVALID)
		FAIL("rbRewindBuf() must fail on NULL buffer");
	if (rbAdvanceBufPosition(NULL, 1) != RB_RESULT_INVALID)
		FAIL("rbAdvanceBufPosition() must fail on NULL buffer");
	if (rbEnsureBufFreeSize(NULL, 10) != RB_RESULT_INVALID)
		FAIL("rbEnsureBufFreeSize() must fail on NULL buffer");
	if (rbFlushBuf(NULL) != RB_RESULT_INVALID)
		FAIL("rbFlushBuf() must fail on NULL buffer");
	ok = true;
cleanup:
	return ok;
}

static bool xtsTestReszBuf_GrowDynamic(xtsContextPtr ctxt)
{
	rbBufPtr buf;
	bool ok = false;

	buf = rbCreateBufParams(16, RB_GROW_FIXED, 0);
	assert(buf);
	if (rbAddDataToBuf(buf, src_bytes, 16) != RB_RESULT_OK)
		FAIL("adding size <= free_space to a fixed buffer must succeed");
	if (rbAddDataToBuf(buf, src_bytes, 1) != RB_RESULT_NO_MEMORY)
		FAIL("adding size > free_space to a fixed buffer must fail");
	if (rbSetBufGrowStrategy(buf, RB_GROW_EXACT) != RB_RESULT_OK)
		FAIL("setting grow_strategy to exact must succeed");
	if (rbAddDataToBuf(buf, src_bytes, 1) != RB_RESULT_OK)
		FAIL("adding to a growable buffer must succeed:1");
	if (rbGetBufContentSize(buf) != 17)
		FAIL("content size must be 17 at this point");
	if (rbGetBufTotalSize(buf) != 17)
		FAIL("total size must be 17 at this point");

	if (rbSetBufGrowIncrement(buf, 16) != RB_RESULT_OK)
		FAIL("setting grow_increment >= 0 must succeed");
	if (rbSetBufGrowStrategy(buf, RB_GROW_INCREMENT) != RB_RESULT_OK)
		FAIL("setting grow_strategy to incremental must succeed");
	if (rbAddDataToBuf(buf, src_bytes, 1) != RB_RESULT_OK)
		FAIL("adding to a growable buffer must succeed:2");
	if (rbGetBufContentSize(buf) != 18)
		FAIL("content size must be 18 at this point");
	if (rbGetBufTotalSize(buf) != 33) /* 17 + 16*1 */
		FAIL("total size must be 33 at this point");

	if (rbSetBufGrowStrategy(buf, RB_GROW_DOUBLE) != RB_RESULT_OK)
		FAIL("setting grow_strategy to doubling must succeed");
	if (rbAddDataToBuf(buf, src_bytes, 16) != RB_RESULT_OK)
		FAIL("adding to a growable buffer must succeed:3");
	if (rbGetBufContentSize(buf) != 34)
		FAIL("content size must be 34 at this point");
	if (rbGetBufTotalSize(buf) != 66) /* 33*2 */
		FAIL("total size must be 66 at this point");

	if (rbSetBufGrowStrategy(buf, RB_GROW_DOUBLE) != RB_RESULT_OK)
		FAIL("setting grow_strategy to doubling must succeed");
	if (rbAddDataToBuf(buf, src_bytes, 16) != RB_RESULT_OK)
		FAIL("adding to a growable buffer must succeed:3");
	if (rbGetBufContentSize(buf) != 50)
		FAIL("content size must be 50 at this point");
	if (rbGetBufTotalSize(buf) != 66) /* 33*2 */
		FAIL("total size must be 66 at this point:1");

	if (rbSetBufFlushCallback(buf, flush_to_dev_null_and_succeed) != RB_RESULT_OK)
		FAIL("setting buffer flush function must succeed");
	if (rbSetBufGrowStrategy(buf, RB_GROW_FLUSH) != RB_RESULT_OK)
		FAIL("setting grow_strategy to flushing must succeed");
	if (rbAddDataToBuf(buf, src_bytes, 32) != RB_RESULT_OK)
			FAIL("adding to a growable buffer must succeed:4");
	if (rbGetBufContentSize(buf) != 16)
		FAIL("content size must be 16 at this point");
	if (rbGetBufTotalSize(buf) != 66) /* 33*2 */
		FAIL("total size must be 66 at this point:2");

	ok = true;
cleanup:
	rbFreeBuf(buf);
	return ok;
}

static bool xtsTestReszBuf_DetachContent_Fixed(xtsContextPtr ctxt)
{
	rbBufPtr buf;
	void *content;
	bool ok = false;

	buf = rbCreateBufParams(16, RB_GROW_FIXED, 0);
	assert(buf);
	if (rbAddDataToBuf(buf, src_bytes, 8) != RB_RESULT_OK)
		FAIL("adding data to a growable buffer must succeed");
	if (!(content = rbDetachBufContent(buf)))
		FAIL("detached content must not be NULL");
	xmlFree(content);
	if (rbAddDataToBuf(buf, src_bytes, 9) != RB_RESULT_OK)
		FAIL("adding data after detaching content must succeed");
	if (rbGetBufContentSize(buf) != 9)
		FAIL("content size must be 9");
	if (rbGetBufTotalSize(buf) != 16)
		FAIL("total size must be 16");
	ok = true;
cleanup:
	rbFreeBuf(buf);
	return ok;
}

static bool xtsTestReszBuf_DetachContent_Exact(xtsContextPtr ctxt)
{
	rbBufPtr buf;
	void *content;
	bool ok = false;

	buf = rbCreateBufParams(16, RB_GROW_EXACT, 0);
	assert(buf);
	if (rbAddDataToBuf(buf, src_bytes, 24) != RB_RESULT_OK)
		FAIL("adding data to a growable buffer must succeed");
	if (!(content = rbDetachBufContent(buf)))
		FAIL("detached content must not be NULL");
	xmlFree(content);
	if (rbAddDataToBuf(buf, src_bytes, 9) != RB_RESULT_OK)
		FAIL("adding data after detaching content must succeed");
	if (rbGetBufContentSize(buf) != 9)
		FAIL("content size must be 9");
	if (rbGetBufTotalSize(buf) != 9)
		FAIL("total size must be 9");
	ok = true;
cleanup:
	rbFreeBuf(buf);
	return ok;
}

static bool xtsTestReszBuf_DetachContent_Increment(xtsContextPtr ctxt)
{
	rbBufPtr buf;
	void *content;
	bool ok = false;

	buf = rbCreateBufParams(16, RB_GROW_INCREMENT, 16);
	assert(buf);
	if (rbAddDataToBuf(buf, src_bytes, 24) != RB_RESULT_OK)
		FAIL("adding data to a growable buffer must succeed");
	if (!(content = rbDetachBufContent(buf)))
		FAIL("detached content must not be NULL");
	xmlFree(content);
	if (rbAddDataToBuf(buf, src_bytes, 9) != RB_RESULT_OK)
		FAIL("adding data after detaching content must succeed");
	if (rbGetBufContentSize(buf) != 9)
		FAIL("content size must be 9");
	if (rbGetBufTotalSize(buf) != 16)
		FAIL("total size must be 16");
	ok = true;
cleanup:
	rbFreeBuf(buf);
	return ok;
}

static bool xtsTestReszBuf_DetachContent_Double(xtsContextPtr ctxt)
{
	rbBufPtr buf;
	void *content;
	bool ok = false;

	buf = rbCreateBufParams(16, RB_GROW_DOUBLE, 0);
	assert(buf);
	if (rbAddDataToBuf(buf, src_bytes, 24) != RB_RESULT_OK)
		FAIL("adding data to a growable buffer must succeed");
	if (!(content = rbDetachBufContent(buf)))
		FAIL("detached content must not be NULL");
	xmlFree(content);
	if (rbAddDataToBuf(buf, src_bytes, 9) != RB_RESULT_OK)
		FAIL("adding data after detaching content must succeed");
	if (rbGetBufContentSize(buf) != 9)
		FAIL("content size must be 9");
	if (rbGetBufTotalSize(buf) != 16)
		FAIL("total size must be 16");
	ok = true;
cleanup:
	rbFreeBuf(buf);
	return ok;
}

static bool xtsTestReszBuf_DetachContent_Flush(xtsContextPtr ctxt)
{
	rbBufPtr buf;
	void *content;
	bool ok = false;

	buf = rbCreateFlushableBuf(16, flush_to_dev_null_and_succeed);
	assert(buf);
	if (rbAddDataToBuf(buf, src_bytes, 24) != RB_RESULT_OK)
		FAIL("adding data to a growable buffer must succeed");
	if (!(content = rbDetachBufContent(buf))) /* should anyone ever need a tail? */
		FAIL("detached content must not be NULL");
	xmlFree(content);
	if (rbAddDataToBuf(buf, src_bytes, 30) != RB_RESULT_OK)
		FAIL("adding data after detaching content must succeed");
	if (rbGetBufContentSize(buf) != 14)
		FAIL("content size must be 14");
	if (rbGetBufTotalSize(buf) != 16)
		FAIL("total size must be 16");
	ok = true;
cleanup:
	rbFreeBuf(buf);
	return ok;
}

static bool xtsTestReszBuf_Position(xtsContextPtr ctxt)
{
	rbBufPtr buf;
	xmlChar *test_string = BAD_CAST "This is a test";
	xmlChar *content;
	bool ok = false;

	buf = rbCreateBufParams(32, RB_GROW_FIXED, 0);
	assert(buf);
	if (rbAddDataToBuf(buf, test_string, xmlStrlen(test_string) + 1) != RB_RESULT_OK)
		FAIL("adding data must succeed");
	if (!(content = (xmlChar*) rbGetBufContent(buf)))
		FAIL("content must not be NULL");
	if ((xmlChar*) rbGetBufPosition(buf) != content + xmlStrlen(test_string) + 1)
		FAIL("wrong buffer position");
	if (xmlStrcmp(content, test_string))
		FAIL("wrong buffer content");

	if (rbRewindBuf(buf) != RB_RESULT_OK)
		FAIL("rewinding failed");
	if (xmlStrcmp((xmlChar*) rbGetBufPosition(buf), test_string))
		FAIL("wrong content after rewinding");

	if (rbAdvanceBufPosition(buf, 5) != RB_RESULT_OK)
		FAIL("advancing position failed");
	if (!(content = (xmlChar*) rbGetBufPosition(buf)))
		FAIL("content must not be NULL");
	if (xmlStrcmp(content, test_string + 5))
		FAIL("wrong content after advancing");
	ok = true;
cleanup:
	rbFreeBuf(buf);
	return ok;
}

static xtsTest resz_buf_tests[] =
{
	{
		SFINIT(.id, BAD_CAST "create_default_params"),
		SFINIT(.displayName, BAD_CAST "Creation with default parameters"),
		SFINIT(.testFunction, xtsTestReszBuf_CreateDefaultParams),
		SFINIT(.flags, XTS_FLAG_CHECK_MEMORY)
	}, {
		SFINIT(.id, BAD_CAST "create_with_size"),
		SFINIT(.displayName, BAD_CAST "Creation with explicit size"),
		SFINIT(.testFunction, xtsTestReszBuf_CreateWithSize),
		SFINIT(.flags, XTS_FLAG_CHECK_MEMORY)
	}, {
		SFINIT(.id, BAD_CAST "create_explicit"),
		SFINIT(.displayName, BAD_CAST "Creation with explicit parameters"),
		SFINIT(.testFunction, xtsTestReszBuf_CreateExplicit),
		SFINIT(.flags, XTS_FLAG_CHECK_MEMORY)
	}, {
		SFINIT(.id, BAD_CAST "create_flushable"),
		SFINIT(.displayName, BAD_CAST "Flushable buffer creation"),
		SFINIT(.testFunction, xtsTestReszBuf_CreateFlushable),
		SFINIT(.flags, XTS_FLAG_CHECK_MEMORY)
	}, {
		SFINIT(.id, BAD_CAST "grow_exact"),
		SFINIT(.displayName, BAD_CAST "Adding data with exact grow strategy"),
		SFINIT(.testFunction, xtsTestReszBuf_GrowExact),
		SFINIT(.flags, XTS_FLAG_CHECK_MEMORY)
	}, {
		SFINIT(.id, BAD_CAST "grow_increment"),
		SFINIT(.displayName, BAD_CAST "Adding data with incremental grow strategy"),
		SFINIT(.testFunction, xtsTestReszBuf_GrowIncrement),
		SFINIT(.flags, XTS_FLAG_CHECK_MEMORY)
	}, {
		SFINIT(.id, BAD_CAST "grow_double"),
		SFINIT(.displayName, BAD_CAST "Adding data with doubling grow strategy"),
		SFINIT(.testFunction, xtsTestReszBuf_GrowDouble),
		SFINIT(.flags, XTS_FLAG_CHECK_MEMORY)
	}, {
		SFINIT(.id, BAD_CAST "grow_fixed"),
		SFINIT(.displayName, BAD_CAST "Adding data with fixed grow strategy"),
		SFINIT(.testFunction, xtsTestReszBuf_GrowFixed),
		SFINIT(.flags, XTS_FLAG_CHECK_MEMORY)
	}, {
		SFINIT(.id, BAD_CAST "grow_flush"),
		SFINIT(.displayName, BAD_CAST "Adding data with flush grow strategy"),
		SFINIT(.testFunction, xtsTestReszBuf_GrowFlush),
		SFINIT(.flags, XTS_FLAG_CHECK_MEMORY)
	}, {
		SFINIT(.id, BAD_CAST "flush"),
		SFINIT(.displayName, BAD_CAST "Various flush functions"),
		SFINIT(.testFunction, xtsTestReszBuf_Flush),
		SFINIT(.flags, XTS_FLAG_CHECK_MEMORY)
	}, {
		SFINIT(.id, BAD_CAST "invalid_params"),
		SFINIT(.displayName, BAD_CAST "Invalid function parameters"),
		SFINIT(.testFunction, xtsTestReszBuf_InvalidParams),
		SFINIT(.flags, XTS_FLAG_CHECK_MEMORY)
	}, {
		SFINIT(.id, BAD_CAST "invalid_buf"),
		SFINIT(.displayName, BAD_CAST "Invalid (NULL) buffer"),
		SFINIT(.testFunction, xtsTestReszBuf_InvalidBuf),
		SFINIT(.flags, XTS_FLAG_CHECK_MEMORY)
	}, {
		SFINIT(.id, BAD_CAST "grow_dynamic"),
		SFINIT(.displayName, BAD_CAST "Dynamically changing grow strategy"),
		SFINIT(.testFunction, xtsTestReszBuf_GrowDynamic),
		SFINIT(.flags, XTS_FLAG_CHECK_MEMORY)
	}, {
		SFINIT(.id, BAD_CAST "detach_fixed"),
		SFINIT(.displayName, BAD_CAST "Detaching content and reusing buffer (fixed GS)"),
		SFINIT(.testFunction, xtsTestReszBuf_DetachContent_Fixed),
		SFINIT(.flags, XTS_FLAG_CHECK_MEMORY)
	}, {
		SFINIT(.id, BAD_CAST "detach_exact"),
		SFINIT(.displayName, BAD_CAST "Detaching content and reusing buffer (exact GS)"),
		SFINIT(.testFunction, xtsTestReszBuf_DetachContent_Exact),
		SFINIT(.flags, XTS_FLAG_CHECK_MEMORY)
	}, {
		SFINIT(.id, BAD_CAST "detach_increment"),
		SFINIT(.displayName, BAD_CAST "Detaching content and reusing buffer (incremental GS)"),
		SFINIT(.testFunction, xtsTestReszBuf_DetachContent_Increment),
		SFINIT(.flags, XTS_FLAG_CHECK_MEMORY)
	}, {
		SFINIT(.id, BAD_CAST "detach_double"),
		SFINIT(.displayName, BAD_CAST "Detaching content and reusing buffer (doubling GS)"),
		SFINIT(.testFunction, xtsTestReszBuf_DetachContent_Double),
		SFINIT(.flags, XTS_FLAG_CHECK_MEMORY)
	}, {
		SFINIT(.id, BAD_CAST "detach_flush"),
		SFINIT(.displayName, BAD_CAST "Detaching content and reusing buffer (flush GS)"),
		SFINIT(.testFunction, xtsTestReszBuf_DetachContent_Flush),
		SFINIT(.flags, XTS_FLAG_CHECK_MEMORY)
	}, {
		SFINIT(.id, BAD_CAST "position"),
		SFINIT(.displayName, BAD_CAST "Rewinding and advancing position"),
		SFINIT(.testFunction, xtsTestReszBuf_Position),
		SFINIT(.flags, XTS_FLAG_CHECK_MEMORY)
	}
};

xtsFixture xtsTestReszBufFixture =
{
	SFINIT(.id, BAD_CAST "resz_buf"),
	SFINIT(.displayName, BAD_CAST "resizable buffer test group"),
	SFINIT(.setup, NULL),
	SFINIT(.teardown, NULL),
	SFINIT(.test_count, sizeof(resz_buf_tests) / sizeof(resz_buf_tests[0])),
	SFINIT(.tests, &resz_buf_tests[0])
};
