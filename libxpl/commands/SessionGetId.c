#include <libxpl/xplcore.h>
#include <libxpl/xplsession.h>

void xplCmdSessionGetIdEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

xplCommand xplSessionGetIdCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdSessionGetIdEpilogue,
	.flags = 0,
	.params_stencil = NULL
};

void xplCmdSessionGetIdEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlNodePtr ret;
	xmlChar *id;

	id = xplSessionGetId(commandInfo->document->main->session);
	if (id)
		ret = xmlNewDocText(commandInfo->document->document, id);
	else
		ret = NULL;
	ASSIGN_RESULT(ret, false, true);
}
