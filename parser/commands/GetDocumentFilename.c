#include "commands/GetDocumentFilename.h"
#include "Core.h"
#include "Messages.h"
#include "Utils.h"

void xplCmdGetDocumentFilenamePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdGetDocumentFilenameEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define DOCUMENT_ATTR (BAD_CAST "document")
	xmlChar *document_attr = NULL;
	xplDocumentPtr doc;
	xmlNodePtr ret;
	xmlChar *fn;

	document_attr = xmlGetNoNsProp(commandInfo->element, DOCUMENT_ATTR);
	if (!xplGetDocByRole(commandInfo->document, document_attr, &doc))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid document attribute value \"%s\"", document_attr), TRUE, TRUE);
		goto done;
	}
	if (!doc || !doc->filename)
		fn = NULL;
	else
		fn = doc->filename + xmlStrlen(xplGetDocRoot());
	ret = xmlNewDocText(commandInfo->element->doc, fn);
	ASSIGN_RESULT(ret, FALSE, TRUE);
done:
	if (document_attr) xmlFree(document_attr);
}

xplCommand xplGetDocumentFilenameCommand = { xplCmdGetDocumentFilenamePrologue, xplCmdGetDocumentFilenameEpilogue };
