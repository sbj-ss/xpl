#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>

void xplCmdDeleteEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdDeleteParams
{
	xmlXPathObjectPtr select;
} xplCmdDeleteParams, *xplCmdDeleteParamsPtr;

static const xplCmdDeleteParams params_stencil =
{
	.select = NULL
};

xplCommand xplDeleteCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdDeleteEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdDeleteParams),
	.parameters = {
		{
			.name = BAD_CAST "select",
			.type = XPL_CMD_PARAM_TYPE_XPATH,
			.required = true,
			.extra.xpath_type = XPL_CMD_PARAM_XPATH_TYPE_NODESET,
			.value_stencil = &params_stencil.select
		}, {
			.name = NULL
		}
	}
};

/* Possible bad things that may happen:
 * a) deletion of the next node set entry ancestor - with the entry itself;
 * b) deletion of the command ancestor or the command itself;
 * c) deletion of a node that is contained in the selection of the currently running iterator.
 * 
 * Consider three situations:
 * 1. We're running inside an iterator. We don't really delete any nodes but mark them for deletion instead
 *    and put them into a buffer which is emptied when document processing is finished;
 * 2. No iterators are currently running.
 *    2.1. When we have less than MAX_ELEMENTS_FOR_RELATIONSHIP_SCAN in selection we calculate a triangle
 *         part of their Cartesian product and remove all descendants of nodes from the selection.
 *         This is quite expensive - O(M*N^2) ~ O(N^3) and may be removed in future versions;
 *    2.2. Otherwise we use a local buffer for deletion in a way similar to how global buffer works.
 *
 *  These checks can be disabled by setting cfgFoolproofDestructiveCommands to false - however the check
 *  for AoS deletion is always performed.
 */

#define MAX_ELEMENTS_FOR_RELATIONSHIP_SCAN 8

void xplCmdDeleteEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdDeleteParamsPtr params = (xplCmdDeleteParamsPtr) commandInfo->params;
	xmlNodeSetPtr nodes;
	xmlNodePtr cur;
	size_t i, j;
	bool double_pass_mode;
	xmlBufferPtr deleted_buf = NULL;
	
	if ((nodes = params->select->nodesetval))
	{
		double_pass_mode = cfgFoolproofDestructiveCommands
			&& (nodes->nodeNr > MAX_ELEMENTS_FOR_RELATIONSHIP_SCAN)
			&& !commandInfo->document->iterator_spin;
		if (!double_pass_mode)
		{
			if (cfgFoolproofDestructiveCommands)
			{
				for (i = 0; i < (size_t) nodes->nodeNr; i++)
					for (j = i + 1; j < (size_t) nodes->nodeNr; j++)
						if (xplIsAncestor(nodes->nodeTab[j], nodes->nodeTab[i]))
							nodes->nodeTab[j] = NULL;
			}
		} else
			deleted_buf = xmlBufferCreateSize((size_t) sizeof(xmlNodePtr)*params->select->nodesetval->nodeNr);
		for (i = 0; i < (size_t) nodes->nodeNr; i++)
		{
			cur = nodes->nodeTab[i];
			if (!cur)
				continue; /* already removed with its parent */
			if ((int) cur->type & XPL_NODE_DELETION_MASK)
			{
				if (cfgWarnOnDeletedNodeReference)
					xplDisplayWarning(commandInfo->element, "node '%s%s%s' (line %d) post-mortem access attempt",
						cur->ns && cur->ns->prefix? cur->ns->prefix: BAD_CAST "",
						cur->ns && cur->ns->prefix? ":": "",
						cur->name, cur->line);
				continue;
			}
			switch (cur->type)
			{
				case XML_ELEMENT_NODE:
					if (xplIsAncestor(commandInfo->element, cur) || (commandInfo->element == cur)) /* self/parent deletion request */
					{
						if (commandInfo->document->iterator_spin) /* mark for deletion at the end of processing */
							xplDocDeferNodeDeletion(commandInfo->document, cur);
						xplMarkAncestorAxisForDeletion(commandInfo->element, cur);
					} else {
						if (double_pass_mode)
							xplDeferNodeDeletion(deleted_buf, cur);
						else
							xplDocDeferNodeDeletion(commandInfo->document, cur);
					}
					break;
				case XML_TEXT_NODE:
				case XML_CDATA_SECTION_NODE:
				case XML_COMMENT_NODE:
				case XML_PI_NODE:
					if (double_pass_mode)
						xplDeferNodeDeletion(deleted_buf, cur);
					else
						xplDocDeferNodeDeletion(commandInfo->document, cur);
					break;
				case XML_ATTRIBUTE_NODE:
					/* attributes can't have child nodes */
					xplUnlinkProp((xmlAttrPtr) cur);
					xplDocDeferNodeDeletion(commandInfo->document, cur);
					break;
				default:
					/* we're trying to do something weird like deleting a DTD */
					if (cfgWarnOnInvalidNodeType)
						xplDisplayWarning(commandInfo->element, "attempting to delete a node '%s%s%s', type %u declared at line %d",
							cur->ns && cur->ns->prefix? cur->ns->prefix: BAD_CAST "",
							cur->ns && cur->ns->prefix? ":": "",
							cur->name, cur->type, cur->line);
					break;
			} /* switch */
		} /* for: first pass */
		if (double_pass_mode)
		{
			xplDeleteDeferredNodes(deleted_buf);
			xmlBufferFree(deleted_buf);
		}
	} /* if nodesetval is present */
	ASSIGN_RESULT(NULL, false, true);
}
