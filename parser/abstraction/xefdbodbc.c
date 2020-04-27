#include <libxpl/abstraction/xef.h>
#include <libxpl/abstraction/xefinternal.h>
#include <libxpl/xpldb.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplutils.h>
#include <sql.h>
#include <sqlext.h>

/*============== basic XEF stuff ===============*/
SQLHANDLE hEnv = NULL;

XEF_STARTUP_PROTO(Database)
{
	if (hEnv)
		return true;
	if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv)))
	{
		hEnv = NULL;
		return false;
	}
	if (SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, 0) != SQL_SUCCESS)
	{
		SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		hEnv = NULL;
		return false;
	}
	return true;
}

XEF_SHUTDOWN_PROTO(Database)
{
	if (!hEnv)
		return;
	SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
	hEnv = NULL;
}

/* this isn't used now */
XEF_GET_ERROR_TEXT_PROTO(Database)
{
	DISPLAY_INTERNAL_ERROR_MESSAGE();
	return NULL;
}

XEF_FREE_ERROR_MESSAGE_PROTO(Database)
{
	DISPLAY_INTERNAL_ERROR_MESSAGE();
}

/* =============== errors ================= */

static xmlChar* _xefDbDecodeOdbcError(SQLHANDLE handle, SQLSMALLINT handleType, RETCODE RetCode)
{
	SQLSMALLINT rec_no = 1, msg_len;
	SQLINTEGER error;
	SQLRETURN r;
	SQLWCHAR *msg = NULL, state[SQL_SQLSTATE_SIZE+1];
	xmlChar *conv, *ret = NULL;

	if (RetCode == SQL_INVALID_HANDLE)
		return xmlStrdup(BAD_CAST "invalid handle");

	while (1)
	{
		msg = NULL;
		r = SQLGetDiagRecW(handleType, handle, rec_no, state, &error, msg, 0, &msg_len);
		if (r == SQL_NO_DATA)
			return ret? ret: xmlStrdup(BAD_CAST "no information available");
		if (SQL_SUCCEEDED(r))
		{
			msg = (SQLWCHAR*) xmlRealloc(msg, (msg_len+1)*sizeof(SQLWCHAR));
			if (!msg)
			{
				if (ret)
					xmlFree(ret);
				return xplFormatMessage(BAD_CAST "%s(): SQLGetDiagRecW(): insufficient memory", __FUNCTION__);
			}
			r = SQLGetDiagRecW(handleType, handle, rec_no, state, &error, msg, msg_len+1, &msg_len);
		}
		if (!SQL_SUCCEEDED(r))
		{
			if (msg)
				xmlFree(msg);
			if (ret)
				return xplFormatMessage(BAD_CAST "%s, %s(): SQLGetDiagRecW() failed", ret, __FUNCTION__);
			return xplFormatMessage(BAD_CAST "%s(): SQLGetDiagRecW() failed", __FUNCTION__);
		}
		rec_no++;
		conv = NULL;
		iconv_string("utf-8", "utf-16le", (const char*) msg, (const char*) (msg + msg_len), (char**) &conv, NULL);
		ret = ret? xmlStrcat(xmlStrcat(ret, BAD_CAST ", "), conv): conv;
		if (ret != conv)
			xmlFree(conv);
	}
}

/* ================= low-level interface ================== */
void xefDbDeallocateDb(void *db_handle)
{
	if (db_handle)
	{
		SQLHDBC hConn = (SQLHDBC) db_handle;
		SQLDisconnect(hConn);
		SQLFreeHandle(SQL_HANDLE_DBC, hConn);
	}
}

static SQLHDBC _xefDbEstablishConnection(const xmlChar* connString, xmlChar **error)
{
	SQLWCHAR *w_conn_string = NULL;
	size_t w_conn_string_len;
	SQLHDBC db_handle;	
	SQLRETURN r;
	xmlChar *odbc_error;

	if (!connString || !*connString)
	{
		if (error)
			*error = xplFormatMessage(BAD_CAST "%s(): connString empty or NULL", __FUNCTION__);
		return SQL_NULL_HANDLE;
	}
	if (iconv_string("utf-16le", "utf-8", connString, connString + xmlStrlen(connString), (char**) &w_conn_string, &w_conn_string_len) == -1)
	{
		if (error)
			*error = xplFormatMessage(BAD_CAST "%s(): cannot convert connection string to UTF-16LE", __FUNCTION__);
		return SQL_NULL_HANDLE;
	}
	r = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &db_handle);
	if (!SQL_SUCCEEDED(r))
	{
		if (error)
		{
			odbc_error = _xefDbDecodeOdbcError(hEnv, SQL_HANDLE_ENV, r);
			*error = xplFormatMessage("%s(): SQLAllocHandle(): %s", __FUNCTION__, odbc_error);
			xmlFree(odbc_error);
		}
		xmlFree(w_conn_string);
		return SQL_NULL_HANDLE;
	}
	r = SQLDriverConnectW(db_handle, NULL, w_conn_string, (SQLSMALLINT) w_conn_string_len, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
	xmlFree(w_conn_string);
	if (!SQL_SUCCEEDED(r))
	{
		if (error)
		{
			odbc_error = _xefDbDecodeOdbcError(db_handle, SQL_HANDLE_DBC, r);
			*error = xplFormatMessage("%s(): SQLDriverConnectW(): %s", __FUNCTION__, odbc_error);
			xmlFree(odbc_error);
		}
		SQLFreeHandle(SQL_HANDLE_DBC, db_handle);
		return SQL_NULL_HANDLE;
	}
	return db_handle;
}

static xplDBPtr _xefDbGetAvailDB(xplDBListPtr list, xmlChar **error)
{
	xplDBPtr db;
	bool append = false;

	if (!(db = xplLocateAvailDB(list)))
	{
		db = xplDBCreate(NULL, xefDbDeallocateDb);
		db->busy = true;
		append = true;
	}
	if (!db->connection)
	{
		db->connection = _xefDbEstablishConnection(list->conn_string, error);
		if (!db->connection)
		{
			db->busy = false;
			return NULL;
		}
	}
	if (append)
		xplAddDBToDBList(list, db);
	return db;
}

static void _xefDbReleaseDB(xplDBPtr db)
{
	if (db)
		db->busy = false;
}

bool xefDbCheckAvail(const xmlChar* connString, const xmlChar *name, xmlChar **msg)
{
	SQLHDBC hDbc;
	xmlChar *error, *stars, *pw;

	if ((hDbc = _xefDbEstablishConnection(connString, &error)))
	{
		xefDbDeallocateDb(hDbc);
		if (msg)
			*msg = xplFormatMessage(BAD_CAST "Successfully connected to database \"%s\"", name);
		return true;
	}

	if (msg)
	{
		stars = xmlStrdup(connString);
		pw = BAD_CAST xmlStrcasestr(stars, BAD_CAST "PWD=");
		/* TODO: there may be quotes. we need some parsing here */
		if (pw)
		{
			pw += 4;
			while ((*pw != ';') && *pw)
				*pw++ = '*';
		}
		*msg = xplFormatMessage(BAD_CAST "Cannot connect to database \"%s\" (connection string \"%s\"), ODBC error \"%s\"", name, stars, error);
		xmlFree(error);
		xmlFree(stars);
	}
	return false;
}

/*================= internal structures ====================*/
typedef struct xefDbContext 
{
	void *user_data;
	xefErrorMessagePtr error;
	xefDbRowDescPtr row_desc;
	xefDbRowPtr row;
	xefDbStreamType stream_type;
	bool cleanup_stream;
	/* module-specific */
	xplDBPtr db; /* dbc */
	SQLHSTMT statement;
	bool end_of_recordsets;
} xefDbContext;

typedef struct xefDbRowBuffer 
{
	wchar_t *data;
	SQLLEN len_or_ind;
} xefDbRowBuffer, *xefDbRowBufferPtr;

static void _xefDbFreeRowDesc(xefDbRowDescPtr desc);

static void _xefDbSetContextError(xefDbContextPtr ctxt, xefErrorMessagePtr error)
{
	if (!ctxt)
		return;
	if (ctxt->error)
		xefFreeErrorMessage(ctxt->error);
	ctxt->error = error;
}

static void _xefDbCreateRowDesc(xefDbContextPtr ctxt)
{
	xefDbRowDescPtr desc;
	xmlChar *error_text = NULL;
	SQLSMALLINT col_count, i, len;
	SQLLEN attr_len;
	wchar_t *w_name;
	xmlChar *col_name;
	SQLRETURN r;
	xefDbRowBufferPtr buf;

	if (!ctxt || ctxt->error)
		return;
	if (!ctxt->statement)
	{
		_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): ctxt->statement is NULL", __FUNCTION__));
		return;
	}
	desc = (xefDbRowDescPtr) xmlMalloc(sizeof(xefDbRowDesc));
	if (!desc)
	{
		/* this will likely fail, too */
		_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): insufficient memory for desc", __FUNCTION__));
		return;
	}
	memset(desc, 0, sizeof(xefDbRowDesc));

	r = SQLNumResultCols(ctxt->statement, &col_count);
	if (!SQL_SUCCEEDED(r))
	{
		error_text = _xefDbDecodeOdbcError(ctxt->statement, SQL_HANDLE_STMT, r);
		_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): SQLNumResultCols(): %s", __FUNCTION__, error_text));
		xmlFree(error_text);
		goto error;
	}
	desc->count = col_count;

	desc->names = (xmlChar**) xmlMalloc(desc->count * sizeof(xmlChar*));
	if (!desc->names)
	{
		_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage("%s(): insufficient memory for desc->fields", __FUNCTION__));
		goto error;
	}
	memset(desc->names, 0, (size_t) desc->count * sizeof(xmlChar*));

	for (i = 1; i <= col_count; i++)
	{
		len = 0;
		r = SQLColAttributeW(ctxt->statement, i, SQL_DESC_NAME, NULL, 0, &len, NULL);
		if (!SQL_SUCCEEDED(r))
		{
			error_text = _xefDbDecodeOdbcError(ctxt->statement, SQL_HANDLE_STMT, r);
			_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): SQLColAttributeW(): %s", __FUNCTION__, error_text));
			goto error;
		}
		if (len)
		{
			w_name = (wchar_t*) xmlMalloc(len+2);
			if (!w_name)
			{
				_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): insufficient memory for column name", __FUNCTION__));
				goto error;
			}
			r = SQLColAttributeW(ctxt->statement, i, SQL_DESC_NAME, w_name, len+1, &len, NULL);
			if (!SQL_SUCCEEDED(r))
			{
				error_text = _xefDbDecodeOdbcError(ctxt->statement, SQL_HANDLE_STMT, r);
				_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): SQLColAttributeW(): %s", __FUNCTION__, error_text));
				goto error;
			}
			w_name[len/sizeof(wchar_t)] = 0;
			col_name = NULL;
			if (iconv_string("utf-8", "utf-16le", (const char*) w_name, (const char*) (w_name + len), (char**) &col_name, NULL) == -1)
			{
				_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): cannot represent column name in UTF-8 encoding", __FUNCTION__));
				xmlFree(w_name);
				goto error;
			}
			xmlFree(w_name);
			if (!col_name)
			{
				_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): insufficient memory for column name", __FUNCTION__));
				goto error;
			}
		} else // if (name_len)
			col_name = NULL;
		desc->names[i - 1] = col_name;
	}

	desc->db_objects = xmlMalloc(col_count * sizeof(xefDbRowBuffer));
	if (!desc->db_objects)
	{
		_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage("%s(): insufficient memory for desc->db_objects", __FUNCTION__));
		goto error;
	}
	memset(desc->db_objects, 0, sizeof(xefDbRowBuffer)*col_count);
	for (i = 0; i < col_count; i++)
	{
		r = SQLColAttributeW(ctxt->statement, i+1, SQL_DESC_DISPLAY_SIZE, NULL, 0, NULL, &attr_len);
		if (!SQL_SUCCEEDED(r))
		{
			error_text = _xefDbDecodeOdbcError(ctxt->statement, SQL_HANDLE_STMT, r);
			_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): SQLColAttribute(): %s", __FUNCTION__, error_text));
			goto error;
		}	
		/* desc->db_objects is of type void** */
		buf = &((xefDbRowBufferPtr) desc->db_objects)[i];
		buf->data = (wchar_t*) xmlMalloc((attr_len+1)*sizeof(wchar_t));
		if (!buf->data)
		{
			_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): insufficient memory for row buffer", __FUNCTION__));
			goto error;
		}
		r = SQLBindCol(ctxt->statement, i+1, SQL_C_WCHAR, (SQLPOINTER) buf->data, (attr_len+1) * sizeof(wchar_t), &buf->len_or_ind);
		if (!SQL_SUCCEEDED(r))
		{
			error_text = _xefDbDecodeOdbcError(ctxt->statement, SQL_HANDLE_STMT, r);
			_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): SQLBindCol(): %s", __FUNCTION__, error_text));
			goto error;
		}
	}
	goto done;
error:
	if (error_text)
		xmlFree(error_text);
	_xefDbFreeRowDesc(desc);
	return;
done:
	ctxt->row_desc = desc;
}

static void _xefDbFreeRowDesc(xefDbRowDescPtr desc)
{
	size_t i;
	xefDbRowBufferPtr buf;

	if (!desc)
		return;
	if (desc->names)
	{
		for (i = 0; i < desc->count; i++)
			if (desc->names[i])
				xmlFree(desc->names[i]);	
		xmlFree(desc->names);
	}
	if (desc->db_objects)
	{
		for (i = 0; i < desc->count; i++)
		{
			buf = &((xefDbRowBufferPtr)desc->db_objects)[i];
			if (buf->data)
				xmlFree(buf->data);
		}
		xmlFree(desc->db_objects);
	}
	xmlFree(desc);
}

static void _xefDbCreateRow(xefDbContextPtr ctxt)
{
	xefDbRowPtr row;

	if (!ctxt)
		return;
	if (!ctxt->row_desc)
	{
		_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): ctxt->row_desc is NULL", __FUNCTION__));
		return;
	}
	row = (xefDbRowPtr) xmlMalloc(sizeof(xefDbRow));
	if (!row)
	{
		_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): insufficient memory for ctxt->row", __FUNCTION__));
		return;
	}
	if (!ctxt->row_desc->count)
	{
		row->fields = NULL;
		ctxt->row = row;
		return;
	}
	row->fields = (xefDbFieldPtr) xmlMalloc(ctxt->row_desc->count * sizeof(xefDbField));
	if (!row->fields)
	{
		xmlFree(row);
		_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): insufficient memory for ctxt->row->fields", __FUNCTION__));
		return;
	}
	memset(row->fields, 0, ctxt->row_desc->count*sizeof(xefDbField));
	ctxt->row = row;
}

static void _xefDbFreeRow(xefDbRowPtr row)
{
	if (!row)
		return;
	if (row->fields)
		xmlFree(row->fields);
	xmlFree(row);
}

xefDbRowDescPtr xefDbGetRowDesc(xefDbContextPtr ctxt)
{
	return ctxt? ctxt->row_desc: NULL;
}

xefDbRowPtr xefDbGetRow(xefDbContextPtr ctxt)
{
	return ctxt? ctxt->row: NULL;
}

xefErrorMessagePtr xefDbGetError(xefDbContextPtr ctxt)
{
	return ctxt? ctxt->error: NULL;
}

void* xefDbGetUserData(xefDbContextPtr ctxt)
{
	return ctxt? ctxt->user_data: NULL;
}

bool xefDbHasRecordset(xefDbContextPtr ctxt)
{
	if (!ctxt)
		return false;
	return !ctxt->end_of_recordsets;
}

static void _xefDbSetParamsError(xefDbQueryParamsPtr params, xefErrorMessagePtr error)
{
	if (!params)
		return;
	if (params->error)
		xefFreeErrorMessage(params->error);
	params->error = error;
}

static xefDbContextPtr _xefDbCreateContext()
{
	xefDbContextPtr ret = (xefDbContextPtr) xmlMalloc(sizeof(xefDbContext));
	memset(ret, 0, sizeof(xefDbContext));
	return ret;
}

static void _xefDbRefreshContext(xefDbContextPtr ctxt)
{
	if (!ctxt)
		return;
	if (ctxt->error)
		return;
	if (ctxt->stream_type != XEF_DB_STREAM_TDS)
		return;
	if (ctxt->row_desc)
	{
		_xefDbFreeRowDesc(ctxt->row_desc);
		ctxt->row_desc = NULL;
	}
	if (ctxt->row)
	{
		_xefDbFreeRow(ctxt->row);
		ctxt->row = NULL;
	}
	if (ctxt->end_of_recordsets)
		return;
	_xefDbCreateRowDesc(ctxt);
	if (ctxt->error)
		return;
	_xefDbCreateRow(ctxt);
}

void xefDbFreeContext(xefDbContextPtr ctxt)
{
	if (!ctxt)
		return;
	_xefDbFreeRow(ctxt->row);
	_xefDbFreeRowDesc(ctxt->row_desc);
	if (ctxt->error)
		xefFreeErrorMessage(ctxt->error);
	if (ctxt->statement)
		SQLFreeHandle(SQL_HANDLE_STMT, ctxt->statement);
	_xefDbReleaseDB(ctxt->db);
	xmlFree(ctxt);
}

static SQLHSTMT _xefCreateStmt(SQLHDBC hConn, xmlChar **error)
{
	SQLRETURN r;
	SQLHSTMT stmt;
	xmlChar *error_text;

	r = SQLAllocHandle(SQL_HANDLE_STMT, hConn, &stmt);
	if (!SQL_SUCCEEDED(r))
	{
		if (error)
		{
			error_text = _xefDbDecodeOdbcError(hConn, SQL_HANDLE_DBC, r);
			*error = xplFormatMessage(BAD_CAST "%s(): SQLAllocHandle(): %s", __FUNCTION__, error_text);
			xmlFree(error_text);
		}
		return SQL_NULL_HANDLE;
	}
	return stmt;
}

static bool _xefDbExecSQL(xefDbContextPtr ctxt, xmlChar *sql)
{
	SQLRETURN r;
	SQLWCHAR *w_sql = NULL;
	xmlChar *error_text;

	if (!ctxt)
		return false;
	if (!sql || !*sql)
	{
		_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): sql is empty or NULL", __FUNCTION__));
		return false;
	}
	iconv_string("utf-16le", "utf-8", sql, sql + xmlStrlen(sql), (char**) &w_sql, NULL);
	r = SQLExecDirectW(ctxt->statement, w_sql, SQL_NTS);
	xmlFree(w_sql);
	if (!SQL_SUCCEEDED(r))
	{
		error_text = _xefDbDecodeOdbcError(ctxt->statement, SQL_HANDLE_STMT, r);
		_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): SQLExecDirect(): %s", __FUNCTION__, error_text));
		xmlFree(error_text);
		return false;
	}
	return true;
}

static bool _xefDbLocateNextNonemptyRecordset(xefDbContextPtr ctxt, bool advance)
{
	xmlChar *error_text;
	bool found = false;
	SQLRETURN r;
	SQLSMALLINT ncols;

	if (!ctxt)
		return false;
	while (1)
	{
		r = SQLNumResultCols(ctxt->statement, &ncols);
		if (!SQL_SUCCEEDED(r))
		{
			error_text = _xefDbDecodeOdbcError(ctxt->statement, SQL_HANDLE_STMT, r);
			_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): SQLNumResultCols(): %s", __FUNCTION__, error_text));
			goto error;
		}
		/* if ncols = 0, it's something like "N rows updated". not interesting */
		if (ncols)
		{
			if (advance)
				advance = false; /* skip current */
			else {
				found = true;
				break;
			}
		}
		r = SQLMoreResults(ctxt->statement);
		if (r == SQL_NO_DATA)
		{
			ctxt->end_of_recordsets = true;
			found = false;
			break;
		}
		if (!SQL_SUCCEEDED(r))
		{
			error_text = _xefDbDecodeOdbcError(ctxt->statement, SQL_HANDLE_STMT, r);
			_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): SQLMoreResults(): %s", __FUNCTION__, error_text));
			goto error;
		}
	}
	goto done;
error:
	if (error_text)
		xmlFree(error_text);
	return false;
done:
	return found;
}

static bool _xefDbNextRecord(xefDbContextPtr ctxt)
{
	xmlChar *error_text;
	SQLRETURN r;

	if (!ctxt)
		return false;
	r = SQLFetch(ctxt->statement);
	if (r == SQL_NO_DATA)
		return false;
	if (!SQL_SUCCEEDED(r))
	{
		error_text = _xefDbDecodeOdbcError(ctxt->statement, SQL_HANDLE_STMT, r);
		_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): SQLFetch(): %s", __FUNCTION__, error_text));
		xmlFree(error_text);
		return false;
	}
	return true;
}

static void _xefDbFillRow(xefDbContextPtr ctxt)
{
	xefDbRowBufferPtr buf;
	xefDbFieldPtr field;
	size_t i;

	if (!ctxt)
		return;
	if (!ctxt->row_desc)
	{
		_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): ctxt->row_desc is NULL", __FUNCTION__));
		return;
	}
	if (!ctxt->row)
	{
		_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): ctxt->row is NULL", __FUNCTION__));
		return;
	}
	for (i = 0; i < ctxt->row_desc->count; i++)
	{
		buf = &((xefDbRowBufferPtr) ctxt->row_desc->db_objects)[i];
		field = &ctxt->row->fields[i];
		if (buf->len_or_ind == SQL_NULL_DATA)
		{
			field->is_null = true;
			field->value = NULL;
			field->value_size = 0;
		} else {
			field->is_null = false;
			field->value = NULL;
			iconv_string("utf-8", "utf-16le", (const char*) buf->data, (const char*) (buf->data + buf->len_or_ind), (char**) &field->value, &field->value_size);
			// TODO cleanup nonprintable
		}
	}
}

/*=============== high-level API ================*/
xefDbContextPtr xefDbQuery(xefDbQueryParamsPtr params)
{
	xplDBPtr db = NULL;
	xmlChar *error_text = NULL;
	xefDbContextPtr ctxt = NULL;

	if (!params)
		return NULL;
	if (!params->db_list)
	{
		_xefDbSetParamsError(params, xefCreateCommonErrorMessage(BAD_CAST "%s(): params->db_list is NULL", __FUNCTION__));
		return NULL;
	}
	db = _xefDbGetAvailDB(params->db_list, &error_text);
	if (!db)
	{
		_xefDbSetParamsError(params, xefCreateCommonErrorMessage(BAD_CAST "(): cannot connect to requested database: %s", __FUNCTION__, error_text));
		xmlFree(error_text);
		return NULL;
	}
	ctxt = _xefDbCreateContext();
	ctxt->db = db;
	ctxt->stream_type = params->stream_type;
	ctxt->user_data = params->user_data;
	ctxt->cleanup_stream = params->cleanup_nonprintable;

	if (!(ctxt->statement = _xefCreateStmt((SQLHDBC)db->connection, &error_text)))
	{
		_xefDbSetParamsError(params, xefCreateCommonErrorMessage(BAD_CAST "%s(): %s", __FUNCTION__, error_text));
		xmlFree(error_text);
		goto done;
	}

	if (!_xefDbExecSQL(ctxt, params->query))
		goto error;

	_xefDbLocateNextNonemptyRecordset(ctxt, false);
	if (ctxt->error)
		goto error;
	_xefDbRefreshContext(ctxt);
	if (ctxt->error)
		goto error;
	goto done;
error:
	if (error_text)
		xmlFree(error_text);
	if (ctxt)
	{
		if (!params->error)
		{
			params->error = ctxt->error;
			ctxt->error = NULL;
		}
		xefDbFreeContext(ctxt);
	}
	if (db)
		_xefDbReleaseDB(db);
	return NULL;
done:
	return ctxt; 
}

bool xefDbNextRowset(xefDbContextPtr ctxt)
{
	if (!ctxt)
		return false;
	if (ctxt->error)
		return false;
	if (ctxt->stream_type != XEF_DB_STREAM_TDS)
	{
		_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage("xefDbNextRowset(): ctxt->stream_type (%d) != XEF_DB_STREAM_TDS", ctxt->stream_type));
		return false;
	}
	if (!ctxt->statement)
	{
		_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage("xefDbNextRowset(): ctxt->statement is NULL"));
		return false;
	}
	if (!_xefDbLocateNextNonemptyRecordset(ctxt, true))
		return false;
	_xefDbRefreshContext(ctxt);
	return !ctxt->end_of_recordsets;
}

void xefDbEnumRows(xefDbContextPtr ctxt, xefDbGetRowCallback cb)
{
	if (!ctxt)
		return;
	if (ctxt->error)
		return;
	if (!cb)
	{
		_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): cb is NULL", __FUNCTION__));
		return;
	}
	if (ctxt->stream_type != XEF_DB_STREAM_TDS)
	{
		_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): ctxt->stream_type (%d) != XEF_DB_STREAM_TDS", __FUNCTION__, ctxt->stream_type));
		return;
	}
	if (!ctxt->statement)
	{
		_xefDbSetContextError(ctxt, xefCreateCommonErrorMessage(BAD_CAST "%s(): ctxt->statement is NULL", __FUNCTION__));
		return;
	}
	if (ctxt->error)
		return;
	while (1)
	{
		if (!_xefDbNextRecord(ctxt))
			return;
		_xefDbFillRow(ctxt);
		if (ctxt->error)
			return;
		if (!cb(ctxt->row_desc, ctxt->row, ctxt->user_data))
			return; /* stop requested */
	}
}

xmlChar* xefDbAccessStreamData(xefDbContextPtr ctxt, size_t *size)
{
	return NULL;
}

void xefDbUnaccessStreamData(xefDbContextPtr ctxt, xmlChar *data)
{
}

void xefDbFreeParams(xefDbQueryParamsPtr params, bool freeCarrier)
{
	if (!params)
		return;
	if (params->query)
		xmlFree(params->query);
	if (params->error)
		xefFreeErrorMessage(params->error);
	if (freeCarrier)
		xmlFree(params);
}
