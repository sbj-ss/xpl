#include <sys/stat.h>
#include <unistd.h>
#include <libxpl/abstraction/xef.h>
#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplutils.h>
#include "commands/Include.h"

#define SELECT_ATTR (BAD_CAST "select")
#define SOURCE_ATTR (BAD_CAST "source")
#define FILE_ATTR (BAD_CAST "file")
#define URI_ATTR (BAD_CAST "uri")
#define REPEAT_ATTR (BAD_CAST "repeat")
#define RESPONSE_TAG_NAME_ATTR (BAD_CAST "responsetagname")
#define ENCODING_ATTR (BAD_CAST "encoding")
#define ABS_PATH_ATTR (BAD_CAST "abspath")
#define INPUT_FORMAT_ATTR (BAD_CAST "inputformat")
#define OUTPUT_FORMAT_ATTR (BAD_CAST "outputformat")
#define POSTDATA_ATTR (BAD_CAST "postdata")
#define REMOVE_SOURCE_ATTR (BAD_CAST "removesource")

typedef enum 
{
	INPUT_FORMAT_XML,
	INPUT_FORMAT_HTML,
	INPUT_FORMAT_RAW
} InputFormat;

typedef enum 
{
	OUTPUT_FORMAT_XML,
	OUTPUT_FORMAT_TEXT,
	OUTPUT_FORMAT_BASE64,
	OUTPUT_FORMAT_HEX
} OutputFormat;

typedef enum 
{
	INPUT_SOURCE_LOCAL,
	INPUT_SOURCE_FILE,
	INPUT_SOURCE_XTP
} InputSource;

typedef struct _IncludeContext
{
	xmlNodePtr command_element;
	xplDocumentPtr doc;
	xmlChar *encoding_str;
	int encoding;
	bool needs_recoding;
	InputFormat input_format;
	OutputFormat output_format;
	/* input source info */
	xmlChar *uri;
	InputSource input_source;
	/* temps */
	xmlNodePtr src_node;
	xmlChar *content;
	size_t content_size;
	xmlNodePtr error;
	xmlNodePtr ret;
} IncludeContext, *IncludeContextPtr;

static void initIncludeContext(IncludeContextPtr ctxt, const xplCommandInfoPtr commandInfo)
{
	memset(ctxt, 0, sizeof(IncludeContext));
	ctxt->command_element = commandInfo->element;
	ctxt->doc = commandInfo->document;
}

static void clearIncludeContext(IncludeContextPtr ctxt)
{
	if (ctxt->encoding_str) 
		xmlFree(ctxt->encoding_str);
	if (ctxt->uri) 
		xmlFree(ctxt->uri);
	if ((ctxt->input_source != INPUT_SOURCE_LOCAL) && ctxt->src_node)
		xmlFreeDoc((xmlDocPtr) ctxt->src_node);
	if (ctxt->content)
		xmlFree(ctxt->content);
}

static void getSourceStep(IncludeContextPtr ctxt)
{
	xmlChar *uri_attr = xmlGetNoNsProp(ctxt->command_element, URI_ATTR);
	xmlChar *file_attr = xmlGetNoNsProp(ctxt->command_element, FILE_ATTR);
	bool abspath;

	if (uri_attr && file_attr)
	{
		ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "file and uri can't be specified simultaneously");
		xmlFree(uri_attr);
		xmlFree(file_attr);
	} else if (file_attr) {
		ctxt->error = xplDecodeCmdBoolParam(ctxt->command_element, ABS_PATH_ATTR, &abspath, false);
		if (ctxt->error)
			return;
		if (abspath)
			ctxt->uri = file_attr;
		else {
			ctxt->uri = xplFullFilename(file_attr, ctxt->doc->app_path);
			xmlFree(file_attr);
		}
		ctxt->input_source = INPUT_SOURCE_FILE;
	} else if (uri_attr) {
		if (xmlStrstr(uri_attr, BAD_CAST "file:///") == uri_attr)
		{
			ctxt->uri = xmlStrdup(file_attr + 8);
			ctxt->input_source = INPUT_SOURCE_FILE;
			xmlFree(uri_attr);
		} else if ((xmlStrstr(uri_attr, BAD_CAST "ftp://") == uri_attr) || (xmlStrstr(uri_attr, BAD_CAST "http://") == uri_attr)) {
			ctxt->uri = uri_attr;
			ctxt->input_source = INPUT_SOURCE_XTP;
		} else {
			ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "invalid uri: \"%s\"", uri_attr);
			xmlFree(uri_attr);
		}
	} else
		ctxt->input_source = INPUT_SOURCE_LOCAL;	
}

static void getFormatsStep(IncludeContextPtr ctxt)
{
	xmlChar *input_format_attr = xmlGetNoNsProp(ctxt->command_element, INPUT_FORMAT_ATTR);
	xmlChar *output_format_attr = xmlGetNoNsProp(ctxt->command_element, OUTPUT_FORMAT_ATTR);

	if (!input_format_attr || !xmlStrcasecmp(input_format_attr, BAD_CAST "xml"))
	{
		ctxt->input_format = INPUT_FORMAT_XML;
		ctxt->output_format = OUTPUT_FORMAT_XML;
	} else if (!xmlStrcasecmp(input_format_attr, BAD_CAST "html")) {
		ctxt->input_format = INPUT_FORMAT_HTML;
		ctxt->output_format = OUTPUT_FORMAT_XML;
	} else if (!xmlStrcasecmp(input_format_attr, BAD_CAST "raw")) {
		ctxt->input_format = INPUT_FORMAT_RAW;
		ctxt->output_format = OUTPUT_FORMAT_TEXT;
	} else { /* error */
		ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "invalid input format value: \"%s\"", input_format_attr);
		goto done;
	}

	if (!output_format_attr)
		(void) 0;
	else if (!xmlStrcasecmp(output_format_attr, BAD_CAST "text"))
		ctxt->output_format = OUTPUT_FORMAT_TEXT;
	else if (!xmlStrcasecmp(output_format_attr, BAD_CAST "xml"))
		ctxt->output_format = OUTPUT_FORMAT_XML;
	else if (!xmlStrcasecmp(output_format_attr, BAD_CAST "hex"))
		ctxt->output_format = OUTPUT_FORMAT_HEX;
	else if (!xmlStrcasecmp(output_format_attr, BAD_CAST "base64") || !xmlStrcasecmp(output_format_attr, BAD_CAST "binarybase64")) 
		ctxt->output_format = OUTPUT_FORMAT_BASE64;
	else { 
		ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "invalid output format value: \"%s\"", output_format_attr);
		goto done;
	}

	/* sanity checks */
	if ((ctxt->input_format == INPUT_FORMAT_RAW) && (ctxt->output_format == OUTPUT_FORMAT_XML))
	{
		ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "cannot create XML output from raw input");
		goto done;
	}
	if ((ctxt->input_source == INPUT_SOURCE_LOCAL) && (ctxt->input_format != INPUT_FORMAT_XML))
	{
		ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "input format for local inclusion must be xml");
		goto done;
	}
done:
	if (input_format_attr) xmlFree(input_format_attr);
	if (output_format_attr) xmlFree(output_format_attr);
}

static void fetchFileContent(IncludeContextPtr ctxt)
{
	int fd;
	struct stat stat;
	size_t file_size, num_read;
	char* file_content;
	
	fd = xprSOpen(ctxt->uri, O_BINARY | O_RDONLY, 0, 0);
	if (fd == -1)
	{
		ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "cannot open file \"%s\"", ctxt->uri);
		return;
	}
	fstat(fd, &stat); /* TODO where's stat64? */
	file_size = stat.st_size;
	ctxt->content = file_content = (char*) xmlMalloc(file_size + 2);
	if (!file_content)
	{
		ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "insufficient memory");
		return;
	}
	ctxt->content_size = file_size;
	while (file_size > 0)
	{
		if ((num_read = read(fd, file_content, (unsigned int)(file_size > 0x10000? 0x10000: file_size))) == -1)
		{
			ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "cannot read from file \"%s\"", ctxt->uri);
			ctxt->content = NULL;
			ctxt->content_size = 0;
			xmlFree(file_content);
			return;
		}
		file_content += num_read;
		file_size -= num_read;
	}
	*file_content = *(file_content + 1) = 0;
	close(fd);
}

static void fetchXTPContent(IncludeContextPtr ctxt)
{
#ifdef _XEF_HAS_TRANSPORT
	xefFetchDocumentParams params;
	xmlChar *error_text;

	memset(&params, 0, sizeof(params));
	params.uri = ctxt->uri;
	params.extra_query = xmlGetNoNsProp(ctxt->command_element, POSTDATA_ATTR);
	if (xefFetchDocument(&params))
	{
		ctxt->content = params.document;
		params.document = NULL;
		ctxt->content_size = params.document_size;
	} else {
		if (params.error)
		{
			error_text = xefGetErrorText(params.error);
			ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "fetch error \"%s\"", error_text);
			xmlFree(error_text);
			xefFreeErrorMessage(params.error);
		} else
			ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "unknown fetch error");
	}
	if (params.extra_query)
		xmlFree(params.extra_query);
	xefFetchParamsClear(&params);
#else
	ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "remote transport support not compiled in");
#endif
}

static void fetchContentStep(IncludeContextPtr ctxt)
{
	switch (ctxt->input_source)
	{
	case INPUT_SOURCE_LOCAL:
		ctxt->src_node = ctxt->command_element;
		break;
	case INPUT_SOURCE_FILE:
		fetchFileContent(ctxt);
		break;
	case INPUT_SOURCE_XTP:
		fetchXTPContent(ctxt);
		break;
	default:
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	}
}

static size_t detectionSampleLen(char* content, size_t size)
{
	int count = 0;
	size_t ret;
	char *end = content + size, *p = content;

	while ((count < 2048) && (p < end))
	{
		if ((*p++) & 0x80)
			count++;
	}
	ret = p - content;
	return ret;
}

static int fastDetectEncoding(char* content, size_t size)
{
	if (size >= 2)
	{
		if ((*content == (char) 0xFE) && (*(content+1) == (char) 0xFF))
			return DETECTED_ENC_UTF16LE;
		if (!*content)
			return DETECTED_ENC_UTF16LE;
		if ((*content == (char) 0xFF) && (*(content+1) == (char) 0xFE))
			return DETECTED_ENC_UTF16BE;
		if (!*(content+1))
			return DETECTED_ENC_UTF16BE;
	}
	if (size >= 3)
	{
		if ((*content == (char) 0xEF) && (*(content+1) == (char) 0xBB) && (*(content+2) == (char) 0xBF))
			return DETECTED_ENC_UTF8;
	}
	return detectEncoding(content, detectionSampleLen(content, size));
}

static void updateCtxtEncoding(IncludeContextPtr ctxt)
{
	xmlChar *encoding_attr = xmlGetNoNsProp(ctxt->command_element, ENCODING_ATTR);
	if (!encoding_attr || !xmlStrcasecmp(encoding_attr, BAD_CAST "utf-8"))
	{
		ctxt->needs_recoding = false;
		ctxt->encoding_str = encoding_attr;
		return;
	} else if (xmlStrcasecmp(encoding_attr, BAD_CAST "auto")) { /* non-utf8 encoding specified */
		ctxt->needs_recoding = true;
		ctxt->encoding_str = encoding_attr;
		return;
	}
	xmlFree(encoding_attr); // was "auto"
	/* run detection */
	ctxt->encoding = fastDetectEncoding(ctxt->content, ctxt->content_size);
	switch (ctxt->encoding)
	{
	case DETECTED_ENC_1251:
		ctxt->encoding_str = xmlStrdup(BAD_CAST "CP1251"); 
		break;
	case DETECTED_ENC_UTF8: 
	case DETECTED_ENC_UNKNOWN:
		ctxt->encoding_str = xmlStrdup(BAD_CAST "utf-8"); 
		ctxt->encoding = DETECTED_ENC_UTF8;
		break;
	case DETECTED_ENC_UTF16LE: 
		ctxt->encoding_str = xmlStrdup(BAD_CAST "utf-16le"); 
		break;
	case DETECTED_ENC_UTF16BE: 
		ctxt->encoding_str = xmlStrdup(BAD_CAST "utf-16be"); 
		break;
	case DETECTED_ENC_866: 
		ctxt->encoding_str = xmlStrdup(BAD_CAST "cp866"); 
		break;
	case DETECTED_ENC_KOI8: 
		ctxt->encoding_str = xmlStrdup(BAD_CAST "KOI8-R"); 
		break;
	default: 
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	}
	ctxt->needs_recoding = (ctxt->encoding != DETECTED_ENC_UTF8);
}

static void recodeStep(IncludeContextPtr ctxt)
{
	size_t recoded_size;
	xmlChar *recoded_content = NULL;

	if (ctxt->input_source == INPUT_SOURCE_LOCAL)
		return;
	updateCtxtEncoding(ctxt);
	if (!ctxt->needs_recoding)
		return;
	if (iconv_string("utf-8", (const char*) ctxt->encoding_str, ctxt->content, ctxt->content + ctxt->content_size, (char**) &recoded_content, &recoded_size) == -1)
	{
		ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "error converting data from encoding \"%s\"", ctxt->encoding_str);
		return;
	}
	xmlFree(ctxt->content);
	ctxt->content = recoded_content;
	ctxt->content_size = recoded_size;
}

static bool checkForNonprintable(xmlChar* src, size_t size)
{
	size_t i;
	static bool map_ok = false;
	static xmlChar map[256];

	if (!map_ok)
	{
		memset(map + 32, 1, 224);
		map[0x09] = 1;
		map[0x0A] = 1;
		map[0x0D] = 1;
		map_ok = true;
	}
	for (i = 0; i < size; i++)
	{
		if (!map[*src++])
			return false;
	}
	return true;
}

static void cleanStep(IncludeContextPtr ctxt)
{
#ifdef _XEF_HAS_HTML_CLEANER
	xefCleanHtmlParams params;
	xmlChar *error_txt;
#endif
	if ((ctxt->input_format == INPUT_FORMAT_RAW) && (ctxt->output_format == OUTPUT_FORMAT_TEXT))
	{
		if (!checkForNonprintable(ctxt->content, ctxt->content_size))
		{
			ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "input document contains characters that cannot be output as text (try hex or base64 output format)");
			return;
		}
		if (!isValidUtf8Sample(ctxt->content, ctxt->content_size, true))
		{
			ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "input document contains characters that cannot be displayed as utf-8 (try recoding or hex/base64 output format)");
			return;
		}
		/* don't call xmlEncodeSpecialChars right here! */
		return;
	}

	if (ctxt->input_format != INPUT_FORMAT_HTML)
		return;
#ifdef _XEF_HAS_HTML_CLEANER
	params.document = ctxt->content;
	if (xefCleanHtml(&params))
	{
		xmlFree(ctxt->content);
		ctxt->content = params.clean_document;
		ctxt->content_size = params.clean_document_size;
	} else if (params.error) {
		error_txt = xefGetErrorText(params.error);
		ctxt->error = xplCreateErrorNode(ctxt->command_element, "Error cleaning HTML document: \"%s\"", error_txt);
		xmlFree(error_txt);
		xefFreeErrorMessage(params.error);
	} else
		ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "cannot clean HTML document: unknown error");
#else
	ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "HTML document cleaning support not compiled in");
#endif
}

static void buildDocumentStep(IncludeContextPtr ctxt)
{
	xmlChar *error_txt;

	switch(ctxt->input_format)
	{
	case INPUT_FORMAT_XML:
	case INPUT_FORMAT_HTML:
		if ((ctxt->input_source != INPUT_SOURCE_LOCAL) && 
			(xmlHasProp(ctxt->command_element, SELECT_ATTR) || (ctxt->output_format == OUTPUT_FORMAT_XML))) 
		{ /* we need a document somewhere on the way, let's build it */
			if (ctxt->content_size > INT32_MAX)
			{
				ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "input document is bigger than 2 Gb");
				return;
			}
			ctxt->src_node = (xmlNodePtr) xmlReadMemory(ctxt->content, (int) ctxt->content_size, NULL, NULL, XML_PARSE_NODICT);
			if (!ctxt->src_node)
			{
				error_txt = getLastLibxmlError();
				ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "error parsing input document: \"%s\"", error_txt);
				xmlFree(error_txt);
				return;
			}
		}
		break;
	case INPUT_FORMAT_RAW:
		break;
	default:
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	}
}

static void selectNodesStep(IncludeContextPtr ctxt)
{
	xmlChar *select_attr;
	xmlChar *response_tag_name_attr;
	xmlXPathObjectPtr sel;
	xmlNodePtr cur, head = NULL, tail, sibling, prnt;
	bool remove_source;
	size_t i;

	/* check select attribute */
	select_attr = xmlGetNoNsProp(ctxt->command_element, SELECT_ATTR);
	if (!select_attr)
		select_attr = xmlGetNoNsProp(ctxt->command_element, SOURCE_ATTR);
	if (select_attr && (ctxt->input_format == INPUT_FORMAT_RAW)) 
	{
		ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "cannot select nodes from non-structured source");
		xmlFree(select_attr);
		return;
	}
	if (ctxt->input_format == INPUT_FORMAT_RAW)
		return;
	if (!select_attr) /* return the input document/command content */
	{
		if (ctxt->input_source == INPUT_SOURCE_LOCAL)
			ctxt->ret = detachContent(ctxt->command_element);
		else {
			cur = ctxt->ret = detachContent(ctxt->src_node);
			while (cur) /* doc may start with a comment */
			{
				if (cur->type == XML_ELEMENT_NODE)
					break;
				ctxt->ret = cur->next;
				xmlUnlinkNode(cur);
				xmlFreeNode(cur);
				cur = ctxt->ret;
			}
			xmlSetTreeDoc(ctxt->ret, ctxt->command_element->doc);
		}
		response_tag_name_attr = xmlGetNoNsProp(ctxt->command_element, RESPONSE_TAG_NAME_ATTR);
		if (response_tag_name_attr)
		{
			cur = ctxt->ret;
			while (cur)
			{
				if (cur->type == XML_ELEMENT_NODE)
					xmlNodeSetName(cur, response_tag_name_attr);
				cur = cur->next;
			}
			xmlFree(response_tag_name_attr);
		}
		return;
	}
	/* select nodes */
	if ((ctxt->error = xplDecodeCmdBoolParam(ctxt->command_element, REMOVE_SOURCE_ATTR, &remove_source, false)))
	{
		xmlFree(select_attr);
		return;
	}
	if (ctxt->input_source == INPUT_SOURCE_LOCAL)
		cur = ctxt->src_node;
	else {
		cur = ctxt->src_node->children;
		while (cur && (cur->type != XML_ELEMENT_NODE)) /* skip comments */
			cur = cur->next;
	}
	sel = xplSelectNodes(ctxt->doc, cur, select_attr);
	if (sel)
	{
		if (sel->type == XPATH_NODESET)
		{
			if (sel->nodesetval)
			{
				/* There could be some speedups for OUTPUT_FORMAT_TEXT, but they would mess the code up completely. */
				response_tag_name_attr = xmlGetNoNsProp(ctxt->command_element, RESPONSE_TAG_NAME_ATTR);
				for (i = 0; i < (size_t) sel->nodesetval->nodeNr; i++)
				{
					sibling = sel->nodesetval->nodeTab[i];
					prnt = sibling->parent;
					sibling->parent = NULL;
					cur = cloneAttrAsText(sibling, ctxt->command_element);
					if (response_tag_name_attr && (cur->type == XML_ELEMENT_NODE))
						xmlNodeSetName(cur, response_tag_name_attr);
					sibling->parent = prnt;
					if (!head)
						head = tail = cur;
					else
						tail = xmlAddNextSibling(tail, cur);
					if (remove_source) /* "move" instead of copy */
					{
						xmlUnlinkNode(sibling);
						if (ctxt->input_source == INPUT_SOURCE_LOCAL)
							xplDocDeferNodeDeletion(ctxt->doc, sibling);
						else
							xmlFreeNode(sibling);
						sel->nodesetval->nodeTab[i] = 0;
					}
				}
				if (response_tag_name_attr)
					xmlFree(response_tag_name_attr);
			}
		} else if (sel->type != XPATH_UNDEFINED) 
			ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "select XPath expression \"%s\" evaluated to non-nodeset value", select_attr);
		else 
			ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "select XPath expression \"%s\" evaluated to undef", select_attr);
	} else
		ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "invalid select XPath expression \"%s\"", select_attr);
	ctxt->ret = head;
	if (sel)
		xmlXPathFreeObject(sel);
	xmlFree(select_attr);
}

static void computeRetStep(IncludeContextPtr ctxt)
{
	size_t encoded_size;
	xmlChar *encoded_content;

	if ((ctxt->input_format != INPUT_FORMAT_RAW) && (ctxt->output_format != OUTPUT_FORMAT_XML) && ctxt->ret)
	{
		/* we have a node list and need an optionally encoded string. */
		if (ctxt->content)
			xmlFree(ctxt->content);
		ctxt->content = serializeNodeList(ctxt->ret);
		ctxt->content_size = xmlStrlen(ctxt->content);
		xmlFreeNodeList(ctxt->ret);
	}
	switch (ctxt->output_format)
	{
	case OUTPUT_FORMAT_XML:
		/* nothing to do here */
		return;
	case OUTPUT_FORMAT_BASE64:
		if ((ctxt->content_size*4.0/3 + 4) > (size_t) SIZE_MAX)
		{
			ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "content cannot be transformed to base64, resulting value is bigger than SIZE_MAX");
			return;
		}
		encoded_size = (size_t) (ctxt->content_size*4.0/3 + 4); /* rounding down, padding and terminating zero */
		encoded_content = (xmlChar*) xmlMalloc(encoded_size);
		if (!encoded_content)
		{
			ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "insufficient memory");
			return;
		}
		base64encode(ctxt->content, ctxt->content_size, (char*) encoded_content, encoded_size);
		xmlFree(ctxt->content);
		ctxt->content = encoded_content;
		ctxt->content_size = encoded_size;
		break;
	case OUTPUT_FORMAT_HEX:
		if (!(encoded_content = bufferToHex(ctxt->content, ctxt->content_size, false)))
		{
			ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "insufficient memory");
			return;
		}
		xmlFree(ctxt->content);
		ctxt->content = encoded_content;
		ctxt->content_size *= 2;
		break;
	case OUTPUT_FORMAT_TEXT:
		break;
	default:
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	}
	/* prepare output */
	ctxt->ret = xmlNewDocText(ctxt->command_element->doc, NULL);
	ctxt->ret->content = ctxt->content;
	ctxt->content = NULL;
}

typedef void (*CtxtStep)(IncludeContextPtr ctxt);

static bool ProcessInclude(xplCommandInfoPtr commandInfo, xmlNodePtr *ret)
{
	CtxtStep steps[] = 
	{
		getSourceStep,
		getFormatsStep,
		fetchContentStep,
		recodeStep,
		cleanStep,
		buildDocumentStep,
		selectNodesStep,
		computeRetStep
	};
	const int step_count = sizeof(steps) / sizeof(steps[0]);
	int i;
	IncludeContext ctxt;

	initIncludeContext(&ctxt, commandInfo);
	/* run the sequence */
	for (i = 0; i < step_count; i++)
	{
		steps[i](&ctxt);
		if (ctxt.error)
		{
			clearIncludeContext(&ctxt);
			*ret = ctxt.error;
			return false;
		}
	}
	*ret = ctxt.ret;
	clearIncludeContext(&ctxt);
	return true;
}

void xplCmdIncludePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdIncludeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlNodePtr ret, error;
	bool repeat;

	if (ProcessInclude(commandInfo, &ret)) /* ok */
	{
		if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, true)))
		{
			ASSIGN_RESULT(error, true, true);
		} else {
			ASSIGN_RESULT(ret, repeat, true);
		}
	} else {
		ASSIGN_RESULT(ret, true, true);
	}
}

xplCommand xplIncludeCommand = { xplCmdIncludePrologue, xplCmdIncludeEpilogue };
