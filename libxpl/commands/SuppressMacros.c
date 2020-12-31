#include <libxml/chvalid.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmacro.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>

void xplCmdSuppressMacrosPrologue(xplCommandInfoPtr commandInfo);
void xplCmdSuppressMacrosEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdSuppressMacrosParams
{
	xmlXPathObjectPtr select;
	xmlChar *list;
	bool repeat;
} xplCmdSuppressMacrosParams, *xplCmdSuppressMacrosParamsPtr;

static const xplCmdSuppressMacrosParams params_stencil =
{
	.select = NULL,
	.list = NULL,
	.repeat = false
};

xplCommand xplSuppressMacrosCommand = {
	.prologue = xplCmdSuppressMacrosPrologue,
	.epilogue = xplCmdSuppressMacrosEpilogue,
	.flags = XPL_CMD_FLAG_CONTENT_SAFE | XPL_CMD_FLAG_PARAMS_FOR_PROLOGUE | XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdSuppressMacrosParams),
	.parameters = {
		{
			.name = BAD_CAST "select",
			.type = XPL_CMD_PARAM_TYPE_XPATH,
			.extra.xpath_type = XPL_CMD_PARAM_XPATH_TYPE_NODESET,
			.value_stencil = &params_stencil.select
		}, {
			.name = BAD_CAST "list",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.list
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = NULL
		}
	}
};

static void _fillMacroHashFromNodeset(xmlNodePtr source, xmlHashTablePtr target, xmlNodeSetPtr nodeset)
{
	xplMacroPtr macro;
	xmlNodePtr cur;
	size_t i;

	for (i = 0; i < (size_t) nodeset->nodeNr; i++)
	{
		cur = nodeset->nodeTab[i];
		if (cur->type == XML_ELEMENT_NODE)
		{
			macro = xplMacroLookup(source, cur->ns? cur->ns->href: NULL, cur->name);
			if (macro)
				xmlHashAddEntry(target, cur->name, macro);
		} /* element */
	} /* for */
}

static void _fillMacroHashFromList(xmlNodePtr source, xmlHashTablePtr target, xmlChar *list)
{
	xmlChar *prev, *cur, *tagname;
	void *macro;
	xmlNsPtr ns;

	prev = cur = list;
	while (*cur)
	{
		if (*cur == ',')
		{
			*cur = 0;
			while(xmlIsBlank(*prev) && (prev < cur))
				prev++;
			EXTRACT_NS_AND_TAGNAME(prev, ns, tagname, source)
			macro = xplMacroLookup(source, ns? ns->href: NULL, tagname);
			if (macro)
				xmlHashAddEntry(target, prev, macro);
			prev = ++cur;
		} else
			cur += xstrGetOffsetToNextUTF8Char(cur);
	}
	if (*prev)
	{
		while(xmlIsBlank(*prev) && *prev)
			prev++;
		EXTRACT_NS_AND_TAGNAME(prev, ns, tagname, source)
		macro = xplMacroLookup(source, ns? ns->href: NULL, tagname);
		if (macro)
			xmlHashAddEntry(target, prev, macro);
	}
}

static void _switchMacro(void *payload, void *data, XML_HCBNC xmlChar *name)
{
	UNUSED_PARAM(name);
	((xplMacroPtr) payload)->disabled_spin += (int) (ptrdiff_t) data;
}

void xplCmdSuppressMacrosPrologue(xplCommandInfoPtr commandInfo)
{
	xplCmdSuppressMacrosParamsPtr params = (xplCmdSuppressMacrosParamsPtr) commandInfo->params;
	xmlHashTablePtr macros = NULL;

	if (params->select || params->list)
	{
		if (params->select && params->select->nodesetval)
		{
				macros = xmlHashCreate(16);
				_fillMacroHashFromNodeset(commandInfo->element, macros, params->select->nodesetval);
		}
		if (params->list)
		{
			if (!macros)
				macros = xmlHashCreate(16);
			_fillMacroHashFromList(commandInfo->element, macros, params->list);
		}
		xmlHashScan(macros, _switchMacro, (void*) 1);
	}
	commandInfo->prologue_state = macros;
}

void xplCmdSuppressMacrosEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdSuppressMacrosParamsPtr params = (xplCmdSuppressMacrosParamsPtr) commandInfo->params;
	xmlHashTablePtr macros;

	macros = (xmlHashTablePtr) commandInfo->prologue_state;
	if (macros)
	{
		xmlHashScan(macros, _switchMacro, (void*) -1);
		xmlHashFree(macros, NULL);
	}
	if (commandInfo->element->type & XPL_NODE_DELETION_MASK)
		ASSIGN_RESULT(NULL, false, false);
	else
		ASSIGN_RESULT(xplDetachContent(commandInfo->element), params->repeat, true);
}
