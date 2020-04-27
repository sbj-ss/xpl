#include <libxpl/xplcore.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplutils.h>
#include "commands/When.h"

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
	bool repeat;
	xmlNodePtr error;

	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, false)))
		ASSIGN_RESULT(error, true, true);
	else
		ASSIGN_RESULT(detachContent(commandInfo->element), repeat, true);
}

xplCommand xplWhenCommand = { xplCmdWhenPrologue, xplCmdWhenEpilogue };
