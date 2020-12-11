#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>

void xplCmdWithPrologue(xplCommandInfoPtr commandInfo);
void xplCmdWithEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef enum _WithMode 
{
	WITH_MODE_UNKNOWN = -1,
	WITH_MODE_REPLACE = 0,
	WITH_MODE_APPEND
} WithMode;

static WithMode _withModeFromString(xmlChar *s)
{
	if (!s || !*s)
		return WITH_MODE_REPLACE;
	if (!xmlStrcasecmp(s, BAD_CAST "replace"))
		return WITH_MODE_REPLACE;
	if (!xmlStrcasecmp(s, BAD_CAST "append"))
		return WITH_MODE_APPEND;
	return WITH_MODE_UNKNOWN;
}

typedef enum _NaosCheckResult
{
	NAOS_CHECK_RESULT_OK,
	NAOS_CHECK_RESULT_IS_ANCESTOR,
	NAOS_CHECK_RESULT_ANCESTOR_DELETED
} NaosCheckResult;

static bool _checkNAOS(xmlNodePtr cmd, xmlNodePtr test)
{
	while (cmd != (xmlNodePtr) cmd->doc)
	{
		if (cmd == test)
			return NAOS_CHECK_RESULT_IS_ANCESTOR;
		if ((int) cmd->type & XPL_NODE_DELETION_MASK)
			return NAOS_CHECK_RESULT_ANCESTOR_DELETED;
		cmd = cmd->parent;
	}
	return NAOS_CHECK_RESULT_OK;
}

void xplCmdWithPrologue(xplCommandInfoPtr commandInfo)
{
#define SELECT_ATTR (BAD_CAST "select")
#define ID_ATTR (BAD_CAST "id")
#define MODE_ATTR (BAD_CAST "mode")
	xmlChar *select_attr = NULL;
	xmlChar *id_attr = NULL;
	xmlChar *mode_attr = NULL;
	xmlXPathObjectPtr sel = NULL;
	WithMode mode;
	xmlNodePtr cur, repl;
	size_t i;
	xplResult temp_result;
	int naos_check_result = NAOS_CHECK_RESULT_OK;
	
	select_attr = xmlGetNoNsProp(commandInfo->element, SELECT_ATTR);
	if (!select_attr)
	{
		commandInfo->prologue_error = xplCreateErrorNode(commandInfo->element, BAD_CAST "missing select attribute");
		goto done;
	}
	id_attr = xmlGetNoNsProp(commandInfo->element, ID_ATTR);
	mode_attr = xmlGetNoNsProp(commandInfo->element, MODE_ATTR);
	mode = _withModeFromString(mode_attr);
	if (mode == WITH_MODE_UNKNOWN)
	{
		commandInfo->prologue_error = xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid mode attribute \"%s\"", mode_attr);
		goto done;
	}

	commandInfo->document->iterator_spinlock++;
	sel = xplSelectNodes(commandInfo, commandInfo->element, select_attr);
	if (sel)
	{
		if (sel->type == XPATH_NODESET)
		{
			if (sel->nodesetval)
			{
				for (i = 0; i < (size_t) sel->nodesetval->nodeNr; i++)
				{
					if (commandInfo->document->fatal_content)
						break;
					cur = sel->nodesetval->nodeTab[i];
					if (((int) cur->type) & XPL_NODE_DELETION_MASK)
					{
						if (cfgWarnOnDeletedNodeReference)
						{
							if (cur->ns)
								xplDisplayMessage(xplMsgWarning, 
									BAD_CAST "node \"%s:%s\" (line %d) post-mortem access attempt (file \"%s\", line %d)", 
									cur->ns->prefix, 
									cur->name, 
									cur->line,
									commandInfo->document->document->URL, 
									commandInfo->element->line);
							else
								xplDisplayMessage(xplMsgWarning, 
									BAD_CAST "node \"%s\" (line %d) post-mortem access attempt (file \"%s\", line %d)", 
									cur->name, 
									cur->line,
									commandInfo->document->document->URL, 
									commandInfo->element->line);
						}
						continue;
					}
					if ((cur->type != XML_ELEMENT_NODE))
						continue;
					naos_check_result = cfgFoolproofDestructiveCommands? _checkNAOS(commandInfo->element, cur): NAOS_CHECK_RESULT_OK;
					if (naos_check_result == NAOS_CHECK_RESULT_IS_ANCESTOR)
					{
						if (cfgWarnOnAncestorModificationAttempt)
						{
							if (cur->ns)
								xplDisplayMessage(xplMsgWarning, BAD_CAST "ancestor/self node \"%s:%s\" modification attempt denied (file \"%s\", line %d)", 
								cur->ns->prefix, cur->name, commandInfo->document->document->URL, commandInfo->element->line);
							else
								xplDisplayMessage(xplMsgWarning, BAD_CAST "ancestor/self node \"%s\" modification attempt denied (file \"%s\", line %d)", 
								cur->name, commandInfo->document->document->URL, commandInfo->element->line);
						}
						continue;
					} else if (naos_check_result == NAOS_CHECK_RESULT_ANCESTOR_DELETED)
						break;
					repl = xplReplaceContentEntries(commandInfo->document, id_attr, cur, commandInfo->element->children); 
					if (mode == WITH_MODE_REPLACE)
					{
						xplDocDeferNodeListDeletion(commandInfo->document, xplDetachContent(cur));
						xplSetChildren(cur, repl);
						xplNodeApply(commandInfo->document, cur, &temp_result);
						if ((int) cur->type & XPL_NODE_DELETION_MASK)
							xmlUnlinkNode(cur);
						else {
							xplReplaceWithList(cur, cur->children);
							cur->children = cur->last = NULL;
						}
						xplDocDeferNodeDeletion(commandInfo->document, cur);
					} else if (mode == WITH_MODE_APPEND) {
						xplAppendChildren(cur, repl);
						xplNodeListApply(commandInfo->document, repl, &temp_result);
					} else
						DISPLAY_INTERNAL_ERROR_MESSAGE();
				} /* for */
			} /* if (sel->nodesetval) */
		} else {
			commandInfo->prologue_error = xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath (%s) evaluated to non-nodeset value", select_attr);
			goto done;
		}
	} else {
		commandInfo->prologue_error = xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid select XPath expression (%s)", select_attr);
		goto done;
	}
done:
	commandInfo->document->iterator_spinlock--;
	if (select_attr) XPL_FREE(select_attr);
	if (id_attr) XPL_FREE(id_attr);
	if (mode_attr) XPL_FREE(mode_attr);
	if (sel)
	{
		if (sel->nodesetval)
			sel->nodesetval->nodeNr = 0;
		xmlXPathFreeObject(sel);
	}
	if (!((int) commandInfo->element->type & XPL_NODE_DELETION_MASK))
		xmlFreeNodeList(xplDetachContent(commandInfo->element));
}

void xplCmdWithEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	ASSIGN_RESULT(commandInfo->prologue_error, (commandInfo->prologue_error)? true: false, true);
}

xplCommand xplWithCommand = 
{ 
	.prologue = xplCmdWithPrologue,
	.epilogue = xplCmdWithEpilogue,
	.initializer = NULL,
	.finalizer = NULL,
	.flags = 0
};