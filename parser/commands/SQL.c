#include "commands/SQL.h"
#include "Core.h"
#include "DB.h"
#include "Messages.h"
#include "Utils.h"
#include "abstraction/ExtFeatures.h"

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
	ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "No database support is compiled in"), TRUE, TRUE);
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

typedef struct _xplSqlRowContext 
{
	xmlNodePtr first;
	xmlNodePtr cur;
	xmlNodePtr parent;
	xplSqlXmlRowDescPtr xml_desc;
	xmlNsPtr ns;
	xmlChar *tagname;
	xmlChar *default_column_name;
	BOOL as_attributes;
	BOOL keep_nulls;
	BOOL show_nulls;
	BOOL copy_data;
} xplSqlRowContext, *xplSqlRowContextPtr;

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

static xplSqlXmlRowDescPtr _createXmlRowDesc(xefDbRowDescPtr desc, xmlNodePtr parent, xmlChar* defaultColumnName)
{
	size_t i;
	xplSqlXmlRowDescPtr ret = (xplSqlXmlRowDescPtr) xmlMalloc(sizeof(xplSqlXmlRowDesc));
	if (!ret)
		return NULL;
	ret->names = (xmlChar **) xmlMalloc(desc->count * sizeof(xmlChar*));
	if (!ret->names)
	{
		xmlFree(ret);
		return NULL;
	}
	ret->namespaces = (xmlNsPtr *) xmlMalloc(desc->count * sizeof(xmlNsPtr));
	if (!ret->namespaces)
	{
		xmlFree(ret->names);
		xmlFree(ret);
		return NULL;
	}
	ret->count = desc->count;
	memset(ret->names, 0, ret->count * sizeof(xmlChar*));
	memset(ret->namespaces, 0, ret->count * sizeof(xmlNsPtr*));
	for (i = 0; i < ret->count; i++)
	{
		if (desc->names[i] && *(desc->names[i]))
		{
			EXTRACT_NS_AND_TAGNAME(desc->names[i], ret->namespaces[i], ret->names[i], parent)
		} else
			EXTRACT_NS_AND_TAGNAME(defaultColumnName, ret->namespaces[i], ret->names[i], parent)
	}
	return ret;
}

static BOOL _RowScanner(xefDbRowDescPtr desc, xefDbRowPtr row, void *payload)
{
	xmlNodePtr row_el = NULL, tail = NULL, col;
	xplSqlRowContextPtr ctxt = (xplSqlRowContextPtr) payload;
	size_t i;

	if (!ctxt->xml_desc)
		ctxt->xml_desc = _createXmlRowDesc(desc, ctxt->parent, ctxt->default_column_name);
	if (ctxt->tagname && *ctxt->tagname)
		row_el = xmlNewDocNode(ctxt->parent->doc, ctxt->ns, ctxt->tagname, NULL);
	for (i = 0; i < desc->count; i++)
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
			} else
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
	return TRUE;
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
	BOOL as_xml;
	BOOL repeat;
	BOOL cleanup_stream;
	BOOL keep_nulls;
	BOOL as_attributes;
	BOOL show_nulls;
	xmlNodePtr attr_error;
	xmlChar *sql = NULL;
	xplSqlRowTagNamesPtr tag_names = NULL;

	xmlNodePtr dbs;
	xplDBListPtr db_list;
	xplDBPtr db = NULL;

	xefDbQueryParams params;
	xefDbContextPtr db_ctxt = NULL;
	xplSqlRowContext row_ctxt;
	xefErrorMessagePtr error;
	xmlChar *error_text = NULL, *xml_doc_cur, *xml_doc_start = NULL, *stream_text;
	size_t stream_size, xml_doc_size;
	xmlDocPtr xml_doc;

	if ((attr_error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, TRUE)))
	{
		ASSIGN_RESULT(attr_error, TRUE, TRUE);
		return;
	}
	if ((attr_error = xplDecodeCmdBoolParam(commandInfo->element, CLEANUPSTREAM_ATTR, &cleanup_stream, FALSE)))
	{
		ASSIGN_RESULT(attr_error, TRUE, TRUE);
		return;
	}
	if ((attr_error = xplDecodeCmdBoolParam(commandInfo->element, MERGE_TABLE_AS_XML_ATTR, &as_xml, FALSE)))
	{
		ASSIGN_RESULT(attr_error, TRUE, TRUE);
		return;
	}
	if ((attr_error = xplDecodeCmdBoolParam(commandInfo->element, ASATTRIBUTES_ATTR, &as_attributes, FALSE)))
	{
		ASSIGN_RESULT(attr_error, TRUE, TRUE);
		return;
	}
	if ((attr_error = xplDecodeCmdBoolParam(commandInfo->element, KEEPNULLS_ATTR, &keep_nulls, FALSE)))
	{
		ASSIGN_RESULT(attr_error, TRUE, TRUE);
		return;
	}
	if ((attr_error = xplDecodeCmdBoolParam(commandInfo->element, SHOWNULLS_ATTR, &show_nulls, FALSE)))
	{
		ASSIGN_RESULT(attr_error, TRUE, TRUE);
		return;
	}

	response_tag_name_attr = xmlGetNoNsProp(commandInfo->element, RESPONSE_TAG_NAME_ATTR);
	if (response_tag_name_attr)
		tag_names = _createRowTagNames(response_tag_name_attr);
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
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "dbsession not found"), TRUE, TRUE);
		goto done;
	}
	dbname_attr = xmlGetNoNsProp(dbs, BAD_CAST "dbname");
	if (!dbname_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "no dbname found in %s:dbsession", dbs->ns? dbs->ns->prefix: BAD_CAST ""), TRUE, TRUE);
		goto done;
	}
	db_list = xplLocateDBList(dbname_attr);
	if (!db_list)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "unknown database \"%s\" in %s:dbsession", 
			dbname_attr, dbs->ns? dbs->ns->prefix: BAD_CAST ""), TRUE, TRUE);
		goto done;
	}
	if (!checkNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(_getNonTextQueryError(commandInfo->element), TRUE, TRUE);
		goto done;
	}
	sql = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, 1);
	if (!sql || !*sql)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "query is empty"), TRUE, TRUE);
		goto done;
	}
	result->list = NULL;

	row_ctxt.as_attributes = as_attributes;
	row_ctxt.keep_nulls = keep_nulls;
	row_ctxt.show_nulls = show_nulls;
	row_ctxt.first = row_ctxt.cur = NULL;
	row_ctxt.parent = commandInfo->element;
	row_ctxt.default_column_name = default_column_name_attr? default_column_name_attr: DEFAULT_COLUMN_NAME;
	row_ctxt.xml_desc = NULL;

	params.cleanup_nonprintable = cleanup_stream;
	params.db_list = db_list;
	params.query = sql;
	params.stream_type = as_xml? XEF_DB_STREAM_XML: XEF_DB_STREAM_TDS;
	params.error = NULL;
	params.user_data = &row_ctxt;
	db_ctxt = xefDbQuery(&params);
	if (!db_ctxt || params.error)
	{
		error_text = xefGetErrorText(params.error);
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, error_text), TRUE, TRUE);
		xefFreeErrorMessage(params.error);
		goto done;
	}
	if (params.stream_type == XEF_DB_STREAM_XML)
	{
		stream_text = xefDbAccessStreamData(db_ctxt, &stream_size);
		if ((error = xefDbGetError(db_ctxt)))
		{
			error_text = xefGetErrorText(error);
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, error_text), TRUE, TRUE);
				goto done;
		}
		if (!stream_size)
		{
			ASSIGN_RESULT(NULL, FALSE, TRUE);
			goto done;
		}
		xml_doc_size = stream_size + xmlStrlen(DOC_START) + xmlStrlen(DOC_END);
		xml_doc_start = xml_doc_cur = (xmlChar*) xmlMalloc(xml_doc_size + 1);
		if (!xml_doc_start)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "insufficient memory for resulting XML document"), TRUE, TRUE);
			xefDbUnaccessStreamData(db_ctxt, stream_text);
			goto done;
		}
		strcpy((char*) xml_doc_cur, (char*) DOC_START);
		xml_doc_cur += xmlStrlen(DOC_START);
		memcpy(xml_doc_cur, stream_text, stream_size);
		xml_doc_cur += stream_size;
		strcpy((char*) xml_doc_cur, (char*) DOC_END);
		xefDbUnaccessStreamData(db_ctxt, stream_text);
		if ((error = xefDbGetError(db_ctxt)))
		{
			error_text = xefGetErrorText(error);
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, error_text), TRUE, TRUE);
			goto done;
		}
		xml_doc = xmlReadMemory(xml_doc_start, (int) xml_doc_size, NULL, NULL, XML_PARSE_NODICT);
		if (!xml_doc)
		{
			error_text = getLastLibxmlError();
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "error parsing input document: \"%s\"", error_text), TRUE, TRUE);
			goto done;
		}
		row_ctxt.first = detachContent(xml_doc->children);
		xmlSetListDoc(row_ctxt.first, commandInfo->element->doc);
		xmlFreeDoc(xml_doc);
		ASSIGN_RESULT(row_ctxt.first, repeat, TRUE);
		goto done;
	}

	while (xefDbHasRecordset(db_ctxt))
	{
		if (tag_names && (tag_names->pos < tag_names->count))
		{
			EXTRACT_NS_AND_TAGNAME(tag_names->names[tag_names->pos], row_ctxt.ns, row_ctxt.tagname, commandInfo->element)
			if (as_attributes && !row_ctxt.tagname)
			{
				EXTRACT_NS_AND_TAGNAME(DEFAULT_RESPONSE_TAG_NAME, row_ctxt.ns, row_ctxt.tagname, commandInfo->element);
			}
			tag_names->pos++;						
		} else {
			EXTRACT_NS_AND_TAGNAME(DEFAULT_RESPONSE_TAG_NAME, row_ctxt.ns, row_ctxt.tagname, commandInfo->element);
		}
		xefDbEnumRows(db_ctxt, _RowScanner);
		_freeXmlRowDesc(row_ctxt.xml_desc);
		row_ctxt.xml_desc = NULL;
		if ((error = xefDbGetError(db_ctxt)))
		{
			error_text = xefGetErrorText(error);
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, error_text), TRUE, TRUE);
			if (row_ctxt.first)
				xmlFreeNodeList(row_ctxt.first);
			goto done;
		}
		xefDbNextRowset(db_ctxt);
		if ((error = xefDbGetError(db_ctxt)))
		{
			error_text = xefGetErrorText(error);
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, error_text), TRUE, TRUE);
			if (row_ctxt.first)
				xmlFreeNodeList(row_ctxt.first);
			goto done;
		}
	}
	ASSIGN_RESULT(row_ctxt.first, repeat, TRUE);
done:
	if (error_text)
		xmlFree(error_text);
	if (db_ctxt)
		xefDbFreeContext(db_ctxt);
	if (dbname_attr)
		xmlFree(dbname_attr);
	if (sql)
		xmlFree(sql);
	if (xml_doc_start)
		xmlFree(xml_doc_start);
	if (tag_names)
		_freeRowTagNames(tag_names);
	if (response_tag_name_attr)
		xmlFree(response_tag_name_attr);
	if (default_column_name_attr)
		xmlFree(default_column_name_attr);
}
#endif

xplCommand xplSqlCommand = { xplCmdSqlPrologue, xplCmdSqlEpilogue };
