#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>

void xplCmdWithPrologue(xplCommandInfoPtr commandInfo);
void xplCmdWithEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef enum _WithMode 
{
	WITH_MODE_REPLACE,
	WITH_MODE_APPEND
} WithMode;

static xplCmdParamDictValue mode_dict[] =
{
	{ .name = BAD_CAST "replace", .value = WITH_MODE_REPLACE },
	{ .name = BAD_CAST "append", .value = WITH_MODE_APPEND },
	{ .name = NULL }
};

typedef struct _xplCmdWithParams
{
	xmlXPathObjectPtr select;
	xmlChar *id;
	WithMode mode;
} xplCmdWithParams, *xplCmdWithParamsPtr;

static const xplCmdWithParams params_stencil =
{
	.select = NULL,
	.id = NULL,
	.mode = WITH_MODE_REPLACE
};

xplCommand xplWithCommand =
{
	.prologue = xplCmdWithPrologue,
	.epilogue = xplCmdWithEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_PROLOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdWithParams),
	.parameters = {
		{
			.name = BAD_CAST "select",
			.type = XPL_CMD_PARAM_TYPE_XPATH,
			.extra.xpath_type = XPL_CMD_PARAM_XPATH_TYPE_NODESET,
			.required = true,
			.value_stencil = &params_stencil.select
		}, {
			.name = BAD_CAST "id",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.id
		}, {
			.name = BAD_CAST "mode",
			.type = XPL_CMD_PARAM_TYPE_DICT,
			.extra.dict_values = mode_dict,
			.value_stencil = &params_stencil.mode
		}, {
			.name = NULL
		}
	}
};

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
	xplCmdWithParamsPtr params = (xplCmdWithParamsPtr) commandInfo->params;
	xmlNodePtr cur, repl;
	size_t i;
	xplResult temp_result;
	int naos_check_result = NAOS_CHECK_RESULT_OK;
	
	commandInfo->document->iterator_spin++;
	if (params->select->nodesetval)
	{
		for (i = 0; i < (size_t) params->select->nodesetval->nodeNr; i++)
		{
			if (commandInfo->document->fatal_content)
				break;
			cur = params->select->nodesetval->nodeTab[i];
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
				continue; // TODO warning
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
			repl = xplReplaceContentEntries(commandInfo->document, params->id, cur, commandInfo->element->children, cur->parent);
			if (params->mode == WITH_MODE_REPLACE)
			{
				xplDocDeferNodeListDeletion(commandInfo->document, xplDetachChildren(cur));
				xplSetChildren(cur, repl);
				xplNodeApply(commandInfo->document, cur, &temp_result);
				if ((int) cur->type & XPL_NODE_DELETION_MASK)
					xmlUnlinkNode(cur);
				else {
					xplReplaceWithList(cur, cur->children);
					cur->children = cur->last = NULL;
				}
				xplDocDeferNodeDeletion(commandInfo->document, cur);
			} else if (params->mode == WITH_MODE_APPEND) {
				xplAppendChildren(cur, repl);
				xplNodeListApply(commandInfo->document, repl, &temp_result);
			} else
				DISPLAY_INTERNAL_ERROR_MESSAGE();
		} /* for */
	} /* if (sel->nodesetval) */
	commandInfo->document->iterator_spin--;
	if (params->select->nodesetval)
		params->select->nodesetval->nodeNr = 0;
	if (!((int) commandInfo->element->type & XPL_NODE_DELETION_MASK))
		xmlFreeNodeList(xplDetachChildren(commandInfo->element));
}

void xplCmdWithEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	UNUSED_PARAM(commandInfo);
	ASSIGN_RESULT(NULL, false, true);
}
