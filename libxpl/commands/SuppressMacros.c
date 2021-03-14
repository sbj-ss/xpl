#include <libxml/chvalid.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmacro.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>

void xplCmdSuppressMacrosPrologue(xplCommandInfoPtr commandInfo);
void xplCmdSuppressMacrosEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);
void xplCmdSuppressMacrosRestoreState(xplCommandInfoPtr commandInfo);

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
	.restore_state = xplCmdSuppressMacrosRestoreState,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_PROLOGUE | XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
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

static void _fillMacroHashFromNodeSet(xmlNodePtr source, xmlHashTablePtr target, xmlNodeSetPtr nodeset)
{
	xplMacroPtr macro;
	xmlNodePtr cur;
	size_t i;

	for (i = 0; i < (size_t) nodeset->nodeNr; i++)
	{
		cur = nodeset->nodeTab[i];
		if (cur->type == XML_ELEMENT_NODE)
		{
			macro = xplMacroLookupByElement(source, cur);
			if (macro)
				xmlHashAddEntry(target, cur->name, macro);
		} else if (cfgWarnOnInvalidNodeType)
			xplDisplayWarning(source, "only element nodes returned by the select parameter are used");
	}
}

static xmlNodePtr _addMacroToHash(xmlChar *str, xmlHashTablePtr hash, xmlNodePtr source)
{
	xplQName qname;
	void *macro;

	switch(xplParseQName(str, source, &qname))
	{
		case XPL_PARSE_QNAME_OK:
			macro = xplMacroLookupByQName(source, qname);
			if (macro)
				xmlHashAddEntry(hash, str, macro);
			xplClearQName(&qname);
			break;
		case XPL_PARSE_QNAME_UNKNOWN_NS:
			break;
		case XPL_PARSE_QNAME_INVALID_QNAME:
			return xplCreateErrorNode(source, "invalid qname '%s'", str);
	}
	return NULL;
}

static xmlNodePtr _fillMacroHashFromList(xmlNodePtr source, xmlHashTablePtr target, xmlChar *list)
{
	xmlChar *prev, *cur;
	xmlNodePtr error;

	prev = cur = list;
	while (*cur)
	{
		if (*cur == ',')
		{
			*cur = 0;
			while(xmlIsBlank(*prev) && (prev < cur))
				prev++;
			if ((error = _addMacroToHash(prev, target, source)))
				return error;
			prev = ++cur;
		} else
			cur += xstrGetOffsetToNextUTF8Char(cur);
	}
	if (*prev)
	{
		while(xmlIsBlank(*prev) && *prev)
			prev++;
		if ((error = _addMacroToHash(prev, target, source)))
			return error;
	}
	return NULL;
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
	xmlNodePtr error;

	if (params->select || params->list)
	{
		if (params->select && params->select->nodesetval)
		{
			macros = xmlHashCreate(16);
			_fillMacroHashFromNodeSet(commandInfo->element, macros, params->select->nodesetval);
		}
		if (params->list)
		{
			if (!macros)
				macros = xmlHashCreate(16);
			if ((error = _fillMacroHashFromList(commandInfo->element, macros, params->list)))
			{
				xmlHashFree(macros, NULL);
				macros = NULL;
				xplDocDeferNodeListDeletion(commandInfo->document, xplDetachChildren(commandInfo->element));
				commandInfo->prologue_error = error;
			}
		}
		if (macros)
			xmlHashScan(macros, _switchMacro, (void*) 1);
	}
	commandInfo->prologue_state = macros;
}

void xplCmdSuppressMacrosEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdSuppressMacrosParamsPtr params = (xplCmdSuppressMacrosParamsPtr) commandInfo->params;

	if (commandInfo->prologue_error)
		ASSIGN_RESULT(commandInfo->prologue_error, true, true);
	else
		ASSIGN_RESULT(xplDetachChildren(commandInfo->element), params->repeat, true);
}

void xplCmdSuppressMacrosRestoreState(xplCommandInfoPtr commandInfo)
{
	xmlHashTablePtr macros;

	macros = (xmlHashTablePtr) commandInfo->prologue_state;
	if (macros)
	{
		xmlHashScan(macros, _switchMacro, (void*) -1);
		xmlHashFree(macros, NULL);
	}
}
