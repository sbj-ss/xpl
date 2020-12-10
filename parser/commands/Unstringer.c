#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>
#include <string.h>

void xplCmdUnstringerEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _UnstringerContext
{
	xmlChar* delimiter;
	xmlChar* start_delimiter;
	xmlChar* end_delimiter;
	xmlChar* tag_name;
	xmlChar* delimiter_tag_name;
	bool unique;
	bool keep_delimiter;
	bool multi_delimiter;
	bool keep_empty_tags;

	xmlChar* input_str;
	xmlHashTablePtr unique_hash;
	xmlDocPtr doc;
	xmlNsPtr ns;
	xmlNsPtr delimiter_ns;
} UnstringerContext;
typedef UnstringerContext *UnstringerContextPtr;

static xmlChar *_getEnd(xmlChar *str)
{
	while (*str) str++;
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
	delim_len = ctxt->delimiter? (size_t) xmlStrlen(ctxt->delimiter): 0;
	while (*start)
	{
		if (delim_len)
		{
			if (ctxt->multi_delimiter)
				cur_end = _getNextMulti(start, ctxt->delimiter, &multi_delim_len);
			else
				cur_end = BAD_CAST xmlStrstr(start, ctxt->delimiter);
			if (cur_end)
			{
				if (cur_end != start)
				{
					tmp = *cur_end;
					*cur_end = 0;
					if (!ctxt->unique || (xmlHashAddEntry(ctxt->unique_hash, start, &marker) != -1)) 
					{ 
						cur = xmlNewDocNode(ctxt->doc, ctxt->ns, ctxt->tag_name, start); 
						APPEND_NODE();
					} 
					*cur_end = tmp;
				} else if (ctxt->keep_empty_tags) {
					if (!ctxt->unique || (xmlHashAddEntry(ctxt->unique_hash, NULL, &marker) != -1)) 
					{ 
						cur = xmlNewDocNode(ctxt->doc, ctxt->ns, ctxt->tag_name, NULL); 
						APPEND_NODE();
					} 
				}
				if (ctxt->keep_delimiter)
				{
					cur = xmlNewDocNode(ctxt->doc, ctxt->delimiter_ns, ctxt->delimiter_tag_name, NULL); 
					cur->children = xmlNewDocText(ctxt->doc, NULL); 
					cur->children->content = xmlStrndup(cur_end, (ctxt->multi_delimiter)? (int) multi_delim_len: (int) delim_len);
					APPEND_NODE();
				}
				start = cur_end + ((ctxt->multi_delimiter)? multi_delim_len: delim_len);
				if (!*start && ctxt->keep_empty_tags)
				{
					if (!ctxt->unique || (xmlHashAddEntry(ctxt->unique_hash, NULL, &marker) != -1)) 
					{ 
						cur = xmlNewDocNode(ctxt->doc, ctxt->ns, ctxt->tag_name, NULL); 
						APPEND_NODE();
					} 
				}
			} else {
				if (!ctxt->unique || (xmlHashAddEntry(ctxt->unique_hash, start, &marker) != -1)) 
				{ 
					cur = xmlNewDocNode(ctxt->doc, ctxt->ns, ctxt->tag_name, start); 
					APPEND_NODE();
				} 
				start = _getEnd(start);
			}
		} else {
			cur_end = start + xstrGetOffsetToNextUTF8Char(start);
			tmp = *cur_end;
			*cur_end = 0;
			if (!ctxt->unique || (xmlHashAddEntry(ctxt->unique_hash, start, &marker) != -1)) 
			{ 
				cur = xmlNewDocNode(ctxt->doc, ctxt->ns, ctxt->tag_name, start); 
				APPEND_NODE();
			} 
			*cur_end = tmp;
			start = cur_end;
		}
	}
	return ret;
#undef CREATE_NODE
#undef APPEND_NODE
}

static xmlNodePtr _splitByCouple(UnstringerContextPtr ctxt)
{
	/* ToDo */
	return NULL;
}

void xplCmdUnstringerEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define SELECT_ATTR (BAD_CAST "select")
#define UNIQUE_ATTR (BAD_CAST "unique")
#define TAG_NAME_ATTR (BAD_CAST "tagname")
#define KEEP_EMPTY_TAGS_ATTR (BAD_CAST "keepemptytags")
#define REPEAT_ATTR (BAD_CAST "repeat")

#define DELIMITER_ATTR (BAD_CAST "delimiter")
#define DELIM_ATTR (BAD_CAST "delim")
#define START_DELIMITER_ATTR (BAD_CAST "startdelimiter")
#define END_DELIMITER_ATTR  (BAD_CAST "enddelimiter")
#define DELIMITER_TAG_NAME_ATTR (BAD_CAST "delimitertagname")
#define KEEP_DELIMITER_ATTR (BAD_CAST "keepdelimiter")
#define MULTI_DELIMITER_ATTR (BAD_CAST "multidelimiter")
/* Lucifer compatibility */
# define DELIMeTER_ATTR (BAD_CAST "delimeter")
# define START_DELIMeTER_ATTR (BAD_CAST "startdelimeter")
# define END_DELIMeTER_ATTR  (BAD_CAST "enddelimeter")
# define DELIMeTER_TAG_NAME_ATTR (BAD_CAST "delimetertagname")
# define KEEP_DELIMeTER_ATTR (BAD_CAST "keepdelimeter")
# define MULTI_DELIMeTER_ATTR (BAD_CAST "multidelimeter")

	xmlChar *select_attr = NULL;
	xmlChar *tag_name_attr = NULL;
	xmlChar *delimiter_tag_name_attr = NULL;
	bool repeat = true;
	xmlNodePtr ret = NULL, error;
	xmlXPathObjectPtr sel = NULL;
	UnstringerContext ctxt;
	size_t i;
	xmlNodePtr tail, out, cur;

	memset(&ctxt, 0, sizeof(UnstringerContext));
	tag_name_attr = xmlGetNoNsProp(commandInfo->element, TAG_NAME_ATTR);
	/* ToDo: check if tag name is valid */
	if (!tag_name_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "missing tagname attribute"), true, true);
		return;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, UNIQUE_ATTR, &ctxt.unique, false)))
	{
		ASSIGN_RESULT(error, true, true);
		return;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, true)))
	{
		ASSIGN_RESULT(error, true, true);
		return;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, KEEP_EMPTY_TAGS_ATTR, &ctxt.keep_empty_tags, false)))
	{
		ASSIGN_RESULT(error, true, true);
		return;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, KEEP_DELIMITER_ATTR, &ctxt.keep_delimiter, false)))
	{
		ASSIGN_RESULT(error, true, true);
		return;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, MULTI_DELIMITER_ATTR, &ctxt.multi_delimiter, false)))
	{
		ASSIGN_RESULT(error, true, true);
		return;
	}
	if (cfgLuciferCompat)
	{
		if ((error = xplDecodeCmdBoolParam(commandInfo->element, KEEP_DELIMeTER_ATTR, &ctxt.keep_delimiter, ctxt.keep_delimiter)))
		{
			ASSIGN_RESULT(error, true, true);
			return;
		}
		if ((error = xplDecodeCmdBoolParam(commandInfo->element, MULTI_DELIMeTER_ATTR, &ctxt.multi_delimiter, ctxt.multi_delimiter)))
		{
			ASSIGN_RESULT(error, true, true);
			return;
		}
	}
	select_attr = xmlGetNoNsProp(commandInfo->element, SELECT_ATTR);
	delimiter_tag_name_attr = xmlGetNoNsProp(commandInfo->element, DELIMITER_TAG_NAME_ATTR);

	ctxt.delimiter = xmlGetNoNsProp(commandInfo->element, DELIMITER_ATTR);
	if (!ctxt.delimiter)
		ctxt.delimiter = xmlGetNoNsProp(commandInfo->element, DELIM_ATTR);
	ctxt.start_delimiter = xmlGetNoNsProp(commandInfo->element, START_DELIMITER_ATTR);
	ctxt.end_delimiter = xmlGetNoNsProp(commandInfo->element, END_DELIMITER_ATTR);	
	if (cfgLuciferCompat)
	{
		if (!ctxt.delimiter)
			ctxt.delimiter = xmlGetNoNsProp(commandInfo->element, DELIMeTER_ATTR);
		if (!ctxt.start_delimiter)
			ctxt.start_delimiter = xmlGetNoNsProp(commandInfo->element, START_DELIMeTER_ATTR);
		if (!ctxt.end_delimiter)
			ctxt.end_delimiter = xmlGetNoNsProp(commandInfo->element, END_DELIMeTER_ATTR);
		if (!ctxt.delimiter_tag_name)
			ctxt.delimiter_tag_name = xmlGetNoNsProp(commandInfo->element, DELIMeTER_TAG_NAME_ATTR);
	}

	if (ctxt.delimiter && (ctxt.start_delimiter || ctxt.end_delimiter))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "can't perform single delimiter and coupled delimiter splitting at the same time"), true, true);
		goto done;
	}
	if (ctxt.multi_delimiter && ctxt.start_delimiter && ctxt.end_delimiter && (xmlStrlen(ctxt.start_delimiter) != xmlStrlen(ctxt.end_delimiter)))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "multi-delimiter splitting is requested, but lengths of start and end delimiters differ"), true, true);
		goto done;
	}

	if (!select_attr)
		select_attr = BAD_CAST XPL_STRDUP(".");
	EXTRACT_NS_AND_TAGNAME(tag_name_attr, ctxt.ns, ctxt.tag_name, commandInfo->element)
	EXTRACT_NS_AND_TAGNAME(delimiter_tag_name_attr, ctxt.delimiter_ns, ctxt.delimiter_tag_name, commandInfo->element);
	if (!ctxt.delimiter_tag_name)
		ctxt.delimiter_tag_name = ctxt.tag_name;

	ctxt.doc = commandInfo->element->doc;
	if (ctxt.unique)
		ctxt.unique_hash = xmlHashCreate(16);
	sel = xplSelectNodes(commandInfo, commandInfo->element, select_attr);
	if (sel)
	{
		if (sel->type == XPATH_NODESET)
		{
			if (sel->nodesetval)
			{
				for (i = 0; i < (size_t) sel->nodesetval->nodeNr; i++)
				{
					cur = sel->nodesetval->nodeTab[i];
					if ((cur->type != XML_ELEMENT_NODE && cur->type != XML_ATTRIBUTE_NODE))
						continue;
					ctxt.input_str = xmlNodeListGetString(cur->doc, cur->children, 1);
					if (!ctxt.input_str)
						continue;
					if (ctxt.start_delimiter && ctxt.end_delimiter)
						out = _splitByCouple(&ctxt);
					else
						out = _splitBySingle(&ctxt);
					if (!ret)
						ret = out;
					else
						xplAppendList(tail, out);
					tail = xplFindTail(out);
					XPL_FREE(ctxt.input_str);
				}
			}
		} else if (sel->type == XPATH_STRING) {
			ctxt.input_str = sel->stringval;
			if (ctxt.start_delimiter && ctxt.end_delimiter)
				ret = _splitByCouple(&ctxt);
			else
				ret = _splitBySingle(&ctxt);
		} else {
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath expression \"%s\" evaluated to neither nodeset nor string value", select_attr), true, true);
			goto done;
		}
	} else {
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid select XPath expression \"%s\"", select_attr), true, true);
		goto done;
	}
	xplDownshiftNodeListNsDef(ret, commandInfo->element->nsDef);
	ASSIGN_RESULT(ret, repeat, true);
done:
	if (select_attr)
		XPL_FREE(select_attr);
	if (tag_name_attr)
		XPL_FREE(tag_name_attr);
	if (delimiter_tag_name_attr)
		XPL_FREE(delimiter_tag_name_attr);

	if (ctxt.delimiter)
		XPL_FREE(ctxt.delimiter);
	if (ctxt.start_delimiter)
		XPL_FREE(ctxt.start_delimiter);
	if (ctxt.end_delimiter)
		XPL_FREE(ctxt.end_delimiter);
	if (ctxt.unique_hash)
		xmlHashFree(ctxt.unique_hash, NULL);

	if (sel)
		xmlXPathFreeObject(sel);
}

xplCommand xplUnstringerCommand = { NULL, xplCmdUnstringerEpilogue };
