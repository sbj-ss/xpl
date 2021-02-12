#include <string.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>

void xplCmdStringerEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdStringerParams
{
	xmlXPathObjectPtr select;
	xmlChar *delimiter;
	xmlChar *start_delimiter;
	xmlChar *end_delimiter;
	bool unique;
	bool keep_empty_tags;
} xplCmdStringerParams, *xplCmdStringerParamsPtr;

static const xplCmdStringerParams params_stencil =
{
	.select = NULL,
	.delimiter = NULL,
	.start_delimiter = NULL,
	.end_delimiter = NULL,
	.unique = false,
	.keep_empty_tags = false
};

static xmlChar* delimiter_aliases[] = { BAD_CAST "delimeter", NULL };
static xmlChar* start_delimiter_aliases[] = { BAD_CAST "startdelimeter", NULL };
static xmlChar* end_delimiter_aliases[] = { BAD_CAST "enddelimeter", NULL };

xplCommand xplStringerCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdStringerEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdStringerParams),
	.parameters = {
		{
			.name = BAD_CAST "select",
			.type = XPL_CMD_PARAM_TYPE_XPATH,
			.extra.xpath_type = XPL_CMD_PARAM_XPATH_TYPE_NODESET,
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
			.value_stencil = &params_stencil.end_delimiter
		}, {
			.name = BAD_CAST "unique",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.unique
		}, {
			.name = BAD_CAST "keepemptytags",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.keep_empty_tags
		}, {
			.name = NULL
		}
	}
};

void xplCmdStringerEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdStringerParamsPtr params = (xplCmdStringerParamsPtr) commandInfo->params;
	xmlNodePtr cur, ret = NULL;
	xmlHashTablePtr unique_hash = NULL;
	size_t i = 0;
	xmlChar *cur_str, *ret_str = NULL, *ret_str_pos = NULL, *tail_pos = NULL;
	size_t delim_len, sd_len, ed_len, delims_len, cur_len, ret_len = 0;

	delim_len = params->delimiter? xmlStrlen(params->delimiter): 0;
	sd_len = params->start_delimiter? xmlStrlen(params->start_delimiter): 0;
	ed_len = params->end_delimiter? xmlStrlen(params->end_delimiter): 0;
	delims_len = delim_len + sd_len + ed_len;
	if (params->unique)
		unique_hash = xmlHashCreate(16);

	if (params->select)
	{
		if (!params->select->nodesetval)
		{
			ASSIGN_RESULT(NULL, false, true);
			return;
		}
		cur = params->select->nodesetval->nodeTab[0];
	} else
		cur = commandInfo->element->children;

	while (cur) /* unified cycle */
	{
		// filter by node type
		if ((cur->type == XML_ELEMENT_NODE) || (cur->type == XML_ATTRIBUTE_NODE) || (cur->type == XML_TEXT_NODE) || (cur->type == XML_CDATA_SECTION_NODE))
		{
			if (cur->type == XML_ELEMENT_NODE || cur->type == XML_ATTRIBUTE_NODE)
				cur_str = xmlNodeListGetString(cur->doc, cur->children, 1);
			else
				cur_str = cur->content;
			if ((cur_str && *cur_str) || params->keep_empty_tags)
			{
				if (!params->unique || (xmlHashAddEntry(unique_hash, cur_str, (void*) 883) != - 1))
				{
					cur_len = xmlStrlen(cur_str);
					ret_str_pos = (xmlChar*) ret_len; // remember previous offset
					ret_len += cur_len + delims_len;
					ret_str = (xmlChar*) XPL_REALLOC(ret_str, ret_len + 1);
					ret_str_pos += (size_t) ret_str; // start of current part
					if (params->start_delimiter)
					{
						memcpy(ret_str_pos, params->start_delimiter, sd_len);
						ret_str_pos += sd_len;
					}
					if (cur_str)
						memcpy(ret_str_pos, cur_str, cur_len);
					ret_str_pos += cur_len;
					if (params->end_delimiter)
					{
						memcpy(ret_str_pos, params->end_delimiter, ed_len);
						ret_str_pos += ed_len;
					}
					if (params->delimiter)
					{
						memcpy(ret_str_pos, params->delimiter, delim_len);
						tail_pos = ret_str_pos; // mark the point to clear the trailing delimiter
						ret_str_pos += delim_len;
					}
					*ret_str_pos = 0;
				}
			}
			if (cur_str && cur_str != cur->content)
				XPL_FREE(cur_str);
		} else if (cfgWarnOnInvalidNodeType)
			xplDisplayWarning(commandInfo->element, BAD_CAST "only elements, attributes and text/cdata can be used for text extraction");
		// advance to next node
		if (params->select)
		{
			if (++i >= (size_t) params->select->nodesetval->nodeNr)
				break;
			cur = params->select->nodesetval->nodeTab[i];
		} else
			cur = cur->next;
	}
	if (tail_pos) // delete last delimiter
		*tail_pos = 0;
	if (ret_str)
	{
		ret = xmlNewDocText(commandInfo->element->doc, NULL);
		ret->content = ret_str;
	}
	ASSIGN_RESULT(ret, false, true);
	if (unique_hash)
		xmlHashFree(unique_hash, NULL);
}
