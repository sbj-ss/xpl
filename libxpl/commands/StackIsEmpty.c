#include <libxpl/xplcore.h>

void xplCmdStackIsEmptyEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

void xplCmdStackIsEmptyEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlNodePtr ret;
	xmlChar *ret_value;

	if (xplDocStackIsEmpty(commandInfo->document))
		ret_value = BAD_CAST "true";
	else
		ret_value = BAD_CAST "false";
	ret = xmlNewDocText(commandInfo->document->document, ret_value);
	ASSIGN_RESULT(ret, false, true);
}

xplCommand xplStackIsEmptyCommand = { NULL, xplCmdStackIsEmptyEpilogue };
