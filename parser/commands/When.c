#include "commands/When.h"
#include "Core.h"
#include "Options.h"
#include "Utils.h"

void xplCmdWhenPrologue(xplCommandInfoPtr commandInfo)
{
#define TEST_ATTR (BAD_CAST "test")
	xmlChar *test_attr = NULL;
	xmlNsPtr xpl_ns;
	xmlNodePtr test_el;

	test_attr = xmlGetNoNsProp(commandInfo->element, TEST_ATTR);
	if (test_attr)
	{
		if (!(xpl_ns = commandInfo->document->root_xpl_ns))
			xpl_ns = xmlSearchNsByHref(commandInfo->element->doc, commandInfo->element, cfgXplNsUri);
		test_el = xmlNewDocNode(commandInfo->element->doc, xpl_ns, BAD_CAST "test", NULL);
		test_el->children = xmlNewDocText(commandInfo->element->doc, NULL);
		test_el->children->content = test_attr;
		prependList(commandInfo->element->children, test_el);
	}
}

void xplCmdWhenEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	/* ToDo: :break should be added here and not by :test
	 * motivation: :choose implies single choice but :when without condition will always succeed.
     */
#define REPEAT_ATTR (BAD_CAST "repeat")
	BOOL repeat;
	xmlNodePtr error;

	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, FALSE)))
		ASSIGN_RESULT(error, TRUE, TRUE);
	else
		ASSIGN_RESULT(detachContent(commandInfo->element), repeat, TRUE);
}

xplCommand xplWhenCommand = { xplCmdWhenPrologue, xplCmdWhenEpilogue };
