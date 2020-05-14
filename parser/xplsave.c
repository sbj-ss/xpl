#include <libxpl/xplsave.h>
#include <libxpl/abstraction/xpr.h>
#include <libxml/xmlsave.h>
#include <unistd.h>

void safeSerializeContent(FILE *fp, xmlChar* content)
{
#define BUF_SIZE 1024
	xmlChar buf[BUF_SIZE], *buf_ptr;

	if (!fp || !content)
		return;
	buf_ptr = buf;
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

void safeSerializeNode(FILE *fp, xmlNodePtr node, int indent)
{
	xmlNsPtr nsdefs;
	int i;

	if (!fp || !node)
		return;
	switch(node->type)
	{
	case XML_ELEMENT_NODE:
		/* start element */
		for (i = 0; i < indent; i++)
			fprintf(fp, " ");
		if (node->ns)
			fprintf(fp, "<%s:%s", node->ns->prefix, node->name);
		else
			fprintf(fp, "<%s", node->name);
		/* cycle through nsdefs */
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
		for (i = 0; i < indent; i++)
			fprintf(fp, " ");
		if (node->ns)
			fprintf(fp, "</%s:%s>\n", node->ns->prefix, node->name);
		else
			fprintf(fp, "</%s>\n", node->name);
		break;
	case XML_TEXT_NODE:
		if (node->content)
			safeSerializeContent(fp, node->content);
		break;
	case XML_ATTRIBUTE_NODE:
		if (node->ns)
			fprintf(fp, "%s:%s=\"", node->ns->prefix, node->name);
		else
			fprintf(fp, "%s=\"", node->name);
		safeSerializeNodeList(fp, node->children, indent);
		fprintf(fp, "\"");
		break;
	case XML_ENTITY_NODE:
		/* ToDo */
		break;
	case XML_ENTITY_REF_NODE:
		/* ToDo */
		break;
	case XML_PI_NODE:
		/* TODO */
		break;
	default:
		return;
	}
}

void safeSerializeNodeList(FILE *fp, xmlNodePtr list, int indent)
{
	if (!fp)
		return;
	while (list)
	{
		safeSerializeNode(fp, list, indent);
		list = list->next;
	}
}

void safeSerializeDocument(char *filename, xmlDocPtr doc)
{
	FILE *fp = fopen(filename, "w");
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

bool saveXmlDocToFile(xmlDocPtr doc, xmlChar *filename, char *encoding, int options)
{
	int fh, ret;
	xmlSaveCtxtPtr save_ctxt;

	fh = xprSOpen(filename, O_CREAT | O_TRUNC | O_RDWR | O_BINARY, 0, S_IREAD | S_IWRITE);
	if (fh == -1)
		return false;
	save_ctxt = xmlSaveToIO(xml_save_write_cb, xml_save_close_cb, (void*) (size_t) fh, encoding, options);
	if (!save_ctxt)
		return false;
	xmlSaveSetEscape(save_ctxt, NULL);
	xmlSaveSetAttrEscape(save_ctxt, NULL);
	xmlSaveDoc(save_ctxt, doc);
	xmlSaveClose(save_ctxt);
	return true;
}


/* serialization */

xmlChar* serializeNodeList(xmlNodePtr cur)
{
	xmlBufferPtr buf;
	xmlSaveCtxtPtr save_ctxt;
	xmlChar* ret;

	if (!cur)
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
	while (cur != NULL)
	{
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

xmlChar* serializeNodeSet(xmlNodeSetPtr set)
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
