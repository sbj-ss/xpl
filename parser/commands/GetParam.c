#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplparams.h>
#include <libxpl/xpltree.h>
#include "commands/GetParam.h"

void xplCmdGetParamPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdGetParamEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define EXPECT_ATTR (BAD_CAST "expect")
#define RESPONSE_TAG_NAME_ATTR (BAD_CAST "responsetagname")
#define SHOW_TAGS_ATTR (BAD_CAST "showtags")
#define NAME_ATTR (BAD_CAST "name")
#define DELIM_ATTR (BAD_CAST "delim")
#define DELIMITER_ATTR (BAD_CAST "delimiter")
#define UNIQUE_ATTR (BAD_CAST "unique")
#define TYPE_ATTR (BAD_CAST "type")
#define REPEAT_ATTR (BAD_CAST "repeat")
#define DEFAULT_ATTR (BAD_CAST "default")

	xmlChar *name_attr = NULL; 
	xmlChar *expect_attr = NULL;
	xmlChar *response_tag_name_attr = NULL;
	xmlChar *delim_attr = NULL;
	xmlChar *type_attr = NULL;
	xmlChar *default_attr = NULL;
	xmlChar *txt, *node_name;
	xmlNsPtr ns;
	xplExpectType expect;
	xplParamValuesPtr values;
	xmlNodePtr ret = NULL, error;
	bool repeat;
	bool unique;
	bool show_tags;
	bool free_needed = false;
	int type_mask = XPL_PARAM_TYPE_USERDATA; /* backward compatibility */

	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	expect_attr = xmlGetNoNsProp(commandInfo->element, EXPECT_ATTR);
	response_tag_name_attr = xmlGetNoNsProp(commandInfo->element, RESPONSE_TAG_NAME_ATTR);
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, SHOW_TAGS_ATTR, &show_tags, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, UNIQUE_ATTR, &unique, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, true)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if (!(delim_attr = xmlGetNoNsProp(commandInfo->element, DELIM_ATTR)))
		delim_attr = xmlGetNoNsProp(commandInfo->element, DELIMITER_ATTR);
	type_attr = xmlGetNoNsProp(commandInfo->element, TYPE_ATTR);
	default_attr = xmlGetNoNsProp(commandInfo->element, DEFAULT_ATTR);

	expect = xplExpectTypeFromString(expect_attr);
	if (cfgWarnOnNoExpectParam) 
	{
		switch(expect)
		{
			case XPL_EXPECT_UNDEFINED:
				xplDisplayMessage(xplMsgWarning, BAD_CAST "no expect attribute in get-param command (file \"%s\", line %d)",
					commandInfo->element->doc->URL, commandInfo->element->line);
				break;
			case XPL_EXPECT_UNKNOWN:
				xplDisplayMessage(xplMsgWarning, BAD_CAST "unknown expect attribute \"%s\" in get-param command (file \"%s\", line %d). Defaulting to \"number\" as most restrictive",
					expect_attr, commandInfo->element->doc->URL, commandInfo->element->line);
				expect = XPL_EXPECT_NUMBER;
				break;
			default:
				break;
		}
	} else if (expect == XPL_EXPECT_UNKNOWN) /* can't clean values here */
		expect = XPL_EXPECT_ANY;

	if (type_attr)
	{
		type_mask = xplParamTypeMaskFromString(type_attr);
		if (type_mask == -1)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "The type mask \"%s\" is invalid", type_attr), true, true);
			goto done;
		}
	}
	if (name_attr) 
	{
		/* specified parameter */
		repeat = repeat && (show_tags || response_tag_name_attr);
		values = xplParamGet(commandInfo->document->environment, name_attr);
		if (values && !(values->type & type_mask))
			values = NULL; /* skip unwanted */
		else if (!values && default_attr) {
			values = xplParamValuesCreate();
			free_needed = true;
			xplParamValuesAdd(values, default_attr, XPL_PARAM_TYPE_USERDATA);
		}
		if (!values)
			; /* nothing to do */
		else if (response_tag_name_attr) {
			EXTRACT_NS_AND_TAGNAME(response_tag_name_attr, ns, node_name, commandInfo->element);
			ret = xplParamValuesToList(values, unique, expect, ns, node_name, commandInfo->element);
		} else if (show_tags)
			ret = xplParamValuesToList(values, unique, expect, NULL, name_attr, commandInfo->element);
		else {
			txt = xplParamValuesToString(values, unique, delim_attr, expect);
			if (txt)
			{
				ret = xmlNewDocText(commandInfo->document->document, NULL);
				ret->content = txt;
			}
		}
	} else {
		/* all params */
		if (show_tags)
		{
			node_name = NULL;
			ns = NULL;
		} else if (response_tag_name_attr) {
			EXTRACT_NS_AND_TAGNAME(response_tag_name_attr, ns, node_name, commandInfo->element);
		} else {
			ns = NULL;
			node_name = BAD_CAST "param";
		}
		ret = xplParamsToList(commandInfo->document->environment, unique, expect, ns, node_name, commandInfo->element, type_mask);
		// ToDo: downshiftNodeListNsDef?..
	} 
	ASSIGN_RESULT(ret, repeat, true);
done:
	if (name_attr)
		XPL_FREE(name_attr);
	if (expect_attr)
		XPL_FREE(expect_attr);
	if (response_tag_name_attr)
		XPL_FREE(response_tag_name_attr);
	if (delim_attr)
		XPL_FREE(delim_attr);
	if (type_attr)
		XPL_FREE(type_attr);
	if (free_needed)
		xplParamValuesFree(values);
	else if (default_attr)
		XPL_FREE(default_attr);
}

xplCommand xplGetParamCommand = { xplCmdGetParamPrologue, xplCmdGetParamEpilogue };
