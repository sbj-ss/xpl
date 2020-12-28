#include <string.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>
#include <libxpl/xplwrappers.h>

void xplCmdSetOutputDocumentEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

xplCommand xplSetOutputDocumentCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdSetOutputDocumentEpilogue,
	.flags = 0,
	.params_stencil = NULL
};

typedef enum _ChildNodeCount {
	CNC_ZERO,
	CNC_ONE,
	CNC_MANY
} ChildNodeCount;

static xmlNodePtr _checkChildNodes(xmlNodePtr src, ChildNodeCount *elCount)
{
	xmlNodePtr ret = NULL, cur = src->children;

	if (!cur)
	{
		*elCount = CNC_ZERO;
		return NULL;
	}
	while (cur)
	{
		if (cur->type == XML_ELEMENT_NODE)
		{
			if (ret)
			{
				*elCount = CNC_MANY;
				return NULL;
			} else
				ret = cur;
		}
		cur = cur->next;
	}
	*elCount = CNC_ONE;
	return ret;
}

void xplCmdSetOutputDocumentEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplDocumentPtr xpl_doc;
	xmlDocPtr xml_doc = NULL;
	ChildNodeCount element_count;
	xmlNodePtr content;

	// TODO перебрать это всё
	if ((commandInfo->document->role != XPL_DOC_ROLE_EPILOGUE) && (commandInfo->document->role != XPL_DOC_ROLE_PROLOGUE))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "this command can only be called from prologue or epilogue"), true, true);
		return;
	}
	content = _checkChildNodes(commandInfo->element, &element_count);
	if (element_count == CNC_ZERO)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "no root node found"), true, true);
		return;
	}
	if (element_count == CNC_MANY)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "multiple root nodes found"), true, true);
		return;
	}
	content = xplCloneNode(content, NULL, NULL); // TODO так нельзя
	xpl_doc = commandInfo->document->main;
	if (!xpl_doc)
	{
		xpl_doc = (xplDocumentPtr) XPL_MALLOC(sizeof(xplDocument));
		memset(xpl_doc, 0, sizeof(xplDocument));
		xpl_doc->prologue = commandInfo->document->prologue;
		xpl_doc->epilogue = commandInfo->document->epilogue;
		commandInfo->document->main = xpl_doc;
	}
	if ((xml_doc = xpl_doc->document))
		xmlFreeDoc(xml_doc);
	xpl_doc->source = XPL_DOC_SOURCE_OVERRIDDEN;
	xpl_doc->status = XPL_ERR_NO_ERROR;
	xpl_doc->document = xml_doc = xmlNewDoc(BAD_CAST "1.0");
	xplSetChildren((xmlNodePtr) xml_doc, content);
	ASSIGN_RESULT(NULL, false, true);
}
