#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include "commands/FileExists.h"

void xplCmdFileExistsPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdFileExistsEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
/* TODO check R/W access */
#define FILE_ATTR (BAD_CAST "file")
#define ABS_PATH_ATTR (BAD_CAST "abspath")

	xmlChar *file_attr = NULL;
	xmlChar *filename = NULL;
	xmlChar *value;
	xmlNodePtr ret, error;
	bool abs_path;

	file_attr = xmlGetNoNsProp(commandInfo->element, FILE_ATTR);
	if (!file_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "file attribute not found"), true, true);
		return;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, ABS_PATH_ATTR, &abs_path, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if (abs_path)
		filename = BAD_CAST XPL_STRDUP(file_attr);
	else
		filename = xplFullFilename(file_attr, commandInfo->document->app_path);

	if (xprCheckFilePresence(filename))
		value = BAD_CAST "true";
	else
		value = BAD_CAST "false";
	ret = xmlNewDocText(commandInfo->document->document, value);
	ASSIGN_RESULT(ret, false, true);
done:
	if (file_attr) XPL_FREE(file_attr);
	if (filename) XPL_FREE(filename);
}

xplCommand xplFileExistsCommand = { xplCmdFileExistsPrologue, xplCmdFileExistsEpilogue };

