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
		filename = xplFullFilename(params->file, commandInfo->document->path);
	if (params->format)
		options |= XML_SAVE_FORMAT;

	doc = xmlNewDoc(BAD_CAST "1.0");
	if (!params->omit_root)
	{
		root = xmlNewDocNode(doc, NULL, params->root.ncname, NULL);
		if (params->root.ns)
			root->nsDef = root->ns = xmlCopyNamespace(params->root.ns);
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
						ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "omitroot requires exactly one element node returned by XPath selection"), true, true);
						goto done;
					}
					root = xplCloneNode(nodes->nodeTab[0], (xmlNodePtr) doc, doc);
				} else {
					for (i = 0; i < (size_t) nodes->nodeNr; i++)
						xmlAddChild(root, xplCloneAsNodeChild(nodes->nodeTab[i], root));
				}
				xmlAddChild((xmlNodePtr) doc, root);
			} else if (params->omit_root) {
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "XPath expression '%s' returned an empty result but omitroot is requested", (char*) params->select->user), true, true);
				goto done;
			}
		} else if (params->omit_root) {
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "select XPath expression '%s' evaluated to non-nodeset value", (char*) params->select->user), true, true);
			goto done;
		} else if ((params->select->type == XPATH_BOOLEAN) || (params->select->type == XPATH_NUMBER) || (params->select->type == XPATH_STRING)) {
			value = xmlXPathCastToString(params->select);
			xmlAddChild(root, xmlNewDocText(doc, NULL));
			xmlAddChild((xmlNodePtr) doc, root);
			root->children->content = value;
		} else {
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "select XPath expression '%s' evaluated to unsupported type %u", (char*) params->select->user, params->select->type), true, true);
			goto done;
		}
	} else {
		if (params->omit_root)
		{
			if (!commandInfo->element->children)
			{
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "omitroot requires non-empty command content"), true, true);
				goto done;				
			}
			if (commandInfo->element->children->next)
			{
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "command has multiple child nodes but omitroot is requested"), true, true);
				goto done;
			}
			if (commandInfo->element->children->type != XML_ELEMENT_NODE)
			{
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "command content is non-element but omitroot is requested"), true, true);
				goto done;
			}
			root = commandInfo->element->children;
		} else {
			xplSetChildren(root, xplDetachChildren(commandInfo->element));
			xplSetChildren(commandInfo->element, root);
		}
		xplMakeNsSelfContainedTree(root);
		xmlAddChild((xmlNodePtr) doc, xplDetachChildren(commandInfo->element)); /* this line is correct. trace commandInfo->element->children before fixing what isn't broken. */
	}

	if (params->create_destination)
		xprEnsurePathExistence(filename);
	if (!xplSaveXmlDocToFile(doc, filename, (char*) params->encoding, options))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "cannot save document \"%s\" using encoding \"%s\"", filename, params->encoding), true, true);
		goto done;
	}
	ASSIGN_RESULT(NULL, false, true);
done:
	if (filename)
		XPL_FREE(filename);
	if (doc)
		xmlFreeDoc(doc);
}
