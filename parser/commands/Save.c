#include "commands/Save.h"
#include "Core.h"
#include "Messages.h"
#include "Options.h"
#include "Utils.h"
#include "abstraction/xpr.h"

/* TODO: do we still need to dig into libxml2 guts? */

struct _xmlSaveCtxt {
    void *_private;
    int type;
    int fd;
    const xmlChar *filename;
    const xmlChar *encoding;
    xmlCharEncodingHandlerPtr handler;
    xmlOutputBufferPtr buf;
    xmlDocPtr doc;
    int options;
    int level;
    int format;
    char indent[60 + 1];	/* array for indenting output */
    int indent_nr;
    int indent_size;
    xmlCharEncodingOutputFunc escape;	/* used for element content */
    xmlCharEncodingOutputFunc escapeAttr;/* used for attribute content */
};
typedef struct _xmlSaveCtxt *xmlSaveCtxtPtr;

void xplCmdSavePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdSaveEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define FILE_ATTR (BAD_CAST "file")
#define SELECT_ATTR (BAD_CAST "select")
#define ENCODING_ATTR (BAD_CAST "encoding")
#define FORMAT_ATTR (BAD_CAST "format")
#define ABS_PATH_ATTR (BAD_CAST "abspath")
#define ROOT_ATTR (BAD_CAST "root")
#define CREATE_DESTINATION_ATTR (BAD_CAST "createdestination")
#define OMIT_ROOT_ATTR (BAD_CAST "omitroot")
#define DESERIALIZE_ATTR (BAD_CAST "deserialize")

	xmlChar *file_attr = NULL;
	xmlChar *select_attr = NULL;
	xmlChar *encoding_attr = NULL;
	xmlChar *root_attr = NULL;
	xmlChar *filename = NULL;
	xmlChar *deserialize_attr = NULL;
	xmlChar *root_name;
	xmlNsPtr ns;
	bool format;
	bool abs_path;
	bool create_destination;
	bool omit_root;
	int ret = -1;
	xmlDocPtr doc = NULL;
	xmlXPathObjectPtr sel = NULL;
	xmlNodePtr root, error;
	size_t i;
	int options = 0;

	file_attr = xmlGetNoNsProp(commandInfo->element, FILE_ATTR);
	if (!file_attr || !*file_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "no file name specified"), true, true);
		goto done;
	}
	select_attr = xmlGetNoNsProp(commandInfo->element, SELECT_ATTR);
	encoding_attr = xmlGetNoNsProp(commandInfo->element, ENCODING_ATTR);
	if (!encoding_attr || !*encoding_attr)
		encoding_attr = xmlStrdup(cfgDefaultEncoding);
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, FORMAT_ATTR, &format, true)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, ABS_PATH_ATTR, &abs_path, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, CREATE_DESTINATION_ATTR, &create_destination, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, OMIT_ROOT_ATTR, &omit_root, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	deserialize_attr = xmlGetNoNsProp(commandInfo->element, DESERIALIZE_ATTR);
	if (deserialize_attr && xmlStrcasecmp(deserialize_attr, BAD_CAST "base64"))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "unknown deserialization method \"%s\", the only supported value is \"base64\"", 
			deserialize_attr), true, true);
		goto done;
	}
	root_attr = xmlGetNoNsProp(commandInfo->element, ROOT_ATTR);

	if (abs_path)
		filename = xmlStrdup(file_attr);
	else
		filename = xplFullFilename(file_attr, commandInfo->document->app_path);
	if (format)
		options |= XML_SAVE_FORMAT;
	xprConvertSlashes(filename);
	doc = xmlNewDoc(BAD_CAST "1.0");
	if (!omit_root)
	{
		EXTRACT_NS_AND_TAGNAME(root_attr, ns, root_name, commandInfo->element)
		root = xmlNewDocNode(doc, NULL, root_name? root_name: BAD_CAST "Root", NULL);
		if (ns)
			root->nsDef = root->ns = xmlCopyNamespace(ns);
		xmlAddChild((xmlNodePtr) doc, root);
	}
	if (select_attr)
	{
		sel = xplSelectNodes(commandInfo->document, commandInfo->element, select_attr);
		if (sel)
		{
			if (sel->type == XPATH_NODESET)
			{
				if (sel->nodesetval && sel->nodesetval->nodeNr)
				{
					if (omit_root)
					{ 
						if ((sel->nodesetval->nodeNr != 1) || (sel->nodesetval->nodeTab[0]->type != XML_ELEMENT_NODE))
						{
							ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "omitroot requires exactly one element node returned by XPath selection"), true, true);
							goto done;
						}
						root = sel->nodesetval->nodeTab[0];
						xmlAddChild((xmlNodePtr) doc, root);
					} else
						for (i = 0; i < (size_t) sel->nodesetval->nodeNr; i++)
							xmlAddChild(root, cloneNode(sel->nodesetval->nodeTab[i], root, doc));
				} else if (omit_root) {
					ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "XPath selection (%s) returned an empty result but omitroot is requested", select_attr), true, true);
					goto done;
				}
			} else {
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath expression (%s) evaluated to non-nodeset value", select_attr), true, true);
				goto done;
			}
		} else {
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid select XPath expression (%s)", select_attr), true, true);
			goto done;
		}
	} else {
		if (omit_root)
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
			root = cloneNode(commandInfo->element->children, (xmlNodePtr) doc, doc);
			xmlAddChild((xmlNodePtr) doc, root);
		} else 
			/* we have to duplicate content or we risk to lose namespaces */
			xmlAddChildList(root, cloneNodeList(commandInfo->element->children, root, doc));
	}

	if (create_destination)
		xprEnsurePathExistence(filename);
	if (!saveXmlDocToFile(doc, filename, (char*) encoding_attr, options))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "cannot save document \"%s\" (encoding \"%s\")", filename, encoding_attr), true, true);
		goto done;
	}
	ASSIGN_RESULT(NULL, false, true);
done:
	if (file_attr)
		xmlFree(file_attr);
	if (select_attr)
		xmlFree(select_attr);
	if (encoding_attr)
		xmlFree(encoding_attr);
	if (root_attr)
		xmlFree(root_attr);
	if (deserialize_attr)
		xmlFree(deserialize_attr);
	if (filename)
		xmlFree(filename);
	if (doc)
		xmlFreeDoc(doc);
	if (sel)
		xmlXPathFreeObject(sel);
}

xplCommand xplSaveCommand = { xplCmdSavePrologue, xplCmdSaveEpilogue };
