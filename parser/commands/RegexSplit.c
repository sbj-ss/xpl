#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplutils.h>
#include "commands/RegexSplit.h"
#include <oniguruma.h>

void xplCmdRegexSplitPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdRegexSplitEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define SELECT_ATTR (BAD_CAST "select")
#define REGEX_ATTR (BAD_CAST "regex")
#define DELIMITER_ATTR (BAD_CAST "delimiter")
#define REPEAT_ATTR (BAD_CAST "repeat")
#define TAGNAME_ATTR (BAD_CAST "tagname")
#define DELIMITERTAGNAME_ATTR (BAD_CAST "delimitertagname")
#define KEEPDELIMITER_ATTR (BAD_CAST "keepdelimiter")
#define KEEPEMPTYTAGS_ATTR (BAD_CAST "keepemptytags")
#define UNIQUE_ATTR (BAD_CAST "unique")

#define APPEND() \
	{\
		if (!ret)\
			ret = tail = cur;\
		else {\
			tail->next = cur;\
			cur->prev = tail;\
			tail = cur;\
		}\
	}

	xmlChar *select_attr = NULL;
	xmlChar *regex_attr = NULL;
	xmlChar *tagname_attr = NULL;
	xmlChar *delimitertagname_attr = NULL;
	xmlChar *tagname, *delimitertagname;
	xmlChar tmp;
	xmlNsPtr ns, delim_ns;
	bool keepdelimiter;
	bool keepemptytags;
	bool repeat;
	bool unique;
	xmlChar *content = NULL, *content_end;
	regex_t *regex = NULL;
	OnigErrorInfo err_info;
	OnigRegion *region = NULL;
	xmlChar onig_err_str[ONIG_MAX_ERROR_MESSAGE_LEN+1];
	int onig_ret_code;
	xmlNodePtr ret = NULL, tail, cur, error;
	bool match;
	xmlChar *start, *end, *prev_boundary;
	xmlHashTablePtr unique_hash = NULL;
	xmlXPathObjectPtr sel = NULL;
	size_t i;
	char marker; 

	regex_attr = xmlGetNoNsProp(commandInfo->element, REGEX_ATTR);
	if (!regex_attr)
		regex_attr = xmlGetNoNsProp(commandInfo->element, DELIMITER_ATTR);
	if (!regex_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "missing regex attribute"), true, true);
		goto done;
	}
	tagname_attr = xmlGetNoNsProp(commandInfo->element, TAGNAME_ATTR);
	if (!tagname_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "missing tagname attribute"), true, true);
		goto done;
	}
	select_attr = xmlGetNoNsProp(commandInfo->element, BAD_CAST "select");
	if (!select_attr)
		select_attr = xmlStrdup(BAD_CAST ".");
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, KEEPDELIMITER_ATTR, &keepdelimiter, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if (keepdelimiter)
	{
		delimitertagname_attr = xmlGetNoNsProp(commandInfo->element, DELIMITERTAGNAME_ATTR);
		if (!delimitertagname_attr)
			delimitertagname_attr = tagname_attr;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, KEEPEMPTYTAGS_ATTR, &keepemptytags, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, true)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, UNIQUE_ATTR, &unique, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if (unique)
		unique_hash = xmlHashCreate(16);

	if ((onig_ret_code = onig_new(&regex, regex_attr, regex_attr + xmlStrlen(regex_attr),
		ONIG_OPTION_NONE, ONIG_ENCODING_UTF8, ONIG_SYNTAX_PERL_NG, &err_info)) != ONIG_NORMAL)
	{
		if (!onig_error_code_to_str(onig_err_str, onig_ret_code))
			strcpy((char*) onig_err_str, "unknown error");
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "Oniguruma error: %s", onig_err_str), true, true);
		goto done;
	}
	region = onig_region_new();
	if (!region)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "Oniguruma cannot create search region"), true, true);
		goto done;
	}
	EXTRACT_NS_AND_TAGNAME(tagname_attr, ns, tagname, commandInfo->element);
	EXTRACT_NS_AND_TAGNAME(delimitertagname_attr, delim_ns, delimitertagname, commandInfo->element);

	sel = xplSelectNodes(commandInfo->document, commandInfo->element, select_attr);
	if (!sel)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid select XPath expression \"%s\"", select_attr), true, true);
		goto done;
	}
	if (sel->type != XPATH_NODESET)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath expression \"%s\" evaluated to non-nodeset value", select_attr), true, true);
		goto done;
	}
	if (!sel->nodesetval)
	{
		ASSIGN_RESULT(NULL, false, true);
		goto done;
	}
	for (i = 0; i < (size_t) sel->nodesetval->nodeNr; i++)
	{
		if ((sel->nodesetval->nodeTab[i]->type != XML_ELEMENT_NODE) && (sel->nodesetval->nodeTab[i]->type != XML_ATTRIBUTE_NODE))
			continue;
		content = xmlNodeListGetString(sel->nodesetval->nodeTab[i]->doc, sel->nodesetval->nodeTab[i]->children, 1);
		if (!content || !*content)
		{
			if (keepemptytags)
			{
				cur = xmlNewDocNode(commandInfo->element->doc, ns, tagname, NULL);
				APPEND()
			}
			if (content) xmlFree(content);
			continue;
		}
		content_end = content + xmlStrlen(content);
		match = (onig_search(regex, content, content_end, content, content_end, region, 0) != ONIG_MISMATCH);
		if (!match)
		{
			cur = xmlNewDocNode(commandInfo->element->doc, ns, tagname, NULL);
			cur->children = xmlNewDocText(commandInfo->element->doc, NULL);
			cur->children->content = content;
			APPEND()
		} else {
			prev_boundary = start = content;
			while (match && *start)
			{
				end = content + (size_t) region->beg[0];
				if ((end > prev_boundary) || keepemptytags)
				{
					tmp = *end;
					*end = 0;
					if (!unique || (xmlHashAddEntry(unique_hash, prev_boundary, &marker) != -1))
						cur = xmlNewDocNode(commandInfo->element->doc, ns, tagname, prev_boundary);
					*end = tmp;
					APPEND()
				}
				end = content + (size_t) region->end[0];
				if (keepdelimiter)
				{
					start = content + (size_t) region->beg[0];
					tmp = *end;
					*end = 0;
					cur = xmlNewDocNode(commandInfo->element->doc, delim_ns, delimitertagname, start);
					*end = tmp;
					APPEND()
				}
				if ((region->beg[0] == region->end[0]) && *start) /* zero-length match */
					start = end + getOffsetToNextUTF8Char(end);
				else
					start = end;
				prev_boundary = end;
				match = (onig_search(regex, content, content_end, start, content_end, region, 0) != ONIG_MISMATCH);
			} /* inner loop */
			if (*prev_boundary || keepemptytags)
			{
				if (!*prev_boundary)
					prev_boundary = NULL;
				if (!unique || (xmlHashAddEntry(unique_hash, prev_boundary, &marker) != -1))
					cur = xmlNewDocNode(commandInfo->element->doc, ns, tagname, prev_boundary);
				APPEND()
			} 
			xmlFree(content);
		} /* first match found */
	} /* outer loop */
	ASSIGN_RESULT(ret, repeat, true);
done:
	if (select_attr)
		xmlFree(select_attr);
	if (regex_attr)
		xmlFree(regex_attr);
	if (delimitertagname_attr && (delimitertagname_attr != tagname_attr))
		xmlFree(delimitertagname_attr);
	if (tagname_attr)
		xmlFree(tagname_attr);
	if (unique_hash)
		xmlHashFree(unique_hash, NULL);
	if (regex)
		onig_free(regex);
	if (region)
		onig_region_free(region, 1);
	if (sel)
		xmlXPathFreeObject(sel);
}

xplCommand xplRegexSplitCommand = { xplCmdRegexSplitPrologue, xplCmdRegexSplitEpilogue };
