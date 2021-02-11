#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>

void xplCmdNamespaceEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdNamespaceParams
{
	xmlChar *prefix;
	xmlXPathObjectPtr destination;
	bool replace;
} xplCmdNamespaceParams, *xplCmdNamespaceParamsPtr;

static const xplCmdNamespaceParams params_stencil =
{
	.prefix = NULL,
	.destination = NULL,
	.replace = false
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
			.name = BAD_CAST "replace",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.replace
		}, {
			.name = NULL
		}
	}
};

static bool _setNs(xmlNodePtr cur, xmlChar *prefix, xmlChar *href, bool replace)
{
	xmlNsPtr ns = cur->nsDef;

	while (ns) {
		if (prefix && !ns->prefix)
			continue;
		if ((!prefix && !ns->prefix) || !xmlStrcmp(ns->prefix, prefix))
		{
			if (!replace)
				return false;
			break;
		}
		ns = ns->next;
	}
	if (ns)
	{
		XPL_FREE(BAD_CAST ns->href);
		ns->href = BAD_CAST XPL_STRDUP(href);
	} else
		xmlNewNs(cur, href, prefix);
	return true;
}

void xplCmdNamespaceEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdNamespaceParamsPtr params = (xplCmdNamespaceParamsPtr) commandInfo->params;
	xmlNodeSetPtr nodeset;
	size_t i;

	if (params->destination && (nodeset = params->destination->nodesetval))
	{
		for (i = 0; i < (size_t) nodeset->nodeNr; i++)
		{
			if (nodeset->nodeTab[i]->type != XML_ELEMENT_NODE)
			{
				if (cfgWarnOnInvalidNodeType)
					xplDisplayWarning(commandInfo->element, BAD_CAST "can't assign namespaces to non-elements, destination '%s'", params->destination->user);
				continue;
			}
			if (!_setNs(nodeset->nodeTab[i], params->prefix, commandInfo->content, params->replace) && cfgWarnOnWontReplace)
				xplDisplayWarning(commandInfo->element, BAD_CAST "won't replace existing namespace");
		}
	} else {
		if (!_setNs(commandInfo->element->parent, params->prefix, commandInfo->content, params->replace) && cfgWarnOnWontReplace)
			xplDisplayWarning(commandInfo->element, BAD_CAST "won't replace existing namespace");
	}
	ASSIGN_RESULT(NULL, false, true);
}
