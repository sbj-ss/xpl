#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>

void xplCmdRenameEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdRenameParams
{
	xmlXPathObjectPtr select;
	xplQName new_name;
} xplCmdRenameParams, *xplCmdRenameParamsPtr;

static const xplCmdRenameParams params_stencil =
{
	.select = NULL,
	.new_name = { NULL, NULL }
};

xplCommand xplRenameCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdRenameEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdRenameParams),
	.parameters = {
		{
			.name = BAD_CAST "select",
			.type = XPL_CMD_PARAM_TYPE_XPATH,
			.required = true,
			.extra.xpath_type = XPL_CMD_PARAM_XPATH_TYPE_NODESET,
			.value_stencil = &params_stencil.select
		}, {
			.name = BAD_CAST "newname",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.required = true,
			.value_stencil = &params_stencil.new_name
		}, {
			.name = NULL
		}
	}
};

void xplCmdRenameEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdRenameParamsPtr params = (xplCmdRenameParamsPtr) commandInfo->params;
	size_t i;
	xmlNodePtr cur;
	xmlNsPtr ns_copy;

	if (params->select->nodesetval)
	{
		for (i = 0; i < (size_t) params->select->nodesetval->nodeNr; i++)
		{
			cur = params->select->nodesetval->nodeTab[i];
			if ((cur->type == XML_ELEMENT_NODE) || (cur->type == XML_ATTRIBUTE_NODE))
			{
				xmlNodeSetName(cur, params->new_name.ncname);
				if (params->new_name.ns)
				{
					if (xmlSearchNsByHref(cur->doc, cur, params->new_name.ns->href))
						cur->ns = params->new_name.ns;
					else {
						ns_copy = xmlCopyNamespace(params->new_name.ns);
						xplAppendNsDef(cur, ns_copy);
						cur->ns = ns_copy;
					}
				} else
					cur->ns = NULL;
			} else if (cfgWarnOnInvalidNodeType)
				xplDisplayWarning(commandInfo->element, "can only rename elements and attributes, select = '%s'", (char*) params->select->user);
		}
	}
	ASSIGN_RESULT(NULL, false, true);
}
