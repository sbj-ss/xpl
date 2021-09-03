#include <string.h>
#include <wchar.h>
#include <libxpl/abstraction/xef.h>
#include <libxpl/xpldb.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplstring.h>
#include <sql.h>
#include <sqlext.h>

// TODO: this should be configurable
#define INITIAL_FIELD_BUFFER_SIZE 65536UL

typedef struct xefDbContext
{
	const void *user_data;
	xmlChar *error;
	int col_count;
	xefDbRowPtr row;
	xefDbStreamType stream_type;
	bool cleanup_stream;
	/* module-specific */
	xplDBPtr db; /* connection list */
	size_t buffer_size;
	SQLWCHAR *buffer;
	SQLHSTMT statement;
	bool end_of_recordsets;
} xefDbContext;

static SQLHANDLE hEnv = NULL;

static size_t sqlwcharlen(SQLWCHAR* s)
{
	SQLWCHAR *p = s;

	while (*p++);
	return p - s - 1;
}

/*============== basic XEF stuff ===============*/
bool xefStartupDatabase(xefStartupParamsPtr params)
{
	UNUSED_PARAM(params);
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

void xefShutdownDatabase()
{
	if (!hEnv)
		return;
	SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
	hEnv = NULL;
}

/* =============== errors ================= */
static xmlChar* _xefDbDecodeOdbcError(SQLHANDLE handle, SQLSMALLINT handleType, RETCODE retCode, xplDBPtr conn)
{
	SQLSMALLINT rec_no = 1, msg_len;
	SQLINTEGER error;
	SQLRETURN r;
	SQLWCHAR *msg = NULL, state[SQL_SQLSTATE_SIZE+1];
	xmlChar *conv, *ret = NULL;
	bool invalid_conn;

	if (retCode == SQL_INVALID_HANDLE)
		return BAD_CAST XPL_STRDUP("invalid handle");

	while (1)
	{
		msg = NULL;
		r = SQLGetDiagRecW(handleType, handle, rec_no, state, &error, msg, 0, &msg_len);
		if (r == SQL_NO_DATA)
			return ret? ret: BAD_CAST XPL_STRDUP("no information available");
		if (!SQL_SUCCEEDED(r))
		{
			if (ret)
				return xplFormat("%s, %s(): SQLGetDiagRecW() failed", ret, __func__);
			return xplFormat("%s(): SQLGetDiagRecW() failed", __func__);
		}
		msg = (SQLWCHAR*) XPL_MALLOC((msg_len+1)*sizeof(SQLWCHAR));
		if (conn)
		{
			invalid_conn = ( // SQL state 08S01
				state[0] == 0x30
				&& state[1] == 0x38
				&& state[2] == 0x53
				&& state[3] == 0x30
				&& state[4] == 0x31
			) || ( // SQL state 08003
				state[0] == 0x30
				&& state[1] == 0x38
				&& state[2] == 0x30
				&& state[3] == 0x30
				&& state[4] == 0x33

			);
			if (invalid_conn)
			{
				xefDbDeallocateDb(conn->connection);
				conn->connection = NULL;
			}
		}
		if (!msg)
		{
			if (ret)
				XPL_FREE(ret);
			return xplFormat("%s(): SQLGetDiagRecW(): insufficient memory", __func__);
		}
		r = SQLGetDiagRecW(handleType, handle, rec_no, state, &error, msg, msg_len+1, &msg_len);
		if (!SQL_SUCCEEDED(r))
		{
			XPL_FREE(msg);
			if (ret)
				return xplFormat("%s, %s(): SQLGetDiagRecW() failed", ret, __func__);
			return xplFormat("%s(): SQLGetDiagRecW() failed", __func__);
		}
		rec_no++;
		conv = NULL;
		xstrIconvString("utf-8", "utf-16le", (const char*) msg, (const char*) (msg + msg_len), (char**) &conv, NULL);
		XPL_FREE(msg);
		ret = ret? xmlStrcat(xmlStrcat(ret, BAD_CAST ", "), conv): conv;
		if (ret != conv)
			XPL_FREE(conv);
	}
}

static void _xefDbSetParamsError(xefDbQueryParamsPtr params, xmlChar *error)
{
	if (!params)
		return;
	if (params->error)
		XPL_FREE(params->error);
	params->error = error;
}

static void _xefDbSetContextError(xefDbContextPtr ctxt, xmlChar *error)
{
	if (!ctxt)
		return;
	if (ctxt->error)
		XPL_FREE(ctxt->error);
	ctxt->error = error;
}

/* ================= database connection ================== */
void xefDbDeallocateDb(void *db_handle)
{
	SQLHDBC hConn = (SQLHDBC) db_handle;

	if (hConn)
	{
		SQLDisconnect(hConn);
		SQLFreeHandle(SQL_HANDLE_DBC, hConn);
	}
}

void* xefDbEstablishConnection(const xmlChar* connString, xmlChar **error)
{
	SQLWCHAR *w_conn_string = NULL;
	size_t w_conn_string_len;
	SQLHDBC db_handle;	
	SQLRETURN r;
	xmlChar *odbc_error;

	if (!connString || !*connString)
	{
		if (error)
			*error = xplFormat("%s(): connString empty or NULL", __func__);
		return SQL_NULL_HANDLE;
	}
	if (xstrIconvString("utf-16le", "utf-8", (char*) connString, (char*) connString + xmlStrlen(connString), (char**) &w_conn_string, &w_conn_string_len) == -1)
	{
		if (error)
			*error = xplFormat("%s(): cannot convert connection string to UTF-16LE", __func__);
		return SQL_NULL_HANDLE;
	}
	r = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &db_handle);
	if (!SQL_SUCCEEDED(r))
	{
		if (error)
		{
			odbc_error = _xefDbDecodeOdbcError(hEnv, SQL_HANDLE_ENV, r, NULL);
			*error = xplFormat("%s(): SQLAllocHandle(): %s", __func__, odbc_error);
			XPL_FREE(odbc_error);
		}
		XPL_FREE(w_conn_string);
		return SQL_NULL_HANDLE;
	}
	r = SQLDriverConnectW(db_handle, NULL, w_conn_string, (SQLSMALLINT) w_conn_string_len, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
	XPL_FREE(w_conn_string);
	if (!SQL_SUCCEEDED(r))
	{
		if (error)
		{
			odbc_error = _xefDbDecodeOdbcError(db_handle, SQL_HANDLE_DBC, r, NULL);
			*error = xplFormat("%s(): SQLDriverConnectW(): %s", __func__, odbc_error);
			XPL_FREE(odbc_error);
		}
		SQLFreeHandle(SQL_HANDLE_DBC, db_handle);
		return SQL_NULL_HANDLE;
	}
	return (void*) db_handle;
}

bool xefDbCheckAvail(const xmlChar* connString, const xmlChar *name, xmlChar **msg)
{
	SQLHDBC hDbc;
	xmlChar *error, *stars, *pw;

	if ((hDbc = xefDbEstablishConnection(connString, &error)))
	{
		xefDbDeallocateDb(hDbc);
		if (msg)
			*msg = xplFormat("Successfully connected to database \"%s\"", name);
		return true;
	}

	if (msg)
	{
		stars = BAD_CAST XPL_STRDUP((char*) connString);
		pw = BAD_CAST xmlStrcasestr(stars, BAD_CAST "PWD=");
		/* TODO: there may be quotes. we need some parsing here */
		if (pw)
		{
			pw += 4;
			while ((*pw != ';') && *pw)
				*pw++ = '*';
		}
		*msg = xplFormat("Cannot connect to database \"%s\" (connection string \"%s\"), ODBC error \"%s\"", name, stars, error);
		XPL_FREE(error);
		XPL_FREE(stars);
	}
	return false;
}

/* ================ statements and queries =============== */
static SQLHSTMT _xefCreateStmt(xefDbContextPtr ctxt, SQLHDBC hConn, xmlChar **error)
{
	SQLRETURN r;
	SQLHSTMT stmt;
	xmlChar *error_text;

	r = SQLAllocHandle(SQL_HANDLE_STMT, hConn, &stmt);
	if (!SQL_SUCCEEDED(r))
	{
		if (error)
		{
			error_text = _xefDbDecodeOdbcError(hConn, SQL_HANDLE_DBC, r, ctxt->db);
			*error = xplFormat("%s(): SQLAllocHandle(): %s", __func__, error_text);
			XPL_FREE(error_text);
		}
		return SQL_NULL_HANDLE;
	}
	return stmt;
}

static bool _xefDbExecSQL(xefDbContextPtr ctxt, const xmlChar *sql)
{
	SQLRETURN r;
	SQLWCHAR *w_sql = NULL;
	xmlChar *error_text;

	if (!ctxt)
		return false;
	if (!sql || !*sql)
	{
		_xefDbSetContextError(ctxt, xplFormat("%s(): sql is empty or NULL", __func__));
		return false;
	}
	xstrIconvString("utf-16le", "utf-8", (char*) sql, (char*) sql + xmlStrlen(sql), (char**) &w_sql, NULL);
	r = SQLExecDirectW(ctxt->statement, w_sql, SQL_NTS);
	XPL_FREE(w_sql);
	if (r == SQL_NO_DATA)
		ctxt->end_of_recordsets = true;
	else if (!SQL_SUCCEEDED(r))	{
		error_text = _xefDbDecodeOdbcError(ctxt->statement, SQL_HANDLE_STMT, r, ctxt->db);
		_xefDbSetContextError(ctxt, xplFormat("%s(): SQLExecDirect(): %s", __func__, error_text));
		XPL_FREE(error_text);
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
	if (ctxt->end_of_recordsets)
		return false;
	while (1)
	{
		r = SQLNumResultCols(ctxt->statement, &ncols);
		if (!SQL_SUCCEEDED(r))
		{
			error_text = _xefDbDecodeOdbcError(ctxt->statement, SQL_HANDLE_STMT, r, ctxt->db);
			_xefDbSetContextError(ctxt, xplFormat("%s(): SQLNumResultCols(): %s", __func__, error_text));
			goto error;
		}
		/* if ncols = 0, it's something like "N rows updated". not interesting */
		if (ncols)
		{
			if (advance)
				advance = false; /* skip current */
			else {
				found = true;
				ctxt->col_count = ncols;
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
			error_text = _xefDbDecodeOdbcError(ctxt->statement, SQL_HANDLE_STMT, r, ctxt->db);
			_xefDbSetContextError(ctxt, xplFormat("%s(): SQLMoreResults(): %s", __func__, error_text));
			goto error;
		}
	}
	goto done;
error:
	if (error_text)
		XPL_FREE(error_text);
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
		error_text = _xefDbDecodeOdbcError(ctxt->statement, SQL_HANDLE_STMT, r, ctxt->db);
		_xefDbSetContextError(ctxt, xplFormat("%s(): SQLFetch(): %s", __func__, error_text));
		XPL_FREE(error_text);
		return false;
	}
	return true;
}

/* ================= row-level processing ================= */
static void _xefDbClearRow(xefDbRowPtr row, bool clear_names, bool force_clear_values)
{
	int i;
	xefDbFieldPtr field;

	if (row->fields)
	{
		for (i = 0; i < row->field_count; i++)
		{
			field = &(row->fields[i]);
			if (clear_names && field->name)
			{
				XPL_FREE(field->name);
				field->name = NULL;
			}
			if (field->value && (field->needs_copy || force_clear_values))
				XPL_FREE(field->value); /* otherwise the value is consumed into a text node */
			field->value = NULL;
		}
	}
}

static void _xefDbFreeRow(xefDbRowPtr row)
{
	if (!row)
		return;
	_xefDbClearRow(row, true, false);
	XPL_FREE(row->fields);
	XPL_FREE(row);
}

static void _xefDbCreateRow(xefDbContextPtr ctxt)
{
	xefDbRowPtr row;
	SQLSMALLINT i, len;
	SQLWCHAR *w_name;
	xmlChar *col_name;
	SQLRETURN r;
	xmlChar *error_text = NULL;

	if (!ctxt)
		return;
	row = (xefDbRowPtr) XPL_MALLOC(sizeof(xefDbRow));
	if (!row)
	{
		_xefDbSetContextError(ctxt, xplFormat("%s(): insufficient memory for ctxt->row", __func__));
		return;
	}
	if (!ctxt->col_count)
	{
		row->fields = NULL;
		ctxt->row = row;
		return;
	}
	row->fields = (xefDbFieldPtr) XPL_MALLOC(ctxt->col_count * sizeof(xefDbField));
	if (!row->fields)
	{
		XPL_FREE(row);
		_xefDbSetContextError(ctxt, xplFormat("%s(): insufficient memory for ctxt->row->fields", __func__));
		return;
	}
	memset(row->fields, 0, ctxt->col_count*sizeof(xefDbField));

	for (i = 1; i <= ctxt->col_count; i++)
	{
		len = 0;
		r = SQLColAttributeW(ctxt->statement, i, SQL_DESC_NAME, NULL, 0, &len, NULL);
		if (!SQL_SUCCEEDED(r))
		{
			error_text = _xefDbDecodeOdbcError(ctxt->statement, SQL_HANDLE_STMT, r, ctxt->db);
			_xefDbSetContextError(ctxt, xplFormat("%s(): SQLColAttributeW(): %s", __func__, error_text));
			goto error;
		}
		if (len)
		{
			w_name = (SQLWCHAR*) XPL_MALLOC((len+2)*sizeof(SQLWCHAR)); /* certain ODBC drivers are crazy */
			if (!w_name)
			{
				_xefDbSetContextError(ctxt, xplFormat("%s(): insufficient memory for column name", __func__));
				goto error;
			}
			*w_name = 0;
			r = SQLColAttributeW(ctxt->statement, i, SQL_DESC_NAME, w_name, len+3, &len, NULL);
			if (!SQL_SUCCEEDED(r))
			{
				error_text = _xefDbDecodeOdbcError(ctxt->statement, SQL_HANDLE_STMT, r, ctxt->db);
				_xefDbSetContextError(ctxt, xplFormat("%s(): SQLColAttributeW(SQL_DESC_NAME): %s", __func__, error_text));
				goto error;
			}
			w_name[len/sizeof(SQLWCHAR) + 1] = 0;
			col_name = NULL;
			if (xstrIconvString("utf-8", "utf-16le", (const char*) w_name, ((const char*) w_name) + len + 2, (char**) &col_name, NULL) == -1)
			{
				_xefDbSetContextError(ctxt, xplFormat("%s(): cannot represent column name in UTF-8 encoding", __func__));
				XPL_FREE(w_name);
				goto error;
			}
			XPL_FREE(w_name);
			if (!col_name)
			{
				_xefDbSetContextError(ctxt, xplFormat("%s(): insufficient memory for column name", __func__));
				goto error;
			}
		} else // if (len)
			col_name = NULL;
		row->fields[i - 1].name = col_name;
	}
	goto done;
error:
	if (error_text)
		XPL_FREE(error_text);
	_xefDbFreeRow(row);
	return;
done:
	row->field_count = ctxt->col_count;
	ctxt->row = row;
}

static void _xefDbFillRow(xefDbContextPtr ctxt)
{
	xefDbFieldPtr field;
	int i;
	bool would_grow;
	SQLWCHAR *buffer = NULL;
	SQLLEN field_size, len_or_ind;
	SQLRETURN r;
	xmlChar *error_text;

	if (!ctxt)
		return;
	if (!ctxt->row)
	{
		_xefDbSetContextError(ctxt, xplFormat("%s(): ctxt->row is NULL", __func__));
		return;
	}
	_xefDbClearRow(ctxt->row, false, false);
	for (i = 0; i < ctxt->col_count; i++)
	{
		field = &ctxt->row->fields[i];
		field_size = 0;
		would_grow = false;
		do {
			if (!ctxt->buffer)
			{
				ctxt->buffer = (SQLWCHAR*) XPL_MALLOC(INITIAL_FIELD_BUFFER_SIZE);
				ctxt->buffer_size = INITIAL_FIELD_BUFFER_SIZE;
				buffer = ctxt->buffer;
			} else if (would_grow) {
				ctxt->buffer = (SQLWCHAR*) XPL_REALLOC(ctxt->buffer, ctxt->buffer_size*2);
				field_size = ctxt->buffer_size - sizeof(SQLWCHAR);
				buffer = ctxt->buffer + ctxt->buffer_size / sizeof(SQLWCHAR) - 1;
				ctxt->buffer_size *= 2;
			} else
				buffer = ctxt->buffer;
			if (!ctxt->buffer)
			{
				_xefDbSetContextError(ctxt, xplFormat("%s(): not enough memory for ctxt->buffer", __func__));
				goto error;
			}
			r = SQLGetData(ctxt->statement, i + 1, SQL_C_WCHAR, buffer, ctxt->buffer_size - field_size, &len_or_ind);
			would_grow = true;
		} while (r == SQL_SUCCESS_WITH_INFO);
		if (!SQL_SUCCEEDED(r))
		{
			error_text = _xefDbDecodeOdbcError(ctxt->statement, SQL_HANDLE_STMT, r, ctxt->db);
			_xefDbSetContextError(ctxt, xplFormat("%s(): SQLGetData(): %s", __func__, error_text));
			XPL_FREE(error_text);
			goto error;
		}
		if (len_or_ind == SQL_NULL_DATA)
		{
			field->is_null = true;
			field->value_size = 0;
		} else {
			field->is_null = false;
			if (len_or_ind == SQL_NO_TOTAL)
				field_size = sqlwcharlen(ctxt->buffer) * sizeof(SQLWCHAR);
			else {
				field_size += len_or_ind;
				while (!*(ctxt->buffer + field_size/sizeof(SQLWCHAR) - 1))
					field_size -= sizeof(SQLWCHAR);
			}
			field->value = NULL;
			xstrIconvString("utf-8", "utf-16le", (char*) ctxt->buffer, ((char*) ctxt->buffer) + field_size, (char**) &(field->value), &(field->value_size));
			if (!field->value)
			{
				_xefDbSetContextError(ctxt, xplFormat("%s(): no memory for value recoding", __func__));
				goto error;
			}
		}
	}
	return;
error:
	_xefDbClearRow(ctxt->row, false, true);
}

/* =============== XEF context =============== */
static xefDbContextPtr _xefDbCreateContext(void)
{
	xefDbContextPtr ret = (xefDbContextPtr) XPL_MALLOC(sizeof(xefDbContext));
	if (!ret)
		return NULL;
	memset(ret, 0, sizeof(xefDbContext));
	ret->stream_type = XEF_DB_STREAM_UNKNOWN;
	return ret;
}

static void _xefDbRefreshContext(xefDbContextPtr ctxt)
{
	if (!ctxt)
		return;
	if (ctxt->error)
		return;
	if (ctxt->row)
	{
		_xefDbFreeRow(ctxt->row);
		ctxt->row = NULL;
	}
	if (ctxt->end_of_recordsets)
		return;
	_xefDbCreateRow(ctxt);
}

void xefDbFreeContext(xefDbContextPtr ctxt)
{
	if (!ctxt)
		return;
	if (ctxt->error)
		XPL_FREE(ctxt->error);
	if (ctxt->row)
		_xefDbFreeRow(ctxt->row);
	if (ctxt->statement)
	{
		SQLFreeStmt(ctxt->statement, SQL_UNBIND);
		SQLFreeHandle(SQL_HANDLE_STMT, ctxt->statement);
	}
	if (ctxt->buffer)
		XPL_FREE(ctxt->buffer);
	xplReleaseDB(ctxt->db);
	XPL_FREE(ctxt);
}

xefDbRowPtr xefDbGetRow(xefDbContextPtr ctxt)
{
	return ctxt? ctxt->row: NULL;
}

xmlChar* xefDbGetError(const xefDbContextPtr ctxt)
{
	return ctxt? ctxt->error: NULL;
}

const void* xefDbGetUserData(const xefDbContextPtr ctxt)
{
	return ctxt? ctxt->user_data: NULL;
}

bool xefDbHasRecordset(const xefDbContextPtr ctxt)
{
	if (!ctxt)
		return false;
	return !ctxt->end_of_recordsets;
}

bool xefDbGetStreamType(const xefDbContextPtr ctxt)
{
	if (!ctxt)
		return XEF_DB_STREAM_UNKNOWN;
	return ctxt->stream_type;
}

ssize_t xefDbGetRowCount(xefDbContextPtr ctxt)
{
	SQLRETURN r;
	SQLLEN len;
	xmlChar *error_text;

	if (!ctxt->statement)
	{
		_xefDbSetContextError(ctxt, xplFormat("%s(): no active statements", __func__));
		return -2;
	}
	r = SQLRowCount(ctxt->statement, &len);
	if (!SQL_SUCCEEDED(r))
	{
		error_text = _xefDbDecodeOdbcError(ctxt->statement, SQL_HANDLE_STMT, r, ctxt->db);
		_xefDbSetContextError(ctxt, xplFormat("%s(): SQLGetData(): %s", __func__, error_text));
		XPL_FREE(error_text);
		return -2;
	}
	return len;
}

/*=============== high-level API ================*/
xefDbContextPtr xefDbQuery(xefDbQueryParamsPtr params)
{
	xmlChar *error_text = NULL;
	xefDbContextPtr ctxt = NULL;

	if (!params)
		return NULL;
	if (!params->db)
	{
		_xefDbSetParamsError(params, xplFormat("%s(): params->db is NULL", __func__));
		return NULL;
	}
	if (!(ctxt = _xefDbCreateContext()))
	{
		_xefDbSetParamsError(params, xplFormat("%s(): cannot allocate context", __func__));
		goto error;
	}
	ctxt->db = params->db;
	ctxt->stream_type = XEF_DB_STREAM_TDS; /* we can't stream XML via ODBC */
	ctxt->user_data = params->user_data;
	ctxt->cleanup_stream = params->cleanup_nonprintable;

	if (!(ctxt->statement = _xefCreateStmt(ctxt, (SQLHDBC) ctxt->db->connection, &error_text)))
	{
		_xefDbSetParamsError(params, xplFormat("%s(): cannot create statement: %s", __func__, error_text? error_text: BAD_CAST "unknown error"));
		goto error;
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
		XPL_FREE(error_text);
	if (ctxt)
	{
		if (!params->error)
		{
			params->error = ctxt->error;
			ctxt->error = NULL;
		}
		xefDbFreeContext(ctxt);
	}
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
		_xefDbSetContextError(ctxt, xplFormat("xefDbNextRowset(): ctxt->stream_type (%d) != XEF_DB_STREAM_TDS", ctxt->stream_type));
		return false;
	}
	if (!ctxt->statement)
	{
		_xefDbSetContextError(ctxt, xplFormat("xefDbNextRowset(): ctxt->statement is NULL"));
		return false;
	}
	if (!_xefDbLocateNextNonemptyRecordset(ctxt, true))
		return false;
	_xefDbRefreshContext(ctxt);
	return !ctxt->end_of_recordsets;
}

void xefDbEnumRows(xefDbContextPtr ctxt, xefDbGetRowCallback cb, void *user_data)
{
	if (!ctxt)
		return;
	if (ctxt->error)
		return;
	if (!cb)
	{
		_xefDbSetContextError(ctxt, xplFormat("%s(): cb is NULL", __func__));
		return;
	}
	if (ctxt->stream_type != XEF_DB_STREAM_TDS)
	{
		_xefDbSetContextError(ctxt, xplFormat("%s(): ctxt->stream_type (%d) != XEF_DB_STREAM_TDS", __func__, ctxt->stream_type));
		return;
	}
	if (!ctxt->statement)
	{
		_xefDbSetContextError(ctxt, xplFormat("%s(): ctxt->statement is NULL", __func__));
		return;
	}
	while (1)
	{
		if (!_xefDbNextRecord(ctxt))
			return;
		_xefDbFillRow(ctxt);
		if (ctxt->error)
			return;
		if (!cb(ctxt->row, user_data))
		{
			_xefDbClearRow(ctxt->row, false, true);
			return; /* stop requested */
		}
	}
}

xmlChar* xefDbAccessStreamData(xefDbContextPtr ctxt, size_t *size)
{
	UNUSED_PARAM(ctxt);
	if (size)
		*size = 0;
	return NULL;
}

void xefDbUnaccessStreamData(xefDbContextPtr ctxt, xmlChar *data)
{
	UNUSED_PARAM(ctxt);
	UNUSED_PARAM(data);
}

void xefDbFreeParams(xefDbQueryParamsPtr params, bool freeCarrier)
{
	if (!params)
		return;
	if (params->query)
		XPL_FREE((void*) params->query);
	if (params->error)
		XPL_FREE(params->error);
	if (freeCarrier)
		XPL_FREE(params);
}
