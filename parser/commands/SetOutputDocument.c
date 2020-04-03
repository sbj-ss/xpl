#include "commands/SetOutputDocument.h"
#include "Core.h"
#include "Messages.h"
#include "Utils.h"
#include "Wrappers.h"

void xplCmdSetOutputDocumentPrologue(xplCommandInfoPtr commandInfo)
{
}

xmlNodePtr checkNodelist(xmlNodePtr src, int *elCount)
{
	xmlNodePtr ret = NULL;

	while (src)
	{
		if (src->type == XML_ELEMENT_NODE)
		{
			if (ret)
			{
				*elCount = 2;
				return NULL;
			} else
				ret = src;
		}
		src = src->next;
	}
	*elCount = (ret? 1: 0);
	return ret;
}

void xplCmdSetOutputDocumentEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplDocumentPtr xpl_doc;
	xmlDocPtr xml_doc = NULL;
	int element_count;
	xmlNodePtr content;

	if ((commandInfo->document->role != XPL_DOC_ROLE_EPILOGUE) && (commandInfo->document->role != XPL_DOC_ROLE_PROLOGUE))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "this command can only be called from prologue or epilogue"), TRUE, TRUE);
		goto done;
	}
	content = checkNodelist(commandInfo->element->children, &element_count);
	if (!element_count)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "no root node found"), TRUE, TRUE);
		goto done;
	}
	if (element_count > 1)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "multiple root nodes found"), TRUE, TRUE);
		goto done;
	}
	content = cloneNode(content, NULL, NULL);
	xpl_doc = commandInfo->document->main;
	if (!xpl_doc)
	{
		xpl_doc = (xplDocumentPtr) xmlMalloc(sizeof(xplDocument));
		memset(xpl_doc, 0, sizeof(xplDocument));
		xpl_doc->prologue = commandInfo->document->prologue;
		xpl_doc->epilogue = commandInfo->document->epilogue;
		commandInfo->document->main = xpl_doc;
		xml_doc = xpl_doc->document;
	}
	if ((xml_doc = xpl_doc->document))
		xmlFreeDoc(xml_doc);
	xpl_doc->source = XPL_DOC_SOURCE_OVERRIDDEN;
	xpl_doc->status = XPL_ERR_NO_ERROR;
	xpl_doc->document = xml_doc = xmlNewDoc(BAD_CAST "1.0");
	setChildren((xmlNodePtr) xml_doc, content);
	ASSIGN_RESULT(NULL, FALSE, TRUE);
done:
	;
}

xplCommand xplSetOutputDocumentCommand = { xplCmdSetOutputDocumentPrologue, xplCmdSetOutputDocumentEpilogue };
