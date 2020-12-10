#include <string.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>

void xplCmdStringerEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

void xplCmdStringerEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define SELECT_ATTR (BAD_CAST "select")
#define DELIMITER_ATTR (BAD_CAST "delimiter")
#define START_DELIMITER_ATTR (BAD_CAST "startdelimiter")
#define END_DELIMITER_ATTR (BAD_CAST "enddelimiter")
#define UNIQUE_ATTR (BAD_CAST "unique")
#define KEEPEMPTYTAGS_ATTR (BAD_CAST "keepemptytags")
/* Lucifer compatibility */
# define DELIMETER_ATTR (BAD_CAST "delimeter")
# define START_DELIMETER_ATTR (BAD_CAST "startdelimeter")
# define END_DELIMETER_ATTR (BAD_CAST "enddelimeter")

	xmlChar *select_attr = NULL;
	xmlChar *delimiter_attr = NULL;
	xmlChar *start_delimiter_attr = NULL;
	xmlChar *end_delimiter_attr = NULL;
	bool unique;
	bool keep_empty_tags;
	xmlXPathObjectPtr sel = NULL;
	xmlNodePtr cur, ret = NULL, error;
	xmlHashTablePtr unique_hash = NULL;
	size_t i = 0;
	xmlChar *cur_str, *ret_str = NULL, *ret_str_pos = NULL, *tail_pos = NULL;
	size_t delim_len = 0, sd_len = 0, ed_len = 0, delims_len, cur_len, ret_len = 0;

	if ((error = xplDecodeCmdBoolParam(commandInfo->element, UNIQUE_ATTR, &unique, false)))
	{
		ASSIGN_RESULT(error, true, true);
		return;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, KEEPEMPTYTAGS_ATTR, &keep_empty_tags, false)))
	{
		ASSIGN_RESULT(error, true, true);
		return;
	}
	select_attr = xmlGetNoNsProp(commandInfo->element, SELECT_ATTR);
	delimiter_attr = xmlGetNoNsProp(commandInfo->element, DELIMITER_ATTR);
	start_delimiter_attr = xmlGetNoNsProp(commandInfo->element, START_DELIMITER_ATTR);
	end_delimiter_attr = xmlGetNoNsProp(commandInfo->element, END_DELIMITER_ATTR);
	if (cfgLuciferCompat)
	{
		if (!delimiter_attr)
			delimiter_attr = xmlGetNoNsProp(commandInfo->element, DELIMETER_ATTR);
		if (!start_delimiter_attr)
			start_delimiter_attr = xmlGetNoNsProp(commandInfo->element, START_DELIMETER_ATTR);
		if (!end_delimiter_attr)
			end_delimiter_attr = xmlGetNoNsProp(commandInfo->element, END_DELIMETER_ATTR);
	}
	if (delimiter_attr)
		delim_len = xmlStrlen(delimiter_attr);
	if (start_delimiter_attr)
		sd_len = xmlStrlen(start_delimiter_attr);
	if (end_delimiter_attr)
		ed_len = xmlStrlen(end_delimiter_attr);
	delims_len = delim_len + sd_len + ed_len;
	if (unique)
		unique_hash = xmlHashCreate(16);

	if (select_attr)
	{
		sel = xplSelectNodes(commandInfo, commandInfo->element, select_attr);
		if (sel)
		{
			if (sel->type == XPATH_NODESET)
			{
				if (!sel->nodesetval || !sel->nodesetval->nodeNr) /* no nodes */
				{
					ASSIGN_RESULT(NULL, false, true);
					goto done;
				}
				cur = sel->nodesetval->nodeTab[0];
			} else {
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath expression \"%s\" evaluated to non-nodeset value", select_attr), true, true);
				goto done;
			}
		} else {
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid select XPath expression (%s)", select_attr), true, true);
			goto done;
		}
	} else
		cur = commandInfo->element->children;

	while (cur) /* unified cycle */
	{
		// filter by node type
		if ((cur->type == XML_ELEMENT_NODE) || (cur->type == XML_ATTRIBUTE_NODE))
		{
			cur_str = xmlNodeListGetString(cur->doc, cur->children, 1);
			if ((cur_str && *cur_str) || keep_empty_tags)
			{
				if (!unique || (xmlHashAddEntry(unique_hash, cur_str, (void*) 883) != - 1)) 
				{
					cur_len = xmlStrlen(cur_str);
					ret_str_pos = (xmlChar*) ret_len; // remember previous offset
					ret_len += cur_len + delims_len;
					ret_str = (xmlChar*) XPL_REALLOC(ret_str, ret_len + 1);
					ret_str_pos += (size_t) ret_str; // start of current part
					if (start_delimiter_attr)
					{
						memcpy(ret_str_pos, start_delimiter_attr, sd_len);
						ret_str_pos += sd_len;
					}
					memcpy(ret_str_pos, cur_str, cur_len);
					ret_str_pos += cur_len;
					if (end_delimiter_attr)
					{
						memcpy(ret_str_pos, end_delimiter_attr, ed_len);
						ret_str_pos += ed_len;
					}
					if (delimiter_attr)
					{
						memcpy(ret_str_pos, delimiter_attr, delim_len);
						tail_pos = ret_str_pos; // mark the point to clear the trailing delimiter
						ret_str_pos += delim_len;
					}
					*ret_str_pos = 0;
				}
			}
			if (cur_str) XPL_FREE(cur_str);
		}
		// advance to next node
		if (sel)
		{
			if (++i >= (size_t) sel->nodesetval->nodeNr)
				break;
			cur = sel->nodesetval->nodeTab[i];
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
done:
	if (select_attr)
		XPL_FREE(select_attr);
	if (delimiter_attr)
		XPL_FREE(delimiter_attr);
	if (start_delimiter_attr)
		XPL_FREE(start_delimiter_attr);
	if (end_delimiter_attr)
		XPL_FREE(end_delimiter_attr);
	if (sel)
		xmlXPathFreeObject(sel);
	if (unique_hash)
		xmlHashFree(unique_hash, NULL);
}

xplCommand xplStringerCommand = { NULL, xplCmdStringerEpilogue };
