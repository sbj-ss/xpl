#include <libxml/xmlsave.h>
#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplsave.h>
#include <libxpl/xpltree.h>

void xplCmdSaveEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdSaveParams
{
	xmlChar *file;
	xmlXPathObjectPtr select;
	xmlChar *encoding;
	bool format;
	bool abs_path;
	bool omit_root;
	bool create_destination;
	xplQName root;
} xplCmdSaveParams, *xplCmdSaveParamsPtr;

static const xplCmdSaveParams params_stencil =
{
	.file = NULL,
	.select = NULL,
	.encoding = BAD_CAST "utf-8",
	.format = true,
	.abs_path = false,
	.omit_root = false,
	.root = { NULL, BAD_CAST "Root" }
};

xplCommand xplSaveCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdSaveEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdSaveParams),
	.parameters = {
		{
			.name = BAD_CAST "file",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.required = true,
			.value_stencil = &params_stencil.file
		}, {
			.name = BAD_CAST "select",
			.type = XPL_CMD_PARAM_TYPE_XPATH,
			.extra.xpath_type = XPL_CMD_PARAM_XPATH_TYPE_ANY,
			.value_stencil = &params_stencil.select
		}, {
			.name = BAD_CAST "encoding",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.encoding
		}, {
			.name = BAD_CAST "format",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.format
		}, {
			.name = BAD_CAST "abspath",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.abs_path
		}, {
			.name = BAD_CAST "omitroot",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.omit_root
		}, {
			.name = BAD_CAST "createdestination",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.create_destination
		}, {
			.name = BAD_CAST "root",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.value_stencil = &params_stencil.root
		}, {
			.name = NULL
		}
	}
};

void xplCmdSaveEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdSaveParamsPtr params = (xplCmdSaveParamsPtr) commandInfo->params;
	xmlChar *filename;
	xmlDocPtr doc = NULL;
	xmlNodePtr root = NULL;
	xmlNodeSetPtr nodes;
	xmlChar *value;
	size_t i;
	int options = 0;

	if (params->abs_path)
		filename = BAD_CAST XPL_STRDUP((char*) params->file);
	else
		filename = xplFullFilename(params->file, commandInfo->document->app_path);
	if (params->format)
		options |= XML_SAVE_FORMAT;
	xprConvertSlashes(filename); // TODO not here

	doc = xmlNewDoc(BAD_CAST "1.0");
	if (!params->omit_root)
	{
		root = xmlNewDocNode(doc, NULL, params->root.ncname, NULL);
		if (params->root.ns)
			root->nsDef = root->ns = xmlCopyNamespace(params->root.ns);
		xmlAddChild((xmlNodePtr) doc, root);
	}
	/* We can transform scalar selection values into text nodes unless omit_root is requested.
	   And if it is - we can only use a single element node for a root. */
	if (params->select)
	{
		if (params->select->type == XPATH_NODESET)
		{
			if (params->select->nodesetval && params->select->nodesetval->nodeNr)
			{
				nodes = params->select->nodesetval;
				if (params->omit_root)
				{
					if ((nodes->nodeNr != 1) || (nodes->nodeTab[0]->type != XML_ELEMENT_NODE))
					{
						ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "omitroot requires exactly one element node returned by XPath selection"), true, true);
						goto done;
					}
					root = nodes->nodeTab[0];
					xmlAddChild((xmlNodePtr) doc, root);
				} else
					for (i = 0; i < (size_t) nodes->nodeNr; i++)
						xmlAddChild(root, xplCloneAsNodeChild(nodes->nodeTab[i], root));
			} else if (params->omit_root) {
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "XPath selection (%s) returned an empty result but omitroot is requested", params->select), true, true);
				goto done;
			}
		} else if (params->omit_root) {
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath expression (%s) evaluated to non-nodeset value", params->select), true, true);
			goto done;
		} else if ((params->select->type == XPATH_BOOLEAN) || (params->select->type == XPATH_NUMBER) || (params->select->type == XPATH_STRING)) {
			value = xmlXPathCastToString(params->select);
			xmlAddChild(root, xmlNewDocText(doc, NULL));
			root->children->content = value;
		} else {
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath expression (%s) evaluated to unsupported type %d", params->select, params->select->type), true, true);
			goto done;
		}
	} else {
		if (params->omit_root)
		{
			if (!commandInfo->element->children)
			{
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "omitroot requires non-empty command content"), true, true);
				goto done;				
			}
			if (commandInfo->element->children->next)
			{
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "command has multiple child nodes but omitroot is requested"), true, true);
				goto done;
			}
			if (commandInfo->element->children->type != XML_ELEMENT_NODE)
			{
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "command content is non-element but omitroot is requested"), true, true);
				goto done;
			}
			root = xplCloneNode(commandInfo->element->children, (xmlNodePtr) doc, doc);
			xmlAddChild((xmlNodePtr) doc, root);
		} else 
			/* we have to duplicate content or we risk to lose namespaces */
			xmlAddChildList(root, xplCloneNodeList(commandInfo->element->children, root, doc));
	}

	if (params->create_destination)
		xprEnsurePathExistence(filename);
	if (!xplSaveXmlDocToFile(doc, filename, (char*) params->encoding, options))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "cannot save document \"%s\" using encoding \"%s\"", filename, params->encoding), true, true);
		goto done;
	}
	ASSIGN_RESULT(NULL, false, true);
done:
	if (filename)
		XPL_FREE(filename);
	if (doc)
		xmlFreeDoc(doc);
}
