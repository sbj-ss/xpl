#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>
#include <string.h>
#include <oniguruma.h>

void xplCmdRegexSplitEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdRegexSplitParams
{
	xmlChar *regex;
	bool repeat;
	xplQName tagname;
	xplQName delimiter_tagname;
	bool keep_delimiter;
	bool keep_empty_tags;
	bool unique;
} xplCmdRegexSplitParams, *xplCmdRegexSplitParamsPtr;

static const xplCmdRegexSplitParams params_stencil =
{
	.regex = NULL,
	.repeat = true,
	.tagname = { NULL, NULL },
	.delimiter_tagname = { NULL, NULL },
	.keep_delimiter = false,
	.keep_empty_tags = false,
	.unique = false
};

static xmlChar* regex_aliases[] = { BAD_CAST "delimiter", NULL };

xplCommand xplRegexSplitCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdRegexSplitEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE | XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdRegexSplitParams),
	.parameters = {
		{
			.name = BAD_CAST "regex",
			.aliases = regex_aliases,
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.required = true,
			.value_stencil = &params_stencil.regex
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = BAD_CAST "tagname",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.required = true,
			.value_stencil = &params_stencil.tagname
		}, {
			.name = BAD_CAST "delimitertagname",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.value_stencil = &params_stencil.delimiter_tagname
		}, {
			.name = BAD_CAST "keepdelimiter",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.keep_delimiter
		}, {
			.name = BAD_CAST "keepemptytags",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.keep_empty_tags
		}, {
			.name = BAD_CAST "unique",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.unique
		}, {
			.name = NULL
		}
	}
};

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

	xplCmdRegexSplitParamsPtr params = (xplCmdRegexSplitParamsPtr) commandInfo->params;
	xmlChar tmp;
	xmlChar *content, *content_end;
	regex_t *regex = NULL;
	OnigErrorInfo err_info;
	OnigRegion *region = NULL;
	xmlChar onig_err_str[ONIG_MAX_ERROR_MESSAGE_LEN+1];
	int onig_ret_code;
	xmlNodePtr ret = NULL, tail = NULL, cur;
	bool match;
	xmlChar *start, *end, *prev_boundary;
	xmlHashTablePtr unique_hash = NULL;
	char marker; 

	if (params->keep_delimiter && !params->delimiter_tagname.ncname)
		params->delimiter_tagname = params->tagname;
	if (params->unique)
		unique_hash = xmlHashCreate(16);

	if ((onig_ret_code = onig_new(&regex, params->regex, params->regex + xmlStrlen(params->regex),
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

	content = commandInfo->content;
	if (!content || !*content)
	{
		if (params->keep_empty_tags)
		{
			cur = xmlNewDocNode(commandInfo->element->doc, params->tagname.ns, params->tagname.ncname, NULL);
			APPEND()
		}
		ASSIGN_RESULT(ret, params->repeat, true);
		goto done;
	}

	content_end = content + xmlStrlen(content);
	match = (onig_search(regex, content, content_end, content, content_end, region, 0) != ONIG_MISMATCH);
	if (!match)
	{
		cur = xmlNewDocNode(commandInfo->element->doc, params->tagname.ns, params->tagname.ncname, NULL);
		cur->children = xmlNewDocText(commandInfo->element->doc, NULL);
		cur->children->content = content;
		APPEND()
	} else {
		prev_boundary = start = content;
		while (match && *start)
		{
			end = content + (size_t) region->beg[0];
			if ((end > prev_boundary) || params->keep_empty_tags)
			{
				tmp = *end;
				*end = 0;
				if (!params->unique || (xmlHashAddEntry(unique_hash, prev_boundary, &marker) != -1))
				{
					cur = xmlNewDocNode(commandInfo->element->doc, params->tagname.ns, params->tagname.ncname, prev_boundary);
					APPEND()
				}
				*end = tmp;
			}
			end = content + (size_t) region->end[0];
			if (params->keep_delimiter)
			{
				start = content + (size_t) region->beg[0];
				tmp = *end;
				*end = 0;
				cur = xmlNewDocNode(commandInfo->element->doc, params->delimiter_tagname.ns, params->delimiter_tagname.ncname, start);
				*end = tmp;
				APPEND()
			}
			if ((region->beg[0] == region->end[0]) && *start) /* zero-length match */
				start = end + xstrGetOffsetToNextUTF8Char(end);
			else
				start = end;
			prev_boundary = end;
			match = (onig_search(regex, content, content_end, start, content_end, region, 0) != ONIG_MISMATCH);
		}
		if (*prev_boundary || params->keep_empty_tags)
		{
			if (!*prev_boundary)
				prev_boundary = NULL;
			if (!params->unique || (xmlHashAddEntry(unique_hash, prev_boundary, &marker) != -1))
			{
				cur = xmlNewDocNode(commandInfo->element->doc, params->tagname.ns, params->tagname.ncname, prev_boundary);
				APPEND()
			}
		}
	}
	ASSIGN_RESULT(ret, params->repeat, true);
done:
	if (params->tagname.ncname == params->delimiter_tagname.ncname)
		params->delimiter_tagname.ncname = NULL;
	if (unique_hash)
		xmlHashFree(unique_hash, NULL);
	if (regex)
		onig_free(regex);
	if (region)
		onig_region_free(region, 1);
}