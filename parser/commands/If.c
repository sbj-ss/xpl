#include "commands/If.h"
#include "Core.h"
#include "Messages.h"
#include "Options.h"
#include "Utils.h"

void xplCmdIfPrologue(xplCommandInfoPtr commandInfo)
{
#define TEST_ATTR (BAD_CAST "test")
	xmlChar *test_attr;
	xmlNsPtr xplns;
	xmlNodePtr test_el;

	test_attr = xmlGetNoNsProp(commandInfo->element, TEST_ATTR);
	if (test_attr && *test_attr)
	{
		if (!(xplns = commandInfo->document->root_xpl_ns))
			xplns = xmlSearchNsByHref(commandInfo->element->doc, commandInfo->element, cfgXplNsUri);
		if (!xplns)
		{
			DISPLAY_INTERNAL_ERROR_MESSAGE();
			goto done; /* this shouldn't ever happen */
		}
		test_el = xmlNewDocNode(commandInfo->element->doc, xplns, BAD_CAST "test", test_attr);
		prependList(commandInfo->element->children, test_el);
	}
done:
	if (test_attr) xmlFree(test_attr);
}

void xplCmdIfEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REPEAT_ATTR (BAD_CAST "repeat")
	BOOL repeat;
	xmlNodePtr error;
	
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, FALSE)))
	{
		ASSIGN_RESULT(error, TRUE, TRUE);
		return;
	}
	ASSIGN_RESULT(detachContent(commandInfo->element), repeat, TRUE);
}

xplCommand xplIfCommand = { xplCmdIfPrologue, xplCmdIfEpilogue };

