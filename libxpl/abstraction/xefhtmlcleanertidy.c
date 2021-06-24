#include <libxpl/abstraction/xef.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <tidy/tidy.h>
#include <tidy/tidybuffio.h>

#ifdef _LEAK_DETECTION
static void* TIDY_CALL xml_tidyMalloc(size_t len)
{
	return XPL_MALLOC(len);
}

static void* TIDY_CALL xml_tidyRealloc(void* buf, size_t len)
{
	return XPL_REALLOC(buf, len);
}

static void TIDY_CALL xml_tidyFree(void* buf)
{
	XPL_FREE(buf);
}
#endif

bool xefStartupHtmlCleaner(xefStartupParamsPtr params)
{
	UNUSED_PARAM(params);
#ifdef _LEAK_DETECTION
	tidySetMallocCall(xml_tidyMalloc);
	tidySetReallocCall(xml_tidyRealloc);
	tidySetFreeCall(xml_tidyFree);
#endif
	return true;
}

void xefShutdownHtmlCleaner()
{
}

bool xefCleanHtml(xefCleanHtmlParamsPtr params)
{
	TidyBuffer output = {0};
	TidyBuffer errbuf = {0};
	TidyDoc tdoc;
	xmlChar *tidy_content = NULL;
	size_t tidy_size;

	params->clean_document = NULL;
	params->clean_document_size = 0;
	
	if (!(tdoc = tidyCreate()))
	{
		params->error = BAD_CAST XPL_STRDUP("cannot create HtmlTidy document");
		return false;
	}
	tidyOptSetBool(tdoc, TidyXhtmlOut, yes); /* don't set to "XML" */
	tidyOptSetBool(tdoc, TidyNumEntities, yes);
	tidyOptSetBool(tdoc, TidyForceOutput, yes);
	tidySetCharEncoding(tdoc, "utf8");
	tidySetErrorBuffer(tdoc, &errbuf);
	if (tidyParseString(tdoc, (char*) params->document) < 0)
	{
		tidyBufFree(&output);
		tidyBufFree(&errbuf);
		params->error = BAD_CAST XPL_STRDUP("HtmlTidy refused to parse input document"); // TODO diagnostics?
		return false;
	}
	if (tidyCleanAndRepair(tdoc) < 0)
	{
		tidyBufFree(&output);
		tidyBufFree(&errbuf);
		params->error = BAD_CAST XPL_STRDUP("HtmlTidy refused to clean input document"); // TODO diagnostics?
		return false;
	}
	if (cfgPrintTidyInfo)
		tidyRunDiagnostics(tdoc);
	if (tidySaveBuffer(tdoc, &output) < 0)
	{
		tidyBufFree(&output);
		tidyBufFree(&errbuf);
		params->error = BAD_CAST XPL_STRDUP("cannot save document cleaned by HtmlTidy to buffer");
		return false;
	}
	tidyBufPutByte(&output, 0); /* ensure NULL-termination */

	tidy_content = BAD_CAST output.bp;
	tidy_size = (size_t) output.size - 1;
	tidyBufDetach(&output);
	params->clean_document = tidy_content;
	params->clean_document_size = tidy_size;
	params->error = NULL;
	tidyBufFree(&output);
	tidyBufFree(&errbuf);
	tidyRelease(tdoc);
	return true;
}
