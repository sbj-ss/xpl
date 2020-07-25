#include <string.h>
#include <libxpl/abstraction/xef.h>
#include <libxpl/xplbuffer.h>
#include <libxpl/xplcore.h>
#include <libxpl/xpldb.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>
#include "commands/SQL.h"

#define DEFAULT_RESPONSE_TAG_NAME (BAD_CAST "Row")
#define DEFAULT_COLUMN_NAME (BAD_CAST "Col")
#define NULL_ATTRIBUTE_NAME (BAD_CAST "isnull")
#define DOC_START "<?xml version=\"1.0\" encoding=\"UTF-8\"?><Root>"
#define DOC_END "</Root>"

void xplCmdSqlPrologue(xplCommandInfoPtr commandInfo)
{
}

#ifndef _XEF_HAS_DB
void xplCmdSqlEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "No database support is compiled in"), true, true);
}
#else
static xmlNodePtr _getNonTextQueryError(xmlNodePtr queryParent)
{
	xmlNodePtr cur = queryParent->children;
	while (cur)
	{
		if (cur->type == XML_TEXT_NODE)
			break;
		cur = cur->next;
	}
	if (cur)
		return xplCreateErrorNode(queryParent, BAD_CAST "query \"%s...\" is non-text", cur->content);
	cur = queryParent->children;
	while (cur)
	{
		if (cur->type == XML_ELEMENT_NODE)
			break;
		cur = cur->next;
	}
	if (cur)
		return xplCreateErrorNode(queryParent, BAD_CAST "query \"<%s...\" is non-text", cur->name);
	return xplCreateErrorNode(queryParent, BAD_CAST "query is non-text");
}

typedef struct _xplSqlRowTagNames
{
	size_t count;
	size_t pos;
	xmlChar **names;
} xplSqlRowTagNames, *xplSqlRowTagNamesPtr;

typedef enum _xplSqlTokenizerState {
	XS_TS_INITIAL,
	XS_TS_DELIMITER,
	XS_TS_WHITESPACE,
	XS_TS_VALUE,
	XS_TS_END
} xplSqlTokenizerState;

static void _freeRowTagNames(xplSqlRowTagNamesPtr names)
{
	size_t i;

	if (!names)
		return;
	if (names->names)
	{
		for (i = 0; i < names->count; i++)
			if(names->names[i])
				xmlFree(names->names[i]);
		xmlFree(names->names);
	}
	xmlFree(names);
}

static xplSqlRowTagNamesPtr _createRowTagNames(xmlChar *list)
{
	xplSqlTokenizerState state = XS_TS_INITIAL;
	xmlChar *value_start = NULL, *p;
	xplSqlRowTagNamesPtr ret = (xplSqlRowTagNamesPtr) xmlMalloc(sizeof(xplSqlRowTagNames));

	if (!ret)
		return NULL;
	ret->count = 1;
	ret->pos = 0;
	p = list;
	if (p)
	{
		while (*p)
		{
			if (*p == ',')
				ret->count++;
			p++;
		}
	}
	ret->names = (xmlChar **) xmlMalloc(ret->count * sizeof(xmlChar*));
	if (!ret->names)
	{
		xmlFree(ret);
		return NULL;
	}
	if (!list)
	{
		ret->names[0] = DEFAULT_RESPONSE_TAG_NAME;
		return ret;
	}
	while (state != XS_TS_END)
	{
		switch (*list)
		{
		case ' ': 
			state = XS_TS_WHITESPACE;
			break;
		case ',':
			state = XS_TS_DELIMITER;
			break;
		case 0:
			state = XS_TS_END;
			break;
		default:
			state = XS_TS_VALUE;
		}
		switch (state)
		{
		case XS_TS_WHITESPACE:
			break;
		case XS_TS_DELIMITER:
		case XS_TS_END:
			if (value_start)
				ret->names[ret->pos] = xmlStrndup(value_start, (int) (list - value_start));
			else
				ret->names[ret->pos] = xmlStrdup(BAD_CAST "");
			ret->pos++;
			value_start = NULL;
			break;
		case XS_TS_VALUE:
			if (!value_start)
				value_start = list;
			break;
		default:
			DISPLAY_INTERNAL_ERROR_MESSAGE();
		}
		list++;
	}
	ret->pos = 0;
	return ret;
}

typedef struct _xplSqlXmlRowDesc {
	size_t count;
	xmlChar **names;
	xmlNsPtr *namespaces;
} xplSqlXmlRowDesc, *xplSqlXmlRowDescPtr;

static void _freeXmlRowDesc(xplSqlXmlRowDescPtr desc)
{
	if (!desc)
		return;
	if (desc->names)
		xmlFree(desc->names);
	if (desc->namespaces)
		xmlFree(desc->namespaces);
	xmlFree(desc);
}

static xplSqlXmlRowDescPtr _createXmlRowDesc(xefDbRowPtr row, xmlNodePtr parent, xmlChar* defaultColumnName)
{
	size_t i;
	xplSqlXmlRowDescPtr ret = (xplSqlXmlRowDescPtr) xmlMalloc(sizeof(xplSqlXmlRowDesc));
	xmlChar *field_name;

	if (!ret)
		return NULL;
	ret->names = (xmlChar**) xmlMalloc(row->field_count * sizeof(xmlChar*));
	if (!ret->names)
	{
		xmlFree(ret);
		return NULL;
	}
	ret->namespaces = (xmlNsPtr*) xmlMalloc(row->field_count * sizeof(xmlNsPtr));
	if (!ret->namespaces)
	{
		xmlFree(ret->names);
		xmlFree(ret);
		return NULL;
	}
	ret->count = row->field_count;
	memset(ret->names, 0, ret->count * sizeof(xmlChar*));
	memset(ret->namespaces, 0, ret->count * sizeof(xmlNsPtr*));
	for (i = 0; i < ret->count; i++)
	{
		field_name = row->fields[i].name;
		if (field_name && *field_name)
		{
			EXTRACT_NS_AND_TAGNAME(field_name, ret->namespaces[i], ret->names[i], parent);
		} else {
			EXTRACT_NS_AND_TAGNAME(defaultColumnName, ret->namespaces[i], ret->names[i], parent);
		}
	}
	return ret;
}

typedef struct _xplTdsFragmentRowContext
{
	xmlNodePtr first;
	xmlNodePtr cur;
	xmlNodePtr parent;
	xplSqlXmlRowDescPtr xml_desc;
	xmlNsPtr ns;
	xmlChar *tagname;
	xmlChar *default_column_name;
	bool as_attributes;
	bool keep_nulls;
	bool show_nulls;
	bool copy_data;
} xplTdsFragmentRowContext, *xplTdsFragmentRowContextPtr;

static bool _TdsFragmentRowScanner(xefDbRowPtr row, void *payload)
{
	xmlNodePtr row_el = NULL, tail = NULL, col;
	xplTdsFragmentRowContextPtr ctxt = (xplTdsFragmentRowContextPtr) payload;
	size_t i;

	if (!ctxt->xml_desc)
		ctxt->xml_desc = _createXmlRowDesc(row, ctxt->parent, ctxt->default_column_name);
	if (ctxt->tagname && *ctxt->tagname)
		row_el = xmlNewDocNode(ctxt->parent->doc, ctxt->ns, ctxt->tagname, NULL);
	for (i = 0; i < row->field_count; i++)
	{
		if (row->fields[i].is_null && !ctxt->keep_nulls)
			continue;
		if (ctxt->as_attributes) 
			col = (xmlNodePtr) xmlNewNsProp(row_el, ctxt->xml_desc->namespaces[i], ctxt->xml_desc->names[i], NULL);
		else {
			col = xmlNewDocNode(ctxt->parent->doc, ctxt->xml_desc->namespaces[i], ctxt->xml_desc->names[i], NULL);
			if (row->fields[i].is_null && ctxt->show_nulls)
				xmlNewProp(col, NULL_ATTRIBUTE_NAME, BAD_CAST "true");
			if (ctxt->tagname && *ctxt->tagname)
			{
				col->parent = row_el;
				if (row_el->last)
				{
					row_el->last->next = col;
					col->prev = row_el->last;
					row_el->last = col;
				} else
					row_el->children = row_el->last = col;
			} else {
				if (tail)
				{
					tail->next = col;
					col->prev = tail;
					tail = col;
				} else
					row_el = tail = col;
			}
		}
		if (row->fields[i].value)
		{
			if (*(row->fields[i].value))
			{
				col->children = col->last = xmlNewDocText(ctxt->parent->doc, NULL);
				col->children->parent = col;
				col->children->content = row->fields[i].needs_copy? xmlStrdup(row->fields[i].value): row->fields[i].value;
			} else if (!row->fields[i].needs_copy)
				xmlFree(row->fields[i].value);
		}
	}
	if (row_el)
	{
		if (ctxt->cur)
		{
			ctxt->cur->next = row_el;
			row_el->prev = ctxt->cur;
		} else 
			ctxt->first = row_el;
		ctxt->cur = tail? tail: row_el;
	}
	return true;
}

static xmlNodePtr _buildFragmentFromTds(
	xefDbContextPtr db_ctxt,
	xplTdsFragmentRowContextPtr row_ctxt,
	xplSqlRowTagNamesPtr tag_names,
	bool *repeat
)
{
	xefErrorMessagePtr error;
	xmlChar *error_text;

	while (xefDbHasRecordset(db_ctxt))
	{
		if (tag_names && (tag_names->pos < tag_names->count))
		{
			EXTRACT_NS_AND_TAGNAME(tag_names->names[tag_names->pos], row_ctxt->ns, row_ctxt->tagname, row_ctxt->parent);
			if (row_ctxt->as_attributes && !row_ctxt->tagname)
			{
				EXTRACT_NS_AND_TAGNAME(DEFAULT_RESPONSE_TAG_NAME, row_ctxt->ns, row_ctxt->tagname, row_ctxt->parent);
			}
			tag_names->pos++;
		} else {
			EXTRACT_NS_AND_TAGNAME(DEFAULT_RESPONSE_TAG_NAME, row_ctxt->ns, row_ctxt->tagname, row_ctxt->parent);
		}
		xefDbEnumRows(db_ctxt, _TdsFragmentRowScanner, row_ctxt);
		_freeXmlRowDesc(row_ctxt->xml_desc);
		row_ctxt->xml_desc = NULL;
		if ((error = xefDbGetError(db_ctxt)))
		{
			error_text = xefGetErrorText(error);
			if (row_ctxt->first)
				xmlFreeNodeList(row_ctxt->first);
			*repeat = true;
			return xplCreateErrorNode(row_ctxt->parent, error_text);
		}
		xefDbNextRowset(db_ctxt);
		if ((error = xefDbGetError(db_ctxt)))
		{
			error_text = xefGetErrorText(error);
			if (row_ctxt->first)
				xmlFreeNodeList(row_ctxt->first);
			*repeat = true;
			return xplCreateErrorNode(row_ctxt->parent, error_text);
		}
	}
	return row_ctxt->first;
}

static xmlNodePtr _buildDocFromMemory(xmlChar *src, size_t size, xmlNodePtr parent, bool *repeat)
{
	xmlDocPtr doc;
	xmlChar *error_text;
	xmlNodePtr ret;

	doc = xmlReadMemory(src, (int) size, NULL, NULL, XML_PARSE_NODICT);
	if (!doc)
	{
		error_text = getLastLibxmlError();
		*repeat = true;
		return xplCreateErrorNode(parent, BAD_CAST "error parsing input document: \"%s\"", error_text);
	}
	ret = detachContent(doc->children);
	xmlSetListDoc(ret, parent->doc);
	xmlFreeDoc(doc);
	return ret;
}

static xmlNodePtr _buildDocFromXmlStream(xefDbContextPtr db_ctxt, xmlNodePtr carrier, bool *repeat)
{
	xmlChar *xml_doc_cur, *xml_doc_start = NULL, *stream_text;
	size_t stream_size, xml_doc_size;
	xefErrorMessagePtr error;
	xmlChar *error_text = NULL;
	xmlNodePtr ret;

	stream_text = xefDbAccessStreamData(db_ctxt, &stream_size);
	if ((error = xefDbGetError(db_ctxt)))
	{
		error_text = xefGetErrorText(error);
		*repeat = true;
		return xplCreateErrorNode(carrier, error_text);
	}
	if (!stream_size)
	{
		*repeat = false;
		ret = NULL;
		goto done;
	}
	xml_doc_size = stream_size + xmlStrlen(DOC_START) + xmlStrlen(DOC_END);
	xml_doc_start = xml_doc_cur = (xmlChar*) xmlMalloc(xml_doc_size + 1);
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
	xefDbUnaccessStreamData(db_ctxt, stream_text);
	if (xml_doc_start)
		xmlFree(xml_doc_start);
	return ret;
}

typedef struct _xplTdsDocRowContext
{
	rbBufPtr buf;
	bool out_of_memory;
	xmlNodePtr parent;
} xplTdsDocRowContext, *xplTdsDocRowContextPtr;

static bool _TdsDocRowScanner(xefDbRowPtr row, void *payload)
{
	xplTdsDocRowContextPtr ctxt = (xplTdsDocRowContextPtr) payload;
	xmlChar *part;

	if (!row->field_count)
		return false;
	if (row->fields[0].is_null)
		return false;
	part = row->fields[0].value;
	if (!part || !*part)
		return false;
	if (rbAddDataToBuf(ctxt->buf, part, row->fields[0].value_size) != RB_RESULT_OK)
	{
		ctxt->out_of_memory = true;
		return false;
	}
	if (!row->fields[0].needs_copy) /* free data as we've copied them */
	{
		xmlFree(row->fields[0].value);
		row->fields[0].value = NULL;
	}
	return true;
}

static xmlNodePtr _buildDocFromUnknownSizeTds(xefDbContextPtr db_ctxt, xplTdsDocRowContextPtr row_ctxt,	bool *repeat)
{
	rbBufPtr buf;
	xmlNodePtr ret;
	xefErrorMessagePtr error;
	xmlChar *error_text = NULL;

	buf = rbCreateBufParams(4096, RB_GROW_DOUBLE, 0);
	if (rbAddDataToBuf(buf, DOC_START, xmlStrlen(DOC_START)) != RB_RESULT_OK)
		goto oom;
	row_ctxt->buf = buf;
	row_ctxt->out_of_memory = false;
	xefDbEnumRows(db_ctxt, _TdsDocRowScanner, row_ctxt);
	if ((error = xefDbGetError(db_ctxt)))
	{
		error_text = xefGetErrorText(error);
		*repeat = true;
		ret = xplCreateErrorNode(row_ctxt->parent, error_text);
		goto done;
	}
	if (row_ctxt->out_of_memory)
		goto oom;
	if (rbAddDataToBuf(buf, DOC_END, xmlStrlen(DOC_END) + 1) != RB_RESULT_OK)
		goto oom;
	ret = _buildDocFromMemory(rbGetBufContent(buf), rbGetBufContentSize(buf), row_ctxt->parent, repeat);
	goto done;
oom:
	ret = xplCreateErrorNode(row_ctxt->parent, BAD_CAST "out of memory");
	*repeat = true;
done:
	if (error_text)
		xmlFree(error_text);
	rbFreeBuf(buf);
	return ret;
}

static xmlNodePtr _buildDocFromKnownSizeTds(xefDbContextPtr db_ctxt, xplTdsDocRowContextPtr row_ctxt, bool *repeat)
{
	DISPLAY_INTERNAL_ERROR_MESSAGE();
	return _buildDocFromUnknownSizeTds(db_ctxt, row_ctxt, repeat); // TODO write more effective code for this case
}

static xmlNodePtr _buildDoc(xefDbContextPtr db_ctxt, xplTdsDocRowContextPtr row_ctxt, bool *repeat)
{
	xefDbStreamType real_stream_type;
	ssize_t row_count;
	xefErrorMessagePtr error;
	xmlChar *error_text = NULL;

	real_stream_type = xefDbGetStreamType(db_ctxt);
	if (real_stream_type == XEF_DB_STREAM_XML)
		return _buildDocFromXmlStream(db_ctxt, row_ctxt->parent, repeat);
	/* no stream support (not ADO backend) */
	row_count = xefDbGetRowCount(db_ctxt);
	if ((error = xefDbGetError(db_ctxt)))
	{
		error_text = xefGetErrorText(error);
		*repeat = true;
		return xplCreateErrorNode(row_ctxt->parent, error_text);
	}
	if (row_count == -1) /* driver doesn't know */
		return _buildDocFromUnknownSizeTds(db_ctxt, row_ctxt, repeat);
	else if (!row_count)
		return NULL;
	else
		return _buildDocFromKnownSizeTds(db_ctxt, row_ctxt, repeat);
}

void xplCmdSqlEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define RESPONSE_TAG_NAME_ATTR (BAD_CAST "responsetagname")
#define MERGE_TABLE_AS_XML_ATTR (BAD_CAST "mergetableasxml")
#define REPEAT_ATTR (BAD_CAST "repeat")
#define CLEANUPSTREAM_ATTR (BAD_CAST "cleanupstream")
#define ASATTRIBUTES_ATTR (BAD_CAST "asattributes")
#define KEEPNULLS_ATTR (BAD_CAST "keepnulls")
#define DEFAULT_COLUMN_NAME_ATTR (BAD_CAST "defaultcolumnname")
#define SHOWNULLS_ATTR (BAD_CAST "shownulls")

	xmlChar *response_tag_name_attr = NULL;
	xmlChar *dbname_attr = NULL;
	xmlChar *default_column_name_attr = NULL;
	bool as_xml;
	bool repeat;
	bool cleanup_stream;
	bool keep_nulls;
	bool as_attributes;
	bool show_nulls;
	xmlNodePtr attr_error;
	xmlChar *sql = NULL;
	xplSqlRowTagNamesPtr row_tag_names = NULL;

	xmlNodePtr dbs;
	xplDBListPtr db_list;
	xplDBPtr db = NULL;

	xefDbQueryParams params;
	xefDbContextPtr db_ctxt = NULL;
	xplTdsFragmentRowContext frag_ctxt;
	xplTdsDocRowContext doc_ctxt;
	xmlChar *error_text = NULL;

	if ((attr_error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, true)))
	{
		ASSIGN_RESULT(attr_error, true, true);
		return;
	}
	if ((attr_error = xplDecodeCmdBoolParam(commandInfo->element, CLEANUPSTREAM_ATTR, &cleanup_stream, false)))
	{
		ASSIGN_RESULT(attr_error, true, true);
		return;
	}
	if ((attr_error = xplDecodeCmdBoolParam(commandInfo->element, MERGE_TABLE_AS_XML_ATTR, &as_xml, false)))
	{
		ASSIGN_RESULT(attr_error, true, true);
		return;
	}
	if ((attr_error = xplDecodeCmdBoolParam(commandInfo->element, ASATTRIBUTES_ATTR, &as_attributes, false)))
	{
		ASSIGN_RESULT(attr_error, true, true);
		return;
	}
	if ((attr_error = xplDecodeCmdBoolParam(commandInfo->element, KEEPNULLS_ATTR, &keep_nulls, false)))
	{
		ASSIGN_RESULT(attr_error, true, true);
		return;
	}
	if ((attr_error = xplDecodeCmdBoolParam(commandInfo->element, SHOWNULLS_ATTR, &show_nulls, false)))
	{
		ASSIGN_RESULT(attr_error, true, true);
		return;
	}

	response_tag_name_attr = xmlGetNoNsProp(commandInfo->element, RESPONSE_TAG_NAME_ATTR);
	default_column_name_attr = xmlGetNoNsProp(commandInfo->element, DEFAULT_COLUMN_NAME_ATTR);

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
		goto done;
	}
	dbname_attr = xmlGetNoNsProp(dbs, BAD_CAST "dbname");
	if (!dbname_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "no dbname found in %s:dbsession", dbs->ns? dbs->ns->prefix: BAD_CAST ""), true, true);
		goto done;
	}
	db_list = xplLocateDBList(dbname_attr);
	if (!db_list)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "unknown database \"%s\" in %s:dbsession", 
			dbname_attr, dbs->ns? dbs->ns->prefix: BAD_CAST ""), true, true);
		goto done;
	}
	if (!checkNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(_getNonTextQueryError(commandInfo->element), true, true);
		goto done;
	}
	sql = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, 1);
	if (!sql || !*sql)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "query is empty"), true, true);
		goto done;
	}

	params.cleanup_nonprintable = cleanup_stream;
	params.db_list = db_list;
	params.query = sql;
	params.desired_stream_type = as_xml? XEF_DB_STREAM_XML: XEF_DB_STREAM_TDS;
	params.error = NULL;
	db_ctxt = xefDbQuery(&params);
	if (!db_ctxt || params.error)
	{
		error_text = xefGetErrorText(params.error);
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, error_text), true, true);
		xefFreeErrorMessage(params.error);
		goto done;
	}
	if (params.desired_stream_type == XEF_DB_STREAM_XML)
	{
		doc_ctxt.parent = commandInfo->element;
		ASSIGN_RESULT(_buildDoc(db_ctxt, &doc_ctxt, &repeat), repeat, true);
	} else {
		if (response_tag_name_attr)
			row_tag_names = _createRowTagNames(response_tag_name_attr);
		frag_ctxt.as_attributes = as_attributes;
		frag_ctxt.keep_nulls = keep_nulls;
		frag_ctxt.show_nulls = show_nulls;
		frag_ctxt.first = frag_ctxt.cur = NULL;
		frag_ctxt.parent = commandInfo->element;
		frag_ctxt.default_column_name = default_column_name_attr? default_column_name_attr: DEFAULT_COLUMN_NAME;
		frag_ctxt.xml_desc = NULL;
		ASSIGN_RESULT(_buildFragmentFromTds(db_ctxt, &frag_ctxt, row_tag_names, &repeat), repeat, true);
	}
done:
	if (error_text)
		xmlFree(error_text);
	if (db_ctxt)
		xefDbFreeContext(db_ctxt);
	if (dbname_attr)
		xmlFree(dbname_attr);
	if (sql)
		xmlFree(sql);
	if (row_tag_names)
		_freeRowTagNames(row_tag_names);
	if (response_tag_name_attr)
		xmlFree(response_tag_name_attr);
	if (default_column_name_attr)
		xmlFree(default_column_name_attr);
}
#endif

xplCommand xplSqlCommand = { xplCmdSqlPrologue, xplCmdSqlEpilogue };
