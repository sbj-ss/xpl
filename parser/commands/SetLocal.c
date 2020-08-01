#include <libxpl/xplcore.h>
#include <libxpl/xplparams.h>
#include <libxpl/xpltree.h>
#include "commands/SetLocal.h"

void xplCmdSetLocalPrologue(xplCommandInfoPtr commandInfo)
{
	xplParamsPtr old_params = commandInfo->document->environment;
	xplParamsPtr tmp_params = NULL;
	if (old_params)
	{
		tmp_params = xplParamsCopy(old_params);
		commandInfo->document->environment = tmp_params;
	}
	commandInfo->prologue_state = old_params;
}

void xplCmdSetLocalEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REPEAT_ATTR BAD_CAST("repeat")
	xplParamsPtr old_params, tmp_params;
	bool repeat;
	xmlNodePtr error;
	
	tmp_params = commandInfo->document->environment;
	if (tmp_params)
		xplParamsFree(tmp_params);
	old_params = (xplParamsPtr) commandInfo->prologue_state;
	commandInfo->document->environment = old_params;
	if (commandInfo->element->type & XPL_NODE_DELETION_MASK)
		ASSIGN_RESULT(NULL, false, false);
	else {
		if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, false)))
			ASSIGN_RESULT(error, true, true);
		else
			ASSIGN_RESULT(xplDetachContent(commandInfo->element), repeat, true);
	}
}

xplCommand xplSetLocalCommand = { 
	.prologue = xplCmdSetLocalPrologue, 
	.epilogue = xplCmdSetLocalEpilogue,
	.initializer = NULL,
	.finalizer = NULL,
	.flags = XPL_CMD_FLAG_CONTENT_SAFE
};
