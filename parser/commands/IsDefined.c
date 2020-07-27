#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>
#include "commands/IsDefined.h"

void xplCmdIsDefinedPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdIsDefinedEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR (BAD_CAST "name")
#define AT_ATTR (BAD_CAST "at")
	xmlChar *name_attr = NULL, *tagname;
	xmlChar *at_attr = NULL;
	xmlNsPtr ns;
	xmlXPathObjectPtr sel = NULL;
	xmlHashTablePtr macro_table;
	xplMacroPtr macro = NULL;
	const xmlChar* value;
	xmlNodePtr ret;
	size_t i;

	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	if (!name_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "missing name attribute"), true, true);
		return;
	}
	EXTRACT_NS_AND_TAGNAME(name_attr, ns, tagname, commandInfo->element);
	at_attr = xmlGetNoNsProp(commandInfo->element, AT_ATTR);
	if (at_attr)
	{
		sel = xplSelectNodes(commandInfo->document, commandInfo->element, at_attr);
		if (sel)
		{
			if (sel->type == XPATH_NODESET)
			{
				if (sel->nodesetval)
				{
					for (i = 0; i < (size_t) sel->nodesetval->nodeNr; i++)
					{
						macro_table = (xmlHashTablePtr) sel->nodesetval->nodeTab[i]->_private;
						if (macro_table)
							macro = (xplMacroPtr) xmlHashLookup2(macro_table, tagname, ns? ns->href: NULL);
						if (macro && !macro->disabled_spin) 
							break;
					}
				}
			} else {
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "at XPath expression \"%s\" evaluated to non-nodeset value", at_attr), true, true);
				goto done;
			}
		} else {
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "invalid at XPath expression \"%s\"", at_attr), true, true);
			goto done;
		}
	} else
		macro = xplMacroLookup(commandInfo->element->parent, ns? ns->href: NULL, tagname);
	if (macro && !macro->disabled_spin)
		value = BAD_CAST "true";
	else
		value = BAD_CAST "false";
	ret = xmlNewDocText(commandInfo->document->document, value);
	ASSIGN_RESULT(ret, false, true);
done:
	if (name_attr) XPL_FREE(name_attr);
	if (at_attr) XPL_FREE(at_attr);
	if (sel) xmlXPathFreeObject(sel);
}

xplCommand xplIsDefinedCommand = { xplCmdIsDefinedPrologue, xplCmdIsDefinedEpilogue }; 
