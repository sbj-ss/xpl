#include <libxpl/xplcommand.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>
#include <string.h>
#include <oniguruma.h>

void xplCmdRegexMatchEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdRegexMatchParams
{
	xmlChar *regex;
	bool full_string;
	bool ignore_case;
} xplCmdRegexMatchParams, *xplCmdRegexMatchParamsPtr;

static const xplCmdRegexMatchParams params_stencil =
{
	.regex = NULL,
	.full_string = false,
	.ignore_case = false
};

xplCommand xplRegexMatchCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdRegexMatchEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE | XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdRegexMatchParams),
	.parameters = {
		{
			.name = BAD_CAST "regex",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.required = true,
			.value_stencil = &params_stencil.regex
		}, {
			.name = BAD_CAST "fullstring",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.full_string
		}, {
			.name = BAD_CAST "ignorecase",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.ignore_case
		}, {
			.name = NULL
		}
	}
};

void xplCmdRegexMatchEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdRegexMatchParamsPtr params = (xplCmdRegexMatchParamsPtr) commandInfo->params;
	OnigOptionType options = 0;
	xmlChar *content_end;
	regex_t *regex = NULL;
	OnigErrorInfo err_info;
	OnigRegion *region = NULL;
	xmlChar err_str[ONIG_MAX_ERROR_MESSAGE_LEN+1];
	bool match;
	xmlNodePtr ret;
	int ret_code;

	if (params->ignore_case)
		ONIG_OPTION_ON(options, ONIG_OPTION_IGNORECASE);
	if ((ret_code = onig_new(&regex, params->regex, params->regex + xmlStrlen(params->regex),
		options, ONIG_ENCODING_UTF8, ONIG_SYNTAX_PERL_NG, &err_info)) != ONIG_NORMAL)
	{
		if (!onig_error_code_to_str(err_str, ret_code))
			strcpy((char*) err_str, "unknown error");
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "Oniguruma error: '%s'", err_str), true, true);
		goto done;
	}
	region = onig_region_new();
	if (!region)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "Oniguruma cannot create search region"), true, true);
		goto done;
	}
	content_end = commandInfo->content + xmlStrlen(commandInfo->content);
	if (params->full_string)
		match = (onig_match(regex, commandInfo->content, content_end, commandInfo->content, region, 0) != ONIG_MISMATCH);
	else
		match = (onig_search(regex, commandInfo->content, content_end, commandInfo->content, content_end, region, 0) != ONIG_MISMATCH);
	ret = xmlNewDocText(commandInfo->element->doc, NULL);
	if (match)
		ret->content = BAD_CAST XPL_STRDUP("true");
	else
		ret->content = BAD_CAST XPL_STRDUP("false");
	ASSIGN_RESULT(ret, false, true);
done:
	if (regex)
		onig_free(regex);
	if (region)
		onig_region_free(region, 1);
}
