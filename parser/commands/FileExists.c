#include "commands/FileExists.h"
#include "Core.h"
#include "Messages.h"
#include "Utils.h"

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
	BOOL abs_path;

	file_attr = xmlGetNoNsProp(commandInfo->element, FILE_ATTR);
	if (!file_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "file attribute not found"), TRUE, TRUE);
		return;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, ABS_PATH_ATTR, &abs_path, FALSE)))
	{
		ASSIGN_RESULT(error, TRUE, TRUE);
		goto done;
	}
	if (abs_path)
		filename = xmlStrdup(file_attr);
	else
		filename = xplFullFilename(file_attr, commandInfo->document->app_path);

	if (xprCheckFilePresence(filename))
		value = BAD_CAST "true";
	else
		value = BAD_CAST "false";
	ret = xmlNewDocText(commandInfo->document->document, value);
	ASSIGN_RESULT(ret, FALSE, TRUE);
done:
	if (file_attr) xmlFree(file_attr);
	if (filename) xmlFree(filename);
}

xplCommand xplFileExistsCommand = { xplCmdFileExistsPrologue, xplCmdFileExistsEpilogue };

