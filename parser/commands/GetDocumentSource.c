#include "commands/GetDocumentSource.h"
#include "Core.h"
#include "Messages.h"
#include "Utils.h"

void xplCmdGetDocumentSourcePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdGetDocumentSourceEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define DOCUMENT_ATTR (BAD_CAST "document")
	xmlChar *document_attr = NULL;
	xplDocSource src;
	xplDocumentPtr doc;
	xmlNodePtr ret;

	document_attr = xmlGetNoNsProp(commandInfo->element, DOCUMENT_ATTR);
	if (!xplGetDocByRole(commandInfo->document, document_attr, &doc))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid select attribute value \"%s\"", document_attr), true, true);
		goto done;
	}
	src = doc? doc->source: XPL_DOC_SOURCE_ABSENT;
	ret = xmlNewDocText(commandInfo->element->doc, xplDocSourceToString(src));
	ASSIGN_RESULT(ret, false, true);
done:
	if (document_attr)
		xmlFree(document_attr);
}

xplCommand xplGetDocumentSourceCommand = { xplCmdGetDocumentSourcePrologue, xplCmdGetDocumentSourceEpilogue };
