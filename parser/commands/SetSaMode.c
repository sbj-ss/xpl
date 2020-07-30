#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplsession.h>
#include "commands/SetSaMode.h"

void xplCmdSetSaModePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdSetSaModeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define ENABLE_ATTR (BAD_CAST "enable")
#define PASSWORD_ATTR (BAD_CAST "password")

	xmlChar *password_attr = NULL;
	bool enable;
	xmlNodePtr error;

	if ((error = xplDecodeCmdBoolParam(commandInfo->element, ENABLE_ATTR, &enable, true)))
	{
		ASSIGN_RESULT(error, true, true);
		return;
	}
	password_attr = xmlGetNoNsProp(commandInfo->element, PASSWORD_ATTR);

	if (!commandInfo->document->session)
		commandInfo->document->session = xplSessionCreateWithAutoId();

	if (xplSessionSetSaMode(commandInfo->document->session, enable, password_attr))
		ASSIGN_RESULT(NULL, false, true);
	else
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "the password is missing or incorrect"), true, true);
	if (password_attr)
		XPL_FREE(password_attr);
}

xplCommand xplSetSaModeCommand = { xplCmdSetSaModePrologue, xplCmdSetSaModeEpilogue };
