#include <libxpl/xplsave.h>
#include <libxpl/abstraction/xpr.h>
#include <libxml/xmlsave.h>
#include <unistd.h>
#include <fcntl.h>

static xplOutputMethodDesc output_methods[] =
{
	{
		.name = BAD_CAST "html",
		.content_type = BAD_CAST "text/html",
		.xml_format = XML_SAVE_AS_HTML,
		.serializer_type = XPL_OST_XML
	}, {
		.name = BAD_CAST "xml",
		.content_type = BAD_CAST "text/xml",
		.xml_format = XML_SAVE_AS_XML,
		.serializer_type = XPL_OST_XML
	}, {
		.name = BAD_CAST "xhtml",
		.content_type = BAD_CAST "application/xhtml+xml",
		.xml_format = XML_SAVE_XHTML,
		.serializer_type = XPL_OST_XML
	}, {
		.name = BAD_CAST "text",
		.content_type = BAD_CAST "text/plain",
		.xml_format = 0,
		.serializer_type = XPL_OST_TEXT
	}, {
		.name = BAD_CAST "none",
		.content_type = BAD_CAST "",
		.xml_format = 0,
		.serializer_type = XPL_OST_NONE
	}
};

xplOutputMethodDescPtr xplOutputMethodDescFromString(xmlChar *name)
{
	size_t i;

	for (i = 0; i < sizeof(output_methods) / sizeof(output_methods[0]); i++)
		if (!xmlStrcasecmp(name, output_methods[i].name))
			return &output_methods[i];
	return NULL;
}

static void safeSerializeIndent(FILE *fp, int indent)
{
	int i;

	for (i = 0; i < indent; i++)
		fprintf(fp, " ");
}

static void safeSerializeNodeList(FILE *fp, xmlNodePtr list, int indent);

static void safeSerializeContent(FILE *fp, xmlChar* content)
{
#define BUF_SIZE 1024
	xmlChar buf[BUF_SIZE], *buf_ptr = buf;

	while (*content)
	{
		switch (*content)
		{
		case '<':
			*buf_ptr++ = '&';
			*buf_ptr++ = 'l';
			*buf_ptr++ = 't';
			*buf_ptr++ = ';';
			break;
		case '>':
			*buf_ptr++ = '&';
			*buf_ptr++ = 'g';
			*buf_ptr++ = 't';
			*buf_ptr++ = ';';
			break;
		case '&':
			*buf_ptr++ = '&';
			*buf_ptr++ = 'a';
			*buf_ptr++ = 'm';
			*buf_ptr++ = 'p';
			*buf_ptr++ = ';';
			break;
		case '"':
			*buf_ptr++ = '&';
			*buf_ptr++ = 'q';
			*buf_ptr++ = 'u';
			*buf_ptr++ = 'o';
			*buf_ptr++ = 't';
			*buf_ptr++ = ';';
			break;
		default:
			*buf_ptr++ = *content;
		}
		content++;
		if ((buf_ptr - buf) >= (BUF_SIZE - 8))
		{
			*buf_ptr = 0;
			fprintf(fp, "%s", buf);
			buf_ptr = buf;
		}
	}
	if (buf_ptr > buf)
	{
		*buf_ptr = 0;
		fprintf(fp, "%s", buf);
	}
#undef BUF_SIZE
}

static void safeSerializeElement(FILE *fp, xmlNodePtr node, int indent)
{
	xmlNsPtr nsdefs;

	/* start element */
	safeSerializeIndent(fp, indent);
	if (node->ns)
		fprintf(fp, "<%s:%s", node->ns->prefix, node->name);
	else
		fprintf(fp, "<%s", node->name);
	/* cycle through namespace definitions */
	nsdefs = node->nsDef;
	while (nsdefs)
	{
		if (nsdefs->prefix)
			fprintf(fp, " xmlns:%s=\"%s\"", nsdefs->prefix, nsdefs->href);
		else
			fprintf(fp, " xmlns=\"%s\"", nsdefs->href);
		nsdefs = nsdefs->next;
	}
	/* cycle through attributes */
	safeSerializeNodeList(fp, (xmlNodePtr) node->properties, indent);
	fprintf(fp, ">");
	/* serialize children */
	safeSerializeNodeList(fp, node->children, indent + 2);
	/* end element */
	safeSerializeIndent(fp, indent);
	if (node->ns)
		fprintf(fp, "</%s:%s>\n", node->ns->prefix, node->name);
	else
		fprintf(fp, "</%s>\n", node->name);
}

static void safeSerializeProp(FILE *fp, xmlNodePtr node, int indent)
{
	if (node->ns)
		fprintf(fp, "%s:%s=\"", node->ns->prefix, node->name);
	else
		fprintf(fp, "%s=\"", node->name);
	safeSerializeNodeList(fp, node->children, indent);
	fprintf(fp, "\"");
}

static void safeSerializeEntityRef(FILE *fp, xmlNodePtr node, int indent)
{
	xmlEntityPtr entity;

	/* dump entities literally */
	if (!(entity = xmlGetDocEntity(node->doc, node->name)))
		return;
	safeSerializeNodeList(fp, entity->children, indent);
}

static void safeSerializeNode(FILE *fp, xmlNodePtr node, int indent)
{
	if (!fp || !node)
		return;
	switch(node->type)
	{
	case XML_ELEMENT_NODE:
		safeSerializeElement(fp, node, indent);
		break;
	case XML_TEXT_NODE:
	case XML_CDATA_SECTION_NODE:
		if (node->content)
			safeSerializeContent(fp, node->content);
		break;
	case XML_ATTRIBUTE_NODE:
		safeSerializeProp(fp, node, indent);
		break;
	case XML_ENTITY_REF_NODE:
		safeSerializeEntityRef(fp, node, indent);
		break;
	case XML_PI_NODE:
		/* this is mostly a session saving mechanism - and PIs are unlikely to be present here. */
		break;
	default:
		return;
	}
}

static void safeSerializeNodeList(FILE *fp, xmlNodePtr list, int indent)
{
	if (!fp)
		return;
	while (list)
	{
		safeSerializeNode(fp, list, indent);
		list = list->next;
	}
}

void xplSafeSerializeDocument(const xmlChar *filename, xmlDocPtr doc)
{
	FILE *fp = xprFOpen(filename, "w");
	if (!fp)
		return;
	fprintf(fp, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	safeSerializeNode(fp, doc->children, 0);
	fclose(fp);
}

/* document saving */

static int xml_save_write_cb(void *context, const char *buffer, int len)
{
	if (!len)
		return 0;
	return write((int) (size_t) context, buffer, len);
}

static int xml_save_close_cb(void *context)
{
	close((int) (size_t) context);
	return 0;
}

bool xplSaveXmlDocToFile(const xmlDocPtr doc, const xmlChar *filename, const char *encoding, int options)
{
	int fh;
	xmlSaveCtxtPtr save_ctxt;

	if (!doc || !filename)
		return false;
	fh = xprSOpen(filename, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY, _SH_DENYWR, S_IREAD | S_IWRITE);
	if (fh == -1)
		return false;
	save_ctxt = xmlSaveToIO(xml_save_write_cb, xml_save_close_cb, (void*) (size_t) fh, encoding, options);
	if (!save_ctxt)
		return false;
	xmlSaveSetEscape(save_ctxt, NULL);
	xmlSaveSetAttrEscape(save_ctxt, NULL);
	xmlSaveDoc(save_ctxt, doc);
	xmlSaveClose(save_ctxt);
	close(fh);
	return true;
}


/* serialization */

xmlChar* xplSerializeNodeList(const xmlNodePtr src)
{
	xmlBufferPtr buf;
	xmlSaveCtxtPtr save_ctxt;
	xmlChar* ret;
	xmlNodePtr cur;

	if (!src)
		return NULL;
	buf = xmlBufferCreate();
	if (!buf)
		return NULL;
	save_ctxt = xmlSaveToBuffer(buf, NULL, 0);
	if (!save_ctxt)
	{
		xmlBufferFree(buf);
		return NULL;
	}
	xmlSaveSetEscape(save_ctxt, NULL);
	for (cur = src; cur; cur = cur->next)
		xmlSaveTree(save_ctxt, cur);
	xmlSaveFlush(save_ctxt);
	ret = buf->content;
	buf->content = NULL;
	xmlSaveClose(save_ctxt);
	xmlBufferFree(buf);
	return ret;
}

xmlChar* xplSerializeNodeSet(const xmlNodeSetPtr set)
{
	size_t i;
	xmlNodePtr cur;
	xmlBufferPtr buf;
	xmlSaveCtxtPtr save_ctxt;
	xmlChar *ret;

	if (!set)
		return NULL;
	buf = xmlBufferCreate();
	if (!buf)
		return NULL;
	save_ctxt = xmlSaveToBuffer(buf, NULL, 0);
	if (!save_ctxt)
	{
		xmlBufferFree(buf);
		return NULL;
	}
	xmlSaveSetEscape(save_ctxt, NULL);
	for (i = 0; i < (size_t) set->nodeNr; i++)
	{
		cur = set->nodeTab[i];
		xmlSaveTree(save_ctxt, cur);
		cur = cur->next;
	}
	xmlSaveFlush(save_ctxt);
	ret = buf->content;
	buf->content = NULL;
	xmlSaveClose(save_ctxt);
	xmlBufferFree(buf);
	return ret;
}
