#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>
#include <string.h>

void xplCmdUnstringerEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdUnstringerParams
{
	xmlXPathObjectPtr select;
	xmlChar* delimiter;
	xmlChar* start_delimiter;
	xmlChar* end_delimiter;
	xplQName tag_name;
	xplQName delimiter_tag_name;
	bool unique;
	bool keep_delimiter;
	bool multi_delimiter;
	bool keep_empty_tags;
	bool repeat;
} xplCmdUnstringerParams, *xplCmdUnstringerParamsPtr;

static const xplCmdUnstringerParams params_stencil =
{
	.select = NULL,
	.delimiter = NULL,
	.start_delimiter = NULL,
	.end_delimiter = NULL,
	.tag_name = { NULL, NULL },
	.delimiter_tag_name = { NULL, NULL },
	.unique = false,
	.keep_delimiter = false,
	.multi_delimiter = false,
	.keep_empty_tags = false,
	.repeat = true
};

static xmlChar* delimiter_aliases[] = { BAD_CAST "delim", BAD_CAST "delimeter", NULL };
static xmlChar* start_delimiter_aliases[] = { BAD_CAST "startdelimeter", NULL };
static xmlChar* end_delimiter_aliases[] = { BAD_CAST "enddelimeter", NULL };
static xmlChar* keep_delimiter_aliases[] = { BAD_CAST "keepdelimeter", NULL };
static xmlChar* multi_delimiter_aliases[] = { BAD_CAST "multidelimeter", NULL };
static xmlChar* delimiter_tag_name_aliases[] = { BAD_CAST "delimetertagname", NULL };

xplCommand xplUnstringerCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdUnstringerEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE | XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdUnstringerParams),
	.parameters = {
		{
			.name = BAD_CAST "select",
			.type = XPL_CMD_PARAM_TYPE_XPATH,
			.extra.xpath_type = XPL_CMD_PARAM_XPATH_TYPE_ANY,
			.value_stencil = &params_stencil.select
		}, {
			.name = BAD_CAST "delimiter",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.aliases = delimiter_aliases,
			.value_stencil = &params_stencil.delimiter
		}, {
			.name = BAD_CAST "startdelimiter",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.aliases = start_delimiter_aliases,
			.value_stencil = &params_stencil.start_delimiter
		}, {
			.name = BAD_CAST "enddelimiter",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.aliases = end_delimiter_aliases,
			.value_stencil = &params_stencil.end_delimiter,
		}, {
			.name = BAD_CAST "tagname",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.required = true,
			.value_stencil = &params_stencil.tag_name
		}, {
			.name = BAD_CAST "delimitertagname",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.aliases = delimiter_tag_name_aliases,
			.value_stencil = &params_stencil.delimiter_tag_name
		}, {
			.name = BAD_CAST "unique",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.unique
		}, {
			.name = BAD_CAST "keepdelimiter",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.aliases = keep_delimiter_aliases,
			.value_stencil = &params_stencil.keep_delimiter
		}, {
			.name = BAD_CAST "multidelimiter",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.aliases = multi_delimiter_aliases,
			.value_stencil = &params_stencil.multi_delimiter
		}, {
			.name = BAD_CAST "keepemptytags",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.keep_empty_tags
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = NULL
		}
	}
};

typedef struct _UnstringerContext
{
	xplCmdUnstringerParamsPtr params;
	xmlChar* input_str;
	xmlHashTablePtr unique_hash;
	xmlDocPtr doc;
} UnstringerContext;
typedef UnstringerContext *UnstringerContextPtr;

static xmlChar *_getEnd(xmlChar *str)
{
	while (*str)
		str++;
	return str;
}

static xmlChar *_getNextMulti(xmlChar *str, xmlChar *delim, size_t *delimLen)
{
	xmlChar *ret = NULL;
	const xmlChar *cur;
	xmlChar tmp;
	size_t offset;

	while (*delim)
	{
		offset = xstrGetOffsetToNextUTF8Char(delim);
		tmp = delim[offset];
		delim[offset] = 0;
		cur = xmlStrstr(str, delim);
		if (cur && ((cur < ret) || !ret))
		{
			ret = BAD_CAST cur;
			*delimLen = offset;
		}
		delim[offset] = tmp;
		delim += offset;
	}
	return ret;
}

static xmlNodePtr _splitBySingle(UnstringerContextPtr ctxt)
{
	xmlNodePtr ret, cur, tail;
	xmlChar *start;
	xmlChar *cur_end = NULL;
	size_t delim_len;
	size_t multi_delim_len = 0;
	xmlChar tmp;
	char marker;

#define APPEND_NODE() do {\
		if (!ret) \
			ret = tail = cur; \
		else { \
			tail->next = cur; \
			cur->prev = tail; \
			tail = cur; \
		}\
	}while(0);

	start = ctxt->input_str;
	ret = tail = NULL;
	delim_len = ctxt->params->delimiter? (size_t) xmlStrlen(ctxt->params->delimiter): 0;
	while (*start)
	{
		if (delim_len)
		{
			if (ctxt->params->multi_delimiter)
				cur_end = _getNextMulti(start, ctxt->params->delimiter, &multi_delim_len);
			else
				cur_end = BAD_CAST xmlStrstr(start, ctxt->params->delimiter);
			if (cur_end)
			{
				if (cur_end != start)
				{
					tmp = *cur_end;
					*cur_end = 0;
					if (!ctxt->params->unique || (xmlHashAddEntry(ctxt->unique_hash, start, &marker) != -1))
					{ 
						cur = xmlNewDocNode(ctxt->doc, ctxt->params->tag_name.ns, ctxt->params->tag_name.ncname, start);
						APPEND_NODE();
					} 
					*cur_end = tmp;
				} else if (ctxt->params->keep_empty_tags) {
					if (!ctxt->params->unique || (xmlHashAddEntry(ctxt->unique_hash, NULL, &marker) != -1))
					{ 
						cur = xmlNewDocNode(ctxt->doc, ctxt->params->tag_name.ns, ctxt->params->tag_name.ncname, NULL);
						APPEND_NODE();
					} 
				}
				if (ctxt->params->keep_delimiter)
				{
					cur = xmlNewDocNode(ctxt->doc, ctxt->params->delimiter_tag_name.ns, ctxt->params->delimiter_tag_name.ncname, NULL);
					cur->children = xmlNewDocText(ctxt->doc, NULL); 
					cur->children->content = xmlStrndup(cur_end, (ctxt->params->multi_delimiter)? (int) multi_delim_len: (int) delim_len);
					APPEND_NODE();
				}
				start = cur_end + ((ctxt->params->multi_delimiter)? multi_delim_len: delim_len);
				if (!*start && ctxt->params->keep_empty_tags)
				{
					if (!ctxt->params->unique || (xmlHashAddEntry(ctxt->unique_hash, NULL, &marker) != -1))
					{ 
						cur = xmlNewDocNode(ctxt->doc, ctxt->params->tag_name.ns, ctxt->params->tag_name.ncname, NULL);
						APPEND_NODE();
					} 
				}
			} else {
				if (!ctxt->params->unique || (xmlHashAddEntry(ctxt->unique_hash, start, &marker) != -1))
				{ 
					cur = xmlNewDocNode(ctxt->doc, ctxt->params->tag_name.ns, ctxt->params->tag_name.ncname, start);
					APPEND_NODE();
				} 
				start = _getEnd(start);
			}
		} else { // zero delimiter = split by chars
			cur_end = start + xstrGetOffsetToNextUTF8Char(start);
			tmp = *cur_end;
			*cur_end = 0;
			if (!ctxt->params->unique || (xmlHashAddEntry(ctxt->unique_hash, start, &marker) != -1))
			{ 
				cur = xmlNewDocNode(ctxt->doc, ctxt->params->tag_name.ns, ctxt->params->tag_name.ncname, start);
				APPEND_NODE();
			} 
			*cur_end = tmp;
			start = cur_end;
		}
	}
	return ret;
#undef APPEND_NODE
}

static xmlNodePtr _splitByCouple(UnstringerContextPtr ctxt)
{
	/* ToDo */
	UNUSED_PARAM(ctxt);
	return NULL;
}

void xplCmdUnstringerEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlXPathObjectPtr sel = NULL;
	UnstringerContext ctxt;
	ssize_t i;
	xmlNodePtr ret = NULL, tail = NULL, out, cur;

	memset(&ctxt, 0, sizeof(UnstringerContext));
	ctxt.params = (xplCmdUnstringerParamsPtr) commandInfo->params;

	if (ctxt.params->delimiter && (ctxt.params->start_delimiter || ctxt.params->end_delimiter))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "can't perform single delimiter and coupled delimiter splitting at the same time"), true, true);
		return;
	}
	if (ctxt.params->multi_delimiter && ctxt.params->start_delimiter && ctxt.params->end_delimiter && (xmlStrlen(ctxt.params->start_delimiter) != xmlStrlen(ctxt.params->end_delimiter)))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "multi-delimiter splitting is requested, but lengths of start and end delimiters differ"), true, true);
		return;
	}

	if (!ctxt.params->delimiter_tag_name.ncname)
		ctxt.params->delimiter_tag_name = ctxt.params->tag_name;
	ctxt.doc = commandInfo->element->doc;
	if (ctxt.params->unique)
		ctxt.unique_hash = xmlHashCreate(16);
	if (ctxt.params->select)
	{
		i = 0;
		cur = NULL;
		if (ctxt.params->select->type == XPATH_NODESET && ctxt.params->select->nodesetval && ctxt.params->select->nodesetval->nodeNr)
			cur = ctxt.params->select->nodesetval->nodeTab[0];
		else if (ctxt.params->select->type == XPATH_STRING) {
			ctxt.input_str = sel->stringval;
			if (ctxt.params->start_delimiter && ctxt.params->end_delimiter)
				ret = _splitByCouple(&ctxt);
			else
				ret = _splitBySingle(&ctxt);
		}
	} else
		cur = commandInfo->element->children;
	while (cur)
	{
		if (cur->type == XML_ELEMENT_NODE || cur->type == XML_ATTRIBUTE_NODE)
			ctxt.input_str = xmlNodeListGetString(cur->doc, cur->children, 1);
		else if ((cur->type == XML_TEXT_NODE || cur->type == XML_CDATA_SECTION_NODE) && cur->content)
			ctxt.input_str = BAD_CAST XPL_STRDUP((char*) cur->content);
		else
			continue; // TODO warn?
		if (!ctxt.input_str)
			continue;
		if (ctxt.params->start_delimiter && ctxt.params->end_delimiter)
			out = _splitByCouple(&ctxt);
		else
			out = _splitBySingle(&ctxt);
		if (!ret)
			ret = out;
		else
			xplAppendList(tail, out);
		tail = xplFindTail(out);
		XPL_FREE(ctxt.input_str);
		if (ctxt.params->select)
		{
			if (++i >= ctxt.params->select->nodesetval->nodeNr)
				break;
			cur = ctxt.params->select->nodesetval->nodeTab[i];
		} else
			cur = cur->next;
	}
	xplDownshiftNodeListNsDef(ret, commandInfo->element->nsDef);
	ASSIGN_RESULT(ret, ctxt.params->repeat, true);

	if (ctxt.unique_hash)
		xmlHashFree(ctxt.unique_hash, NULL);
	if (ctxt.params->delimiter_tag_name.ncname == ctxt.params->tag_name.ncname)
		ctxt.params->delimiter_tag_name.ncname = NULL;
}
