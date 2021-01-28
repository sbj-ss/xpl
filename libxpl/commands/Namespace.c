#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>

void xplCmdNamespaceEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdNamespaceParams
{
	xmlChar *prefix;
	xmlXPathObjectPtr destination;
} xplCmdNamespaceParams, *xplCmdNamespaceParamsPtr;

static const xplCmdNamespaceParams params_stencil =
{
	.prefix = NULL,
	.destination = NULL
};

xplCommand xplNamespaceCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdNamespaceEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE | XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE | XPL_CMD_FLAG_REQUIRE_CONTENT,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdNamespaceParams),
	.parameters = {
		{
			.name = BAD_CAST "prefix",
			.type = XPL_CMD_PARAM_TYPE_NCNAME,
			.value_stencil = &params_stencil.prefix
		}, {
			.name = BAD_CAST "destination",
			.type = XPL_CMD_PARAM_TYPE_XPATH,
			.extra.xpath_type = XPL_CMD_PARAM_XPATH_TYPE_NODESET,
			.value_stencil = &params_stencil.destination
		}, {
			.name = NULL
		}
	}
};

void xplCmdNamespaceEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdNamespaceParamsPtr params = (xplCmdNamespaceParamsPtr) commandInfo->params;
	xmlNodeSetPtr nodeset;
	xmlNsPtr ns;
	size_t i;

	if (params->destination && params->destination->nodesetval)
	{
		nodeset = params->destination->nodesetval;
		for (i = 0; i < (size_t) nodeset->nodeNr; i++)
		{
			if (nodeset->nodeTab[i]->type != XML_ELEMENT_NODE)
				continue;
			ns = xmlNewNs(nodeset->nodeTab[i], commandInfo->content, params->prefix);
			if (!params->prefix)
				nodeset->nodeTab[i]->ns = ns;
			/* ToDo: checks and warnings */
		}
	} else {
		ns = xmlNewNs(commandInfo->element->parent, commandInfo->content, params->prefix);
		if (!params->prefix)
			commandInfo->element->parent->ns = ns;
		/* ToDo: checks and warnings */
	}
	ASSIGN_RESULT(NULL, false, true);
}
