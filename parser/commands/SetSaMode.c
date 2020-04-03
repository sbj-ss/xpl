#include "commands/SetSaMode.h"
#include "Core.h"
#include "Messages.h"
#include "Session.h"
#include "Utils.h"

void xplCmdSetSaModePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdSetSaModeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define ENABLE_ATTR (BAD_CAST "enable")
#define PASSWORD_ATTR (BAD_CAST "password")

	xmlChar *password_attr = NULL;
	BOOL enable;
	xmlNodePtr error;

	if ((error = xplDecodeCmdBoolParam(commandInfo->element, ENABLE_ATTR, &enable, TRUE)))
	{
		ASSIGN_RESULT(error, TRUE, TRUE);
		return;
	}
	password_attr = xmlGetNoNsProp(commandInfo->element, PASSWORD_ATTR);

	if (!commandInfo->document->session)
		commandInfo->document->session = xplSessionCreateWithAutoId();

	if (xplSessionSetSaMode(commandInfo->document->session, enable, password_attr))
		ASSIGN_RESULT(NULL, FALSE, TRUE);
	else
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "the password is missing or incorrect"), TRUE, TRUE);
	if (password_attr)
		xmlFree(password_attr);
}

xplCommand xplSetSaModeCommand = { xplCmdSetSaModePrologue, xplCmdSetSaModeEpilogue };
