#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplsession.h>
#include <libxpl/xplstring.h>

void xplCmdSessionIsPresentEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

void xplCmdSessionIsPresentEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR (BAD_CAST "name")
#define THREADLOCAL_ATTR (BAD_CAST "threadlocal")
	xmlChar *name_attr;
	bool ret_value;
	bool threadlocal;
	xmlNodePtr ret, error;

	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	if (!name_attr)
		ret_value = (commandInfo->document->main->session 
		&& xplSessionIsValid(commandInfo->document->main->session));
	else {
		if ((error = xplDecodeCmdBoolParam(commandInfo->element, THREADLOCAL_ATTR, &threadlocal, false)))
		{
			ASSIGN_RESULT(error, true, true);
			goto done;
		}
		if (threadlocal)
			name_attr = xstrAppendThreadIdToString(name_attr, xprGetCurrentThreadId());
		ret_value = (commandInfo->document->main->session 
			&& xplSessionIsValid(commandInfo->document->main->session) 
			&& xplSessionGetObject(commandInfo->document->main->session, name_attr));
	}
	ret = xmlNewDocText(commandInfo->document->document, ret_value? BAD_CAST "true": BAD_CAST "false");
	ASSIGN_RESULT(ret, false, true);	
done:
	if (name_attr) XPL_FREE(name_attr);
}

xplCommand xplSessionIsPresentCommand = { NULL, xplCmdSessionIsPresentEpilogue };
