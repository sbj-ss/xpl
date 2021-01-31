#include <libxpl/abstraction/xef.h>
#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplsave.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

void xplCmdIncludeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

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
	OUTPUT_FORMAT_HEX,
	OUTPUT_FORMAT_UNSPECIFIED
} OutputFormat;

typedef struct _xplCmdIncludeParams
{
	xmlChar *select; // we can't make use of built-in XPath management here
	xmlChar *uri;
	bool repeat;
	xplQName tag_name;
	xmlChar *encoding;
	bool abs_path;
	InputFormat input_format;
	OutputFormat output_format;
	xmlChar *post_data;
	bool remove_source;
} xplCmdIncludeParams, *xplCmdIncludeParamsPtr;

static const xplCmdIncludeParams params_stencil =
{
	.select = NULL,
	.uri = NULL,
	.repeat = true,
	.tag_name = { NULL, NULL },
	.encoding = BAD_CAST "utf-8",
	.abs_path = false,
	.input_format = INPUT_FORMAT_XML,
	.output_format = OUTPUT_FORMAT_UNSPECIFIED,
	.post_data = NULL,
	.remove_source = false
};

static xmlChar* select_aliases[] = { BAD_CAST "source", NULL };
static xmlChar* tagname_aliases[] = { BAD_CAST "responsetagname", NULL };
static xmlChar* uri_aliases[] = { BAD_CAST "file", NULL };

static xplCmdParamDictValue input_format_dict[] = {
	{ .name = BAD_CAST "xml", .value = INPUT_FORMAT_XML },
	{ .name = BAD_CAST "html", .value = INPUT_FORMAT_HTML },
	{ .name = BAD_CAST "raw", .value = INPUT_FORMAT_RAW },
	{ .name = NULL }
};
static xplCmdParamDictValue output_format_dict[] = {
	{ .name = BAD_CAST "xml", .value = OUTPUT_FORMAT_XML },
	{ .name = BAD_CAST "text", .value = OUTPUT_FORMAT_TEXT },
	{ .name = BAD_CAST "base64", .value = OUTPUT_FORMAT_BASE64 },
	{ .name = BAD_CAST "hex", .value = OUTPUT_FORMAT_HEX },
	{ .name = NULL }
};

xplCommand xplIncludeCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdIncludeEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdIncludeParams),
	.parameters = {
		{
			.name = BAD_CAST "select",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.aliases = select_aliases,
			.value_stencil = &params_stencil.select
		}, {
			.name = BAD_CAST "uri",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.aliases = uri_aliases,
			.value_stencil = &params_stencil.uri
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = BAD_CAST "tagname",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.aliases = tagname_aliases,
			.value_stencil = &params_stencil.tag_name
		}, {
			.name = BAD_CAST "encoding",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.encoding,
		}, {
			.name = BAD_CAST "abspath",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.abs_path
		}, {
			.name = BAD_CAST "inputformat",
			.type = XPL_CMD_PARAM_TYPE_DICT,
			.extra.dict_values = input_format_dict,
			.value_stencil = &params_stencil.input_format
		}, {
			.name = BAD_CAST "outputformat",
			.type = XPL_CMD_PARAM_TYPE_DICT,
			.extra.dict_values = output_format_dict,
			.value_stencil = &params_stencil.output_format
		}, {
			.name = BAD_CAST "postdata",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.post_data
		}, {
			.name = BAD_CAST "removesource",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.remove_source
		}, {
			.name = NULL
		}
	}
};

typedef enum 
{
	INPUT_SOURCE_LOCAL,
	INPUT_SOURCE_FILE,
	INPUT_SOURCE_XTP
} InputSource;

typedef struct _IncludeContext
{
	xplCmdIncludeParamsPtr params;
	xmlNodePtr command_element;
	xplDocumentPtr doc;
	int encoding;
	bool needs_recoding;
	/* input source info */
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
	ctxt->params = (xplCmdIncludeParamsPtr) commandInfo->params;
}

static void clearIncludeContext(IncludeContextPtr ctxt)
{
	if ((ctxt->input_source != INPUT_SOURCE_LOCAL) && ctxt->src_node)
		xmlFreeDoc((xmlDocPtr) ctxt->src_node);
	if (ctxt->content)
		XPL_FREE(ctxt->content);
}

typedef void (*CtxtStep)(IncludeContextPtr ctxt);

static void getSourceStep(IncludeContextPtr ctxt)
{
	xmlChar *uri = ctxt->params->uri;

	if (!uri)
	{
		ctxt->input_source = INPUT_SOURCE_LOCAL;
		return;
	}
	if (xmlStrstr(uri, BAD_CAST "file:///") == uri)
	{
		ctxt->params->uri = BAD_CAST XPL_STRDUP((char*) uri + 8);
		ctxt->input_source = INPUT_SOURCE_FILE;
		XPL_FREE(uri);
	} else if (
		(xmlStrstr(uri, BAD_CAST "ftp://") == uri) ||
		(xmlStrstr(uri, BAD_CAST "http://") == uri) ||
		(xmlStrstr(uri, BAD_CAST "https://") == uri)
	)
		ctxt->input_source = INPUT_SOURCE_XTP;
	else
		ctxt->input_source = INPUT_SOURCE_FILE;

	if (ctxt->params->abs_path)
	{
		ctxt->params->uri = xplFullFilename(uri, ctxt->doc->app_path);
		XPL_FREE(uri);
	}
}

static void getFormatsStep(IncludeContextPtr ctxt)
{
	if (ctxt->params->output_format == OUTPUT_FORMAT_UNSPECIFIED)
	{
		if (ctxt->params->input_format == INPUT_FORMAT_RAW)
			ctxt->params->output_format = OUTPUT_FORMAT_TEXT;
		else
			ctxt->params->output_format = OUTPUT_FORMAT_XML;
	}
	/* sanity checks */
	if ((ctxt->params->input_format == INPUT_FORMAT_RAW) && (ctxt->params->output_format == OUTPUT_FORMAT_XML))
	{
		ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "cannot create XML output from raw input");
		return;
	}
	if ((ctxt->input_source == INPUT_SOURCE_LOCAL) && (ctxt->params->input_format != INPUT_FORMAT_XML))
	{
		ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "input format for local inclusion must be xml");
		return;
	}
}

static void fetchFileContent(IncludeContextPtr ctxt)
{
	int fd;
	struct stat stat;
	size_t file_size;
	ssize_t num_read;
	char* file_content;
	
	fd = xprSOpen(ctxt->params->uri, O_BINARY | O_RDONLY, 0, 0);
	if (fd == -1)
	{
		ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "cannot open file \"%s\"", ctxt->params->uri);
		return;
	}
	fstat(fd, &stat); /* TODO where's stat64? */
	file_size = stat.st_size;
	file_content = (char*) XPL_MALLOC(file_size + 2);
	ctxt->content = BAD_CAST file_content;
	if (!file_content)
	{
		ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "insufficient memory");
		return;
	}
	ctxt->content_size = file_size;
	while (file_size > 0)
	{
		if ((num_read = read(fd, file_content, file_size > 0x10000? 0x10000: file_size)) == -1)
		{
			ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "cannot read from file \"%s\"", ctxt->params->uri);
			ctxt->content = NULL;
			ctxt->content_size = 0;
			XPL_FREE(file_content);
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
			XPL_FREE(error_text);
			xefFreeErrorMessage(params.error);
		} else
			ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "unknown fetch error");
	}
	if (params.extra_query)
		XPL_FREE(params.extra_query);
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
			return XSTR_ENC_UTF16LE;
		if (!*content)
			return XSTR_ENC_UTF16LE;
		if ((*content == (char) 0xFF) && (*(content+1) == (char) 0xFE))
			return XSTR_ENC_UTF16BE;
		if (!*(content+1))
			return XSTR_ENC_UTF16BE;
	}
	if (size >= 3)
	{
		if ((*content == (char) 0xEF) && (*(content+1) == (char) 0xBB) && (*(content+2) == (char) 0xBF))
			return XSTR_ENC_UTF8;
	}
	return xstrDetectEncoding(content, detectionSampleLen(content, size));
}

static void updateCtxtEncoding(IncludeContextPtr ctxt)
{
	if (!ctxt->params->encoding || !xmlStrcasecmp(ctxt->params->encoding, BAD_CAST "utf-8"))
	{
		ctxt->needs_recoding = false;
		return;
	}
	if (xmlStrcasecmp(ctxt->params->encoding, BAD_CAST "auto")) { /* non-utf8 encoding specified */
		ctxt->needs_recoding = true;
		return;
	}
	XPL_FREE(ctxt->params->encoding); // was "auto"
	/* run detection */
	ctxt->encoding = fastDetectEncoding((char*) ctxt->content, ctxt->content_size);
	switch (ctxt->encoding) // TODO array
	{
	case XSTR_ENC_1251:
		ctxt->params->encoding = BAD_CAST XPL_STRDUP("CP1251");
		break;
	case XSTR_ENC_UTF8: 
	case XSTR_ENC_UNKNOWN: // TODO warn
		ctxt->params->encoding = BAD_CAST XPL_STRDUP("utf-8");
		ctxt->encoding = XSTR_ENC_UTF8;
		break;
	case XSTR_ENC_UTF16LE: 
		ctxt->params->encoding = BAD_CAST XPL_STRDUP("utf-16le");
		break;
	case XSTR_ENC_UTF16BE: 
		ctxt->params->encoding = BAD_CAST XPL_STRDUP("utf-16be");
		break;
	case XSTR_ENC_866: 
		ctxt->params->encoding = BAD_CAST XPL_STRDUP("cp866");
		break;
	case XSTR_ENC_KOI8: 
		ctxt->params->encoding = BAD_CAST XPL_STRDUP("KOI8-R");
		break;
	default: 
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	}
	ctxt->needs_recoding = (ctxt->encoding != XSTR_ENC_UTF8);
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
	if (xstrIconvString(
		"utf-8",
		(const char*) ctxt->params->encoding,
		(const char*) ctxt->content,
		(const char*) ctxt->content + ctxt->content_size,
		(char**) &recoded_content,
		&recoded_size
	) == -1)
	{
		ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "error converting data from encoding \"%s\"", ctxt->params->encoding);
		return;
	}
	XPL_FREE(ctxt->content);
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
#endif
	if ((ctxt->params->input_format == INPUT_FORMAT_RAW) && (ctxt->params->output_format == OUTPUT_FORMAT_TEXT))
	{
		if (!checkForNonprintable(ctxt->content, ctxt->content_size))
		{
			ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "input document contains characters that cannot be output as text (try hex or base64 output format)");
			return;
		}
		if (!xstrIsValidUtf8Sample(ctxt->content, ctxt->content_size, true))
		{
			ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "input document contains characters that cannot be displayed as utf-8 (try recoding or hex/base64 output format)");
			return;
		}
		/* don't call xmlEncodeSpecialChars right here! */
		return;
	}

	if (ctxt->params->input_format != INPUT_FORMAT_HTML)
		return;
#ifdef _XEF_HAS_HTML_CLEANER
	params.document = ctxt->content;
	if (xefCleanHtml(&params))
	{
		XPL_FREE(ctxt->content);
		ctxt->content = params.clean_document;
		ctxt->content_size = params.clean_document_size;
	} else if (params.error) {
		ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "Error cleaning HTML document: \"%s\"", params.error);
		XPL_FREE(params.error);
	} else
		ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "cannot clean HTML document: unknown error");
#else
	ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "HTML document cleaning support not compiled in");
#endif
}

static void buildDocumentStep(IncludeContextPtr ctxt)
{
	xmlChar *error_txt;

	switch(ctxt->params->input_format)
	{
	case INPUT_FORMAT_XML:
	case INPUT_FORMAT_HTML:
		if (
			ctxt->input_source != INPUT_SOURCE_LOCAL &&
			(ctxt->params->select || ctxt->params->output_format == OUTPUT_FORMAT_XML)
		)
		{ /* we need a document somewhere on the way, let's build it */
			if (ctxt->content_size > INT_MAX) // TODO do we need this limitation?
			{
				ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "input document is bigger than 2 Gb");
				return;
			}
			ctxt->src_node = (xmlNodePtr) xmlReadMemory((char*) ctxt->content, (int) ctxt->content_size, NULL, NULL, XML_PARSE_NODICT);
			if (!ctxt->src_node)
			{
				error_txt = xstrGetLastLibxmlError();
				ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "error parsing input document: \"%s\"", error_txt);
				XPL_FREE(error_txt);
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
	xmlXPathObjectPtr sel;
	xmlNodePtr cur, head = NULL, tail, sibling;
	size_t i;

	/* check select attribute */
	if (ctxt->params->select && ctxt->params->input_format == INPUT_FORMAT_RAW)
	{
		ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "cannot select nodes from non-structured source");
		return;
	}
	if (ctxt->params->input_format == INPUT_FORMAT_RAW)
		return;
	if (!ctxt->params->select) /* return the input document/command content */
	{
		if (ctxt->input_source == INPUT_SOURCE_LOCAL)
			ctxt->ret = xplDetachChildren(ctxt->command_element);
		else {
			cur = ctxt->ret = xplDetachChildren(ctxt->src_node);
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
		if (ctxt->params->tag_name.ncname)
		{
			cur = ctxt->ret;
			while (cur)
			{
				if (cur->type == XML_ELEMENT_NODE)
				{
					xmlNodeSetName(cur, ctxt->params->tag_name.ncname);
					cur->ns = ctxt->params->tag_name.ns;
				}
				cur = cur->next;
			}
		}
		return;
	}
	/* select nodes */
	if (ctxt->input_source == INPUT_SOURCE_LOCAL)
		cur = ctxt->src_node;
	else {
		cur = ctxt->src_node->children;
		while (cur && (cur->type != XML_ELEMENT_NODE)) /* skip comments */
			cur = cur->next;
	}
	sel = xplSelectNodesWithCtxt(ctxt->doc->xpath_ctxt, cur, ctxt->params->select);
	if (sel)
	{
		if (sel->type == XPATH_NODESET)
		{
			if (sel->nodesetval)
			{
				/* There could be some speedups for OUTPUT_FORMAT_TEXT, but they would mess the code up completely. */
				for (i = 0; i < (size_t) sel->nodesetval->nodeNr; i++)
				{
					sibling = sel->nodesetval->nodeTab[i];
					cur = xplCloneAsNodeChild(sibling, ctxt->command_element);
					if (ctxt->params->tag_name.ncname && (cur->type == XML_ELEMENT_NODE))
					{
						xmlNodeSetName(cur, ctxt->params->tag_name.ncname);
						cur->ns = ctxt->params->tag_name.ns;
					}
					if (!head)
						head = tail = cur;
					else
						tail = xmlAddNextSibling(tail, cur);
					if (ctxt->params->remove_source) /* "move" instead of copy */
					{
						xmlUnlinkNode(sibling);
						if (ctxt->input_source == INPUT_SOURCE_LOCAL)
							xplDocDeferNodeDeletion(ctxt->doc, sibling);
						else
							xmlFreeNode(sibling);
						sel->nodesetval->nodeTab[i] = 0;
					}
				}
			}
		} else if (sel->type != XPATH_UNDEFINED) 
			ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "select XPath expression \"%s\" evaluated to non-nodeset value", ctxt->params->select);
		else 
			ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "select XPath expression \"%s\" evaluated to undef", ctxt->params->select);
	} else
		ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "invalid select XPath expression \"%s\"", ctxt->params->select);
	ctxt->ret = head;
	if (sel)
		xmlXPathFreeObject(sel);
}

static void computeRetStep(IncludeContextPtr ctxt)
{
	size_t encoded_size;
	xmlChar *encoded_content;

	if (ctxt->params->input_format != INPUT_FORMAT_RAW && ctxt->params->output_format != OUTPUT_FORMAT_XML && ctxt->ret)
	{
		/* we have a node list and need an optionally encoded string. */
		if (ctxt->content)
			XPL_FREE(ctxt->content);
		ctxt->content = xplSerializeNodeList(ctxt->ret);
		ctxt->content_size = xmlStrlen(ctxt->content);
		xmlFreeNodeList(ctxt->ret);
	}
	switch (ctxt->params->output_format)
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
		encoded_content = (xmlChar*) XPL_MALLOC(encoded_size);
		if (!encoded_content)
		{
			ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "insufficient memory");
			return;
		}
		xstrBase64Encode(ctxt->content, ctxt->content_size, (char*) encoded_content, encoded_size);
		XPL_FREE(ctxt->content);
		ctxt->content = encoded_content;
		ctxt->content_size = encoded_size;
		break;
	case OUTPUT_FORMAT_HEX:
		if (!(encoded_content = xstrBufferToHex(ctxt->content, ctxt->content_size, false)))
		{
			ctxt->error = xplCreateErrorNode(ctxt->command_element, BAD_CAST "insufficient memory");
			return;
		}
		XPL_FREE(ctxt->content);
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

void xplCmdIncludeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	static const CtxtStep steps[] =
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
	static const int step_count = sizeof(steps) / sizeof(steps[0]);
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
			ASSIGN_RESULT(ctxt.error, true, true);
			return;
		}
	}
	clearIncludeContext(&ctxt);
	ASSIGN_RESULT(ctxt.ret, ctxt.params->repeat, true);
}
