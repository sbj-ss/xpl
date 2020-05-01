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
	commandInfo->_private = old_params;
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
	old_params = (xplParamsPtr) commandInfo->_private;
	commandInfo->document->environment = old_params;
	if (commandInfo->element->type & XML_NODE_DELETION_MASK)
		ASSIGN_RESULT(NULL, false, false);
	else {
		if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, false)))
			ASSIGN_RESULT(error, true, true);
		else
			ASSIGN_RESULT(detachContent(commandInfo->element), repeat, true);
	}
}

xplCommand xplSetLocalCommand = { 
	SFINIT(.prologue, xplCmdSetLocalPrologue), 
	SFINIT(.epilogue, xplCmdSetLocalEpilogue),
	SFINIT(.initializer, NULL),
	SFINIT(.finalizer, NULL),
	SFINIT(.flags, XPL_CMD_FLAG_CONTENT_SAFE)
};
