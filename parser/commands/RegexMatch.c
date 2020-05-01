#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>
#include "commands/RegexMatch.h"
#include <oniguruma.h>

void xplCmdRegexMatchPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdRegexMatchEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REGEX_ATTR (BAD_CAST "regex")

	xmlChar *regex_attr = NULL;
	xmlChar *content = NULL, *content_end;
	regex_t *regex = NULL;
	OnigErrorInfo err_info;
	OnigRegion *region = NULL;
	xmlChar err_str[ONIG_MAX_ERROR_MESSAGE_LEN+1];
	bool match;
	xmlNodePtr ret;
	int ret_code;

	regex_attr = xmlGetNoNsProp(commandInfo->element, REGEX_ATTR);
	if (!regex_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "regex attribute is missing"), true, true);
		return;
	}
	if (!checkNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "non-text nodes inside"), true, true);
		goto done;
	}
	content = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, true);
	/* ToDo: maybe some options controlled by XPL programmer? */
	if ((ret_code = onig_new(&regex, regex_attr, regex_attr + xmlStrlen(regex_attr),
		ONIG_OPTION_NONE, ONIG_ENCODING_UTF8, ONIG_SYNTAX_PERL_NG, &err_info)) != ONIG_NORMAL)
	{
		if (!onig_error_code_to_str(err_str, ret_code))
			strcpy((char*) err_str, "unknown error");
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "Oniguruma error: %s", err_str), true, true);
		goto done;
	}
	region = onig_region_new();
	if (!region)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "Oniguruma cannot create search region"), true, true);
		goto done;
	}
	content_end = content + xmlStrlen(content);
#ifdef _REGEX_MATCH_FULL_STRING_ONLY
	match = (onig_match(regex, content, content_end, content, region, 0) != ONIG_MISMATCH);
#else
	match = (onig_search(regex, content, content_end, content, content_end, region, 0) != ONIG_MISMATCH);
#endif
	ret = xmlNewDocText(commandInfo->element->doc, NULL);
	if (match)
		ret->content = xmlStrdup(BAD_CAST "true");
	else
		ret->content = xmlStrdup(BAD_CAST "false");
	ASSIGN_RESULT(ret, false, true);
done:
	if (regex_attr)
		xmlFree(regex_attr);
	if (content)
		xmlFree(content);
	if (regex)
		onig_free(regex);
	if (region)
		onig_region_free(region, 1);
}

xplCommand xplRegexMatchCommand = { xplCmdRegexMatchPrologue, xplCmdRegexMatchEpilogue };

