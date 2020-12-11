#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>

void xplCmdIsDefinedEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdIsDefinedParams
{
	xplQName name;
	xmlXPathObjectPtr at;
} xplCmdIsDefinedParams, *xplCmdIsDefinedParamsPtr;

static const xplCmdIsDefinedParams params_stencil =
{
	.name = { NULL, NULL },
	.at = NULL
};

xplCommand xplIsDefinedCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdIsDefinedEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdIsDefinedParams),
	.parameters = {
		{
			.name = BAD_CAST "name",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.required = true,
			.value_stencil = &params_stencil.name
		}, {
			.name = BAD_CAST "at",
			.type = XPL_CMD_PARAM_TYPE_XPATH,
			.extra.xpath_type = XPL_CMD_PARAM_XPATH_TYPE_NODESET,
			.value_stencil = &params_stencil.at
		}, {
			.name = NULL
		}
	}
};

// TODO warning if at evaluates into multiple nodes
// TODO error if at evaluates into non-elements
// TODO do we check right at selected elements?
void xplCmdIsDefinedEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdIsDefinedParamsPtr params = (xplCmdIsDefinedParamsPtr) commandInfo->params;
	xmlHashTablePtr macro_table;
	xplMacroPtr macro = NULL;
	const xmlChar* value;
	xmlNodePtr ret;
	size_t i;

	if (params->at)
	{
		if (params->at->nodesetval)
		{
			for (i = 0; i < (size_t) params->at->nodesetval->nodeNr; i++)
			{
				macro_table = (xmlHashTablePtr) params->at->nodesetval->nodeTab[i]->_private;
				if (macro_table)
					macro = (xplMacroPtr) xmlHashLookup2(macro_table, params->name.ncname, params->name.ns? params->name.ns->href: NULL);
				if (macro && !macro->disabled_spin)
					break;
			}
		}
	} else
		macro = xplMacroLookup(commandInfo->element->parent, params->name.ns? params->name.ns->href: NULL, params->name.ncname);
	if (macro && !macro->disabled_spin)
		value = BAD_CAST "true";
	else
		value = BAD_CAST "false";
	ret = xmlNewDocText(commandInfo->document->document, value);
	ASSIGN_RESULT(ret, false, true);
}
