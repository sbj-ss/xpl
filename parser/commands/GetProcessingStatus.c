#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplutils.h>
#include "commands/GetProcessingStatus.h"

void xplCmdGetProcessingStatusPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdGetProcessingStatusEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define DOCUMENT_ATTR (BAD_CAST "document")
	xmlChar *document_attr = NULL;
	xplDocumentPtr doc;
	xmlNodePtr ret;
	const xmlChar *status;

	document_attr = xmlGetNoNsProp(commandInfo->element, DOCUMENT_ATTR);
	if (!xplGetDocByRole(commandInfo->document, document_attr, &doc))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid document attribute value \"%s\"", document_attr), true, true);
		goto done;
	}
	status = doc? (xplErrorToShortString(doc->status)): BAD_CAST "no_such_doc";
	ret = xmlNewDocText(commandInfo->element->doc, status);
	ASSIGN_RESULT(ret, false, true);

done:
	if (document_attr) xmlFree(document_attr);

}

xplCommand xplGetProcessingStatusCommand = { xplCmdGetProcessingStatusPrologue, xplCmdGetProcessingStatusEpilogue };
