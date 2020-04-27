#include <libxpl/abstraction/xef.h>
#include <libxpl/abstraction/xefinternal.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplutils.h>
#include <tidy/tidy.h>
#include <tidy/tidybuffio.h>


void* TIDY_CALL xml_tidyMalloc(size_t len)
{
	return xmlMalloc(len);
}
void* TIDY_CALL xml_tidyRealloc(void* buf, size_t len)
{
	return xmlRealloc(buf, len);
}
void TIDY_CALL xml_tidyFree(void* buf)
{
	xmlFree(buf);
}

XEF_STARTUP_PROTO(HtmlCleaner)
{
#ifdef _LEAK_DETECTION
	tidySetMallocCall(xml_tidyMalloc);
	tidySetReallocCall(xml_tidyRealloc);
	tidySetFreeCall(xml_tidyFree);
#endif
	return true;
}

XEF_SHUTDOWN_PROTO(HtmlCleaner)
{
}

/* unused */
XEF_GET_ERROR_TEXT_PROTO(HtmlCleaner)
{
	DISPLAY_INTERNAL_ERROR_MESSAGE();
	return NULL;
}

/* unused */
XEF_FREE_ERROR_MESSAGE_PROTO(HtmlCleaner)
{
	DISPLAY_INTERNAL_ERROR_MESSAGE();
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
		params->error = xefCreateCommonErrorMessage("cannot create HtmlTidy document");
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
		params->error = xefCreateCommonErrorMessage("HtmlTidy refused to parse input document");
		return false;
	}
	if (tidyCleanAndRepair(tdoc) < 0)
	{
		tidyBufFree(&output);
		tidyBufFree(&errbuf);
		params->error = xefCreateCommonErrorMessage("HtmlTidy refused to clean input document");
		return false;
	}
	if (cfgPrintTidyInfo)
		tidyRunDiagnostics(tdoc);
	if (tidySaveBuffer(tdoc, &output) < 0)
	{
		tidyBufFree(&output);
		tidyBufFree(&errbuf);
		params->error = xefCreateCommonErrorMessage("cannot save document cleaned by HtmlTidy to buffer");
		return false;
	}
	tidyBufPutByte(&output, 0); /* ensure NULL-termination */
#if 0
	tidySaveFile(tdoc, "d:\\ss\\debug.xml");
#endif
	tidy_content = BAD_CAST output.bp;
	tidy_size = (size_t) output.size - 1;
	tidyBufDetach(&output);
	params->clean_document = tidy_content;
	params->clean_document_size = tidy_size;
	params->error = NULL;
	tidyBufFree(&output);
	tidyBufFree(&errbuf);
	if (tdoc) tidyRelease(tdoc);
	return true;
}
