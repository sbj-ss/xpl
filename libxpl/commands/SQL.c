#include <string.h>
#include <libxpl/abstraction/xef.h>
#include <libxpl/xplcore.h>
#include <libxpl/xpldb.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>

void xplCmdSqlEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

#define DEFAULT_RESPONSE_TAG_NAME (BAD_CAST "Row")
#define DEFAULT_COLUMN_NAME (BAD_CAST "Col")

typedef struct _xplCmdSqlParams
{
	bool as_attributes;
	bool cleanup_stream;
	xplQName default_column_name;
	bool merge_table_as_xml;
	bool repeat;
	bool keep_nulls;
	xmlChar *tag_name;
	bool show_nulls;
} xplCmdSqlParams, *xplCmdSqlParamsPtr;

static const xplCmdSqlParams params_stencil =
{
	.as_attributes = false,
	.cleanup_stream = false,
	.default_column_name = { NULL, DEFAULT_COLUMN_NAME },
	.keep_nulls = false,
	.merge_table_as_xml = false,
	.repeat = true,
	.tag_name = DEFAULT_RESPONSE_TAG_NAME,
	.show_nulls = false
};

static xmlChar* tagname_aliases[] = { BAD_CAST "responsetagname", NULL };

xplCommand xplSqlCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdSqlEpilogue,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdSqlParams),
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE | XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE | XPL_CMD_FLAG_REQUIRE_CONTENT,
	.parameters = {
		{
			.name = BAD_CAST "asattributes",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.as_attributes
		}, {
			.name = BAD_CAST "cleanupstream",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.cleanup_stream
		}, {
			.name = BAD_CAST "defaultcolumnname",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.value_stencil = &params_stencil.default_column_name
		}, {
			.name = BAD_CAST "keepnulls",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.keep_nulls
		}, {
			.name = BAD_CAST "mergetableasxml",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.merge_table_as_xml
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = BAD_CAST "tagname",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.aliases = tagname_aliases,
			.value_stencil = &params_stencil.tag_name
		}, {
			.name = BAD_CAST "shownulls",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.show_nulls
		}, {
			.name = NULL
		}
	}
};

#ifndef _XEF_HAS_DB
void xplCmdSqlEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "No database support is compiled in"), true, true);
}
#else

#define NULL_ATTRIBUTE_NAME (BAD_CAST "isnull")
#define DOC_START (BAD_CAST "<?xml version=\"1.0\" encoding=\"UTF-8\"?><Root>")
#define DOC_END (BAD_CAST "</Root>")

typedef struct _xplSqlRowTagNames
{
	size_t count;
	size_t pos;
	xmlChar **names;
} xplSqlRowTagNames, *xplSqlRowTagNamesPtr;

static void _freeRowTagNames(xplSqlRowTagNamesPtr names)
{
	size_t i;

	if (!names)
		return;
	if (names->names)
	{
		for (i = 0; i < names->count; i++)
			if(names->names[i])
				XPL_FREE(names->names[i]);
		XPL_FREE(names->names);
	}
	XPL_FREE(names);
}

static xplSqlRowTagNamesPtr _createRowTagNames(xmlChar *names)
{
	xmlChar *value_start = NULL, *p;
	xplSqlRowTagNamesPtr ret;
	bool stop = false;

	if (!(ret = (xplSqlRowTagNamesPtr) XPL_MALLOC(sizeof(xplSqlRowTagNames))))
		return NULL;
	ret->pos = 0;
	if (!names)
	{
		ret->count = 0;
		ret->names = NULL;
		return ret;
	}
	ret->count = 1;
	p = names;
	while (*p)
		if (*p++ == ',')
			ret->count++;
	ret->names = (xmlChar**) XPL_MALLOC(ret->count * sizeof(xmlChar*));
	if (!ret->names)
	{
		XPL_FREE(ret);
		return NULL;
	}
	p = names;
	do {
		if (!*p)
			stop = true;
		switch (*p)
		{
			case ' ':
			case '\t':
				break;
			case 0:
			case ',':
				if (value_start)
					ret->names[ret->pos] = xmlStrndup(value_start, (int) (p - value_start));
				else
					ret->names[ret->pos] = BAD_CAST XPL_STRDUP("");
				ret->pos++;
				value_start = NULL;
				break;
			default:
				if (!value_start)
					value_start = p;
		}
		p++;
	} while (!stop);
	ret->pos = 0;
	return ret;
}

typedef struct _xplSqlXmlRowDesc {
	size_t count;
	xplQName *qnames;
} xplSqlXmlRowDesc, *xplSqlXmlRowDescPtr;

static void _freeXmlRowDesc(xplSqlXmlRowDescPtr desc)
{
	size_t i;

	if (!desc)
		return;
	if (desc->qnames)
	{
		for (i = 0; i < desc->count; i++)
			xplClearQName(&desc->qnames[i]);
		XPL_FREE(desc->qnames);
	}
	XPL_FREE(desc);
}

static xplSqlXmlRowDescPtr _createXmlRowDesc(xefDbRowPtr row, xmlNodePtr parent, xplQName defaultColumnName, xmlNodePtr *error)
{
	size_t i;
	xplSqlXmlRowDescPtr ret;
	xmlChar *field_name;

	*error = NULL;
	ret = (xplSqlXmlRowDescPtr) XPL_MALLOC(sizeof(xplSqlXmlRowDesc));
	if (!ret)
		return NULL;
	ret->qnames = (xplQName*) XPL_MALLOC(row->field_count * sizeof(xplQName));
	if (!ret->qnames)
	{
		XPL_FREE(ret);
		return NULL;
	}
	ret->count = row->field_count;
	memset(ret->qnames, 0, ret->count * sizeof(xplQName));
	for (i = 0; i < ret->count; i++)
	{
		field_name = row->fields[i].name;
		if (!field_name || !*field_name)
			ret->qnames[i] = defaultColumnName;
		else if (xplParseQName(field_name, parent, &ret->qnames[i]) != XPL_PARSE_QNAME_OK)
		{
			*error = xplCreateErrorNode(parent, BAD_CAST "invalid field name '%s'", field_name);
			_freeXmlRowDesc(ret);
			return NULL;
		}
	}
	return ret;
}

typedef struct _xplTdsFragmentRowContext
{
	/* set by caller */
	xmlNodePtr head;
	xmlNodePtr tail;
	xmlNodePtr parent;
	xplQName default_column_qname;
	bool as_attributes;
	bool keep_nulls;
	bool show_nulls;
	/* used internally */
	xplSqlXmlRowDescPtr xml_desc;
	xplQName row_qname;
} xplTdsFragmentRowContext, *xplTdsFragmentRowContextPtr;

static bool _TdsFragmentRowScanner(xefDbRowPtr row, void *payload)
{
	xmlNodePtr row_el = NULL, col, error = NULL;
	xplTdsFragmentRowContextPtr ctxt = (xplTdsFragmentRowContextPtr) payload;
	ssize_t i;

	if (!ctxt->xml_desc)
		ctxt->xml_desc = _createXmlRowDesc(row, ctxt->parent, ctxt->default_column_qname, &error);
	if (error)
	{
		if (ctxt->head)
			xmlFreeNodeList(ctxt->head);
		ctxt->head = ctxt->tail = error;
		return false;
	}
	if (ctxt->row_qname.ncname && *(ctxt->row_qname.ncname))
		row_el = xmlNewDocNode(ctxt->parent->doc, ctxt->row_qname.ns, ctxt->row_qname.ncname, NULL);
	for (i = 0; i < row->field_count; i++)
	{
		if (row->fields[i].is_null && !ctxt->keep_nulls)
			continue;
		if (ctxt->as_attributes) 
			col = (xmlNodePtr) xmlNewNsProp(row_el, ctxt->xml_desc->qnames[i].ns, ctxt->xml_desc->qnames[i].ncname, NULL);
		else {
			col = xmlNewDocNode(ctxt->parent->doc, ctxt->xml_desc->qnames[i].ns, ctxt->xml_desc->qnames[i].ncname, NULL);
			if (row->fields[i].is_null && ctxt->show_nulls)
				xmlNewProp(col, NULL_ATTRIBUTE_NAME, BAD_CAST "true");
			if (row_el)
			{
				col->parent = row_el;
				APPEND_NODE_TO_LIST(row_el->children, row_el->last, col);
			} else
				APPEND_NODE_TO_LIST(ctxt->head, ctxt->tail, col);
		}
		if (row->fields[i].value)
		{
			if (*(row->fields[i].value))
			{
				col->children = col->last = xmlNewDocText(ctxt->parent->doc, NULL);
				col->children->parent = col;
				col->children->content = row->fields[i].needs_copy? BAD_CAST XPL_STRDUP((char*) row->fields[i].value): row->fields[i].value;
			} else if (!row->fields[i].needs_copy)
				XPL_FREE(row->fields[i].value);
		}
	}
	if (row_el)
		APPEND_NODE_TO_LIST(ctxt->head, ctxt->tail, row_el);
	return true;
}

static xmlNodePtr _getNextRTN(xplTdsFragmentRowContextPtr row_ctxt, xplSqlRowTagNamesPtr tag_names)
{
	xmlChar *name;

	if (!tag_names || (tag_names->pos >= tag_names->count))
		return xplCreateErrorNode(row_ctxt->parent, BAD_CAST "query result has more recordsets than tag names specified");
	name = tag_names->names[tag_names->pos++];
	if (name && *name)
	{
		if (xplParseQName(name, row_ctxt->parent, &(row_ctxt->row_qname)) != XPL_PARSE_QNAME_OK)
			return xplCreateErrorNode(row_ctxt->parent, BAD_CAST "invalid row name '%s'", name);
	} else if (row_ctxt->as_attributes)
		return xplCreateErrorNode(row_ctxt->parent, BAD_CAST "can't use empty row name if asattributes is set");
	else {
		row_ctxt->row_qname.ns = NULL;
		row_ctxt->row_qname.ncname = NULL;
	}
	return NULL;
}

static xmlNodePtr _buildFragmentFromTds(
	xefDbContextPtr db_ctxt,
	xplTdsFragmentRowContextPtr row_ctxt,
	xplSqlRowTagNamesPtr tag_names,
	bool *repeat
)
{
	xmlNodePtr error_node;
	xmlChar *error;

	while (xefDbHasRecordset(db_ctxt))
	{
		if ((error_node = _getNextRTN(row_ctxt, tag_names)))
			goto fail;
		// row_ctxt->xml_desc is created on demand in _TdsFragmentRowScanner
		xefDbEnumRows(db_ctxt, _TdsFragmentRowScanner, row_ctxt);
		if (row_ctxt->xml_desc)
		{
			_freeXmlRowDesc(row_ctxt->xml_desc);
			row_ctxt->xml_desc = NULL;
		}
		if ((error = xefDbGetError(db_ctxt)))
		{
			error_node = xplCreateErrorNode(row_ctxt->parent, error);
			goto fail;
		}
		xefDbNextRowset(db_ctxt);
		if ((error = xefDbGetError(db_ctxt)))
		{
			error_node = xplCreateErrorNode(row_ctxt->parent, error);
			goto fail;
		}
		xplClearQName(&row_ctxt->row_qname);
	}
	goto done;
fail:
	*repeat = true;
	xplClearQName(&row_ctxt->row_qname);
	if (row_ctxt->head)
		xmlFreeNodeList(row_ctxt->head);
	row_ctxt->head = row_ctxt->tail = error_node;
done:
	return row_ctxt->head;
}

static xmlNodePtr _buildDocFromMemory(const xmlChar *src, size_t size, xmlNodePtr parent, bool *repeat)
{
	xmlDocPtr doc;
	xmlChar *error = NULL;
	xmlNodePtr ret;

	doc = xmlReadMemory((char*) src, (int) size, NULL, NULL, XML_PARSE_NODICT);
	if (!doc)
	{
		error = xstrGetLastLibxmlError();
		*repeat = true;
		ret = xplCreateErrorNode(parent, BAD_CAST "error parsing input document: \"%s\"", error);
		goto done;
	}
	ret = xplDetachChildren(doc->children);
	xmlSetListDoc(ret, parent->doc);
	xmlFreeDoc(doc);
done:
	if (error)
		XPL_FREE(error);
	return ret;
}

static xmlNodePtr _buildDocFromXmlStream(xefDbContextPtr db_ctxt, xmlNodePtr carrier, bool *repeat)
{
	xmlChar *xml_doc_cur, *xml_doc_start = NULL, *stream_text = NULL;
	size_t stream_size, xml_doc_size;
	xmlChar *error = NULL;
	xmlNodePtr ret;

	stream_text = xefDbAccessStreamData(db_ctxt, &stream_size);
	if ((error = xefDbGetError(db_ctxt)))
	{
		*repeat = true;
		ret = xplCreateErrorNode(carrier, error);
		goto done;
	}
	if (!stream_size)
	{
		*repeat = false;
		ret = NULL;
		goto done;
	}
	xml_doc_size = stream_size + xmlStrlen(DOC_START) + xmlStrlen(DOC_END);
	xml_doc_start = xml_doc_cur = (xmlChar*) XPL_MALLOC(xml_doc_size + 1);
	if (!xml_doc_start)
	{
		*repeat = true;
		ret = xplCreateErrorNode(carrier, BAD_CAST "insufficient memory for resulting XML document");
		goto done;
	}
	strcpy((char*) xml_doc_cur, (char*) DOC_START);
	xml_doc_cur += xmlStrlen(DOC_START);
	memcpy(xml_doc_cur, stream_text, stream_size);
	xml_doc_cur += stream_size;
	strcpy((char*) xml_doc_cur, (char*) DOC_END);
    ret = _buildDocFromMemory(xml_doc_start, xml_doc_size, carrier, repeat);
done:
	if (stream_text)
		xefDbUnaccessStreamData(db_ctxt, stream_text);
	if (xml_doc_start)
		XPL_FREE(xml_doc_start);
	if (error)
		XPL_FREE(error);
	return ret;
}

typedef struct _xplTdsDocRowContext
{
	xmlBufferPtr buf;
	bool out_of_memory;
	bool must_ensure_buf_size;
	xmlNodePtr parent;
	ssize_t row_count;
} xplTdsDocRowContext, *xplTdsDocRowContextPtr;

static bool _TdsDocRowScanner(xefDbRowPtr row, void *payload)
{
	xplTdsDocRowContextPtr ctxt = (xplTdsDocRowContextPtr) payload;
	xmlChar *part;
	size_t buf_size;

	if (!row->field_count)
		return false;
	if (row->fields[0].is_null)
		return false;

	if (ctxt->must_ensure_buf_size)
	{
		if (ctxt->row_count == 1)
			buf_size = row->fields[0].value_size;
		else
			buf_size = ctxt->row_count * (row->fields[0].value_size + 3);
		buf_size += xmlStrlen(DOC_END) + 1; /* DOC_START is already in buffer */
		if (xmlBufferResize(ctxt->buf, buf_size) < 0)
		{
			ctxt->out_of_memory = true;
			return false;
		}
		ctxt->must_ensure_buf_size = false;
	}
	part = row->fields[0].value;
	if (!part || !*part)
		return false;
	if (xmlBufferAdd(ctxt->buf, part, row->fields[0].value_size) < 0)
	{
		ctxt->out_of_memory = true;
		return false;
	}
	if (!row->fields[0].needs_copy) /* free data as we've copied them */
	{
		XPL_FREE(row->fields[0].value);
		row->fields[0].value = NULL;
	}
	return true;
}

static xmlNodePtr _buildDocFromTds(xefDbContextPtr db_ctxt, xplTdsDocRowContextPtr row_ctxt, bool *repeat)
{
	xmlBufferPtr buf;
	xmlNodePtr ret;
	xmlChar *error = NULL;


	if (!(buf = xmlBufferCreateSize(4096)))
		goto oom;
	xmlBufferSetAllocationScheme(buf, row_ctxt->row_count == -1? XML_BUFFER_ALLOC_DOUBLEIT: XML_BUFFER_ALLOC_EXACT);
	if (xmlBufferCat(buf, DOC_START) < 0)
		goto oom;
	row_ctxt->buf = buf;
	row_ctxt->out_of_memory = false;
	row_ctxt->must_ensure_buf_size = (row_ctxt->row_count != -1);
	xefDbEnumRows(db_ctxt, _TdsDocRowScanner, row_ctxt);
	if ((error = xefDbGetError(db_ctxt)))
	{
		*repeat = true;
		ret = xplCreateErrorNode(row_ctxt->parent, error);
		goto done;
	}
	if (row_ctxt->out_of_memory)
		goto oom;
	if (xmlBufferCat(buf, DOC_END) < 0)
		goto oom;
	ret = _buildDocFromMemory(xmlBufferContent(buf), xmlBufferLength(buf), row_ctxt->parent, repeat);
	goto done;
oom:
	ret = xplCreateErrorNode(row_ctxt->parent, BAD_CAST "out of memory");
	*repeat = true;
done:
	if (error)
		XPL_FREE(error);
	if (buf)
		xmlBufferFree(buf);
	return ret;
}

static xmlNodePtr _buildDoc(xefDbContextPtr db_ctxt, xplTdsDocRowContextPtr row_ctxt, bool *repeat)
{
	xefDbStreamType real_stream_type;
	xmlChar *error = NULL;
	xmlNodePtr ret;

	real_stream_type = xefDbGetStreamType(db_ctxt);
	if (real_stream_type == XEF_DB_STREAM_XML)
		return _buildDocFromXmlStream(db_ctxt, row_ctxt->parent, repeat);
	/* no stream support (not ADO backend) */
	row_ctxt->row_count = xefDbGetRowCount(db_ctxt);
	if ((error = xefDbGetError(db_ctxt)))
	{
		*repeat = true;
		ret = xplCreateErrorNode(row_ctxt->parent, error);
		XPL_FREE(error);
	} else if (!row_ctxt->row_count)
		ret = NULL;
	else
		ret = _buildDocFromTds(db_ctxt, row_ctxt, repeat);
	return ret;
}

void xplCmdSqlEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdSqlParamsPtr cmd_params = (xplCmdSqlParamsPtr) commandInfo->params;
	xmlChar *dbname_attr = NULL;
	xplSqlRowTagNamesPtr row_tag_names = NULL;

	xmlNodePtr dbs;
	xplDBListPtr db_list;

	xefDbQueryParams dbq_params;
	xefDbContextPtr db_ctxt = NULL;
	xplTdsFragmentRowContext frag_ctxt;
	xplTdsDocRowContext doc_ctxt;

	dbq_params.error = NULL;
	if (cmd_params->as_attributes && cmd_params->show_nulls)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "asattributes and shownulls can't be used simultaneously"), true, true);
		return;
	}
	dbs = commandInfo->element->parent;
	while (dbs)
	{
		if (!xmlStrcmp(dbs->name, BAD_CAST "dbsession") && xplCheckNodeForXplNs(commandInfo->document, dbs))
			break;
		dbs = dbs->parent;
	}
	if (!dbs)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "dbsession not found"), true, true);
		return;
	}
	dbname_attr = xmlGetNoNsProp(dbs, BAD_CAST "dbname");
	if (!dbname_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "no dbname found in %s:dbsession", dbs->ns? dbs->ns->prefix: BAD_CAST ""), true, true);
		return;
	}
	db_list = xplLocateDBList(dbname_attr);
	if (!db_list)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "unknown database \"%s\" in %s:dbsession", 
			dbname_attr, dbs->ns? dbs->ns->prefix: BAD_CAST ""), true, true);
		goto done;
	}

	memset(&dbq_params, 0, sizeof(dbq_params));
	dbq_params.cleanup_nonprintable = cmd_params->cleanup_stream;
	dbq_params.db_list = db_list;
	dbq_params.query = commandInfo->content;
	dbq_params.desired_stream_type = cmd_params->merge_table_as_xml? XEF_DB_STREAM_XML: XEF_DB_STREAM_TDS;
	db_ctxt = xefDbQuery(&dbq_params);
	if (!db_ctxt || dbq_params.error)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, dbq_params.error? dbq_params.error: BAD_CAST "unknown error"), true, true);
		goto done;
	}
	if (dbq_params.desired_stream_type == XEF_DB_STREAM_XML)
	{
		doc_ctxt.parent = commandInfo->element;
		ASSIGN_RESULT(_buildDoc(db_ctxt, &doc_ctxt, &cmd_params->repeat), cmd_params->repeat, true);
	} else {
		memset(&frag_ctxt, 0, sizeof(frag_ctxt));
		if (cmd_params->tag_name)
			row_tag_names = _createRowTagNames(cmd_params->tag_name);
		frag_ctxt.as_attributes = cmd_params->as_attributes;
		frag_ctxt.default_column_qname = cmd_params->default_column_name;
		frag_ctxt.keep_nulls = cmd_params->keep_nulls;
		frag_ctxt.show_nulls = cmd_params->show_nulls;
		frag_ctxt.head = frag_ctxt.tail = NULL;
		frag_ctxt.parent = commandInfo->element;
		ASSIGN_RESULT(_buildFragmentFromTds(db_ctxt, &frag_ctxt, row_tag_names, &cmd_params->repeat), cmd_params->repeat, true);
	}
done:
	if (dbq_params.error)
		XPL_FREE(dbq_params.error);
	if (db_ctxt)
		xefDbFreeContext(db_ctxt);
	if (dbname_attr)
		XPL_FREE(dbname_attr);
	if (row_tag_names)
		_freeRowTagNames(row_tag_names);
}
#endif
