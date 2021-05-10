#include <libxpl/xplmessages.h>
#include <libxpl/xplstring.h>
#include <libxpl/abstraction/xef.h>

#ifdef __cplusplus
#error This module must be compiled in C mode
#endif

#define COBJMACROS
#define INITGUID 
/* adoint.h is broken */
#include <objbase.h>
typedef interface Property ADOProperty;
typedef interface Properties ADOProperties;
typedef interface ADOError ADOError;
typedef interface ADOErrors ADOErrors;
typedef interface _Connection _ADOConnection;
typedef interface _Recordset _ADORecordset;
typedef interface _ADOParameter _ADOParameter;
typedef interface ADOParameters ADOParameters;
typedef interface _Command _ADOCommand;
typedef interface _ADORecord _ADORecord;
typedef interface Field ADOField;
typedef interface Fields ADOFields;
typedef interface _Stream _ADOStream;

#include <ole2.h>
/* avoid double wchar.h inclusion */
#define _INC_TCHAR
#include <adoid.h> 
#include <adoint.h>
#include <math.h>

static xmlChar* _xefDbDecodeComError()
{
	IErrorInfo *err;
	BSTR bstr_error;
	xmlChar *error_text = NULL;

	GetErrorInfo(0, &err);
	if (err)
	{		
		if SUCCEEDED(IErrorInfo_GetDescription(err, &bstr_error))
		{
			xstrIconvString("utf-8", "utf-16le", (char*) bstr_error, (char*) bstr_error + wcslen(bstr_error)*sizeof(OLECHAR), (char**) &error_text, NULL);
			SysFreeString(bstr_error);
			return error_text;
		} else {
			return BAD_CAST XPL_STRDUP("Unknown COM error");
		}
		IErrorInfo_Release(err);
	}
	return BAD_CAST XPL_STRDUP("Unknown COM error");
}

static void _xefDbSetContextError(xefDbContextPtr ctxt, xmlChar *error)
{
	if (!ctxt)
		return;
	if (ctxt->error)
		XPL_FREE(ctxt->error);
	ctxt->error = error;
}

static void _xefDbSetParamsError(xefDbQueryParamsPtr params, xmlChar *error)
{
	if (!params)
		return;
	if (params->error)
		XPL_FREE(params->error);
	params->error = error;
}

/* ================= low-level ================== */
void xefDbDeallocateDb(void *db_handle)
{
	if (db_handle)
	{
		ADOConnection *conn = (ADOConnection*) db_handle;
		_Connection_Close(conn);
		_Connection_Release(conn);
	}
}

static ADOConnection* _xefDbEstablishConnection(const xmlChar* connString, xmlChar **error)
{
	ADOConnection *conn;
	HRESULT res;
	OLECHAR *w_connstring = NULL;
	BSTR bstr_connstring = NULL;

	res = CoCreateInstance(&CLSID_CADOConnection, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, &IID_IADOConnection, (LPVOID*) &conn);
	if FAILED(res)
		return NULL;
	xstrIconvString("utf-16le", "utf-8", (char*) connString, (char*) connString + xmlStrlen(connString), 
		(char**) &w_connstring, NULL);
	if (w_connstring)
		bstr_connstring = SysAllocString(w_connstring);
	_Connection_put_ConnectionString(conn, bstr_connstring);
	res = _Connection_Open(conn, NULL, NULL, NULL, adConnectUnspecified);
	if (w_connstring) XPL_FREE(w_connstring);
	if (bstr_connstring) SysFreeString(bstr_connstring);
	if FAILED(res)
	{
		// TODO error info
		_Connection_Release(conn);
		return NULL;
	}
	return conn;
}

bool xefDbCheckAvail(const xmlChar* connString, const xmlChar *name, xmlChar **msg)
{
	ADOConnection *conn;
	xmlChar *error, *stars, *pw;

	if ((conn = _xefDbEstablishConnection(connString)))
	{
		_Connection_Close(conn);
		_Connection_Release(conn);
		if (msg)
			*msg = xplFormat("Successfully connected to database \"%s\"", name);
		return true;
	}

	if (msg)
	{
		stars = XPL_STRDUP(connString);
		pw = BAD_CAST xmlStrcasestr(stars, BAD_CAST "Password=");
		/* TODO: there can be quotes */
		if (pw)
		{
			pw += 9;
			while ((*pw != ';') && *pw)
				*pw++ = '*';
		}
		error = _xefDbDecodeComError();
		*msg = xplFormat("Cannot connect to database \"%s\" (connection string \"%s\"), ADO error \"%s\"", name, stars, error);
		XPL_FREE(error);
		XPL_FREE(stars);
	}
	return false;
}

/*====================== helpers ==================== */

xmlChar* _xefDbCleanTextStream(xmlChar *src, size_t size, size_t *out_size)
{
	xmlChar *p = src;
	xmlChar *ret, *cur;
	size_t ofs;

	if (!p)
		return NULL;
	ret = cur = (xmlChar*) XPL_MALLOC((size_t) xmlStrlen(p) + 1);
	if (!ret)
		return NULL;
	while (p < (src+size))
	{
		while (*p && (*p < 32) && (*p != '\t') && (*p != '\r') && (*p != '\n'))
			p++;
		ofs = xstrGetOffsetToNextUTF8Char(p);
		while (ofs--)
			*cur++ = *p++;
	}
	*cur = 0;
	if (out_size)
		*out_size = cur - ret;
	return ret;
}

static void _removeTrailingZeros(xmlChar *number)
{
	xmlChar *p = number;
	while (*p) p++;
	while (*--p == '0');
	if (*p != '.') p++;
	*p = 0;
}

xmlChar* _xefDbConvertValueToString(VARIANT value, size_t size, bool cleanStream)
{
	char number[32];
	xmlChar *str = NULL, *temp;
	void *data;
	SYSTEMTIME st;

	switch (value.vt)
	{
	case VT_NULL:
	case VT_EMPTY:
		return NULL;
	case VT_I1:
		_itoa(value.cVal, number, 10);
		return BAD_CAST XPL_STRDUP(number);
	case VT_UI1:
		_ultoa(value.bVal, number, 10);
		return BAD_CAST XPL_STRDUP(number);
	case VT_I2:
		_itoa(value.iVal, number, 10);
		return BAD_CAST XPL_STRDUP(number);
	case VT_UI2:
		_ultoa(value.uiVal, number, 10);
		return BAD_CAST XPL_STRDUP(number);
	case VT_I4:
		_itoa(value.lVal, number, 10);
		return BAD_CAST XPL_STRDUP(number);
	case VT_UI4:
		_ultoa(value.ulVal, number, 10);
		return BAD_CAST XPL_STRDUP(number);
	case VT_I8:
		_i64toa(value.llVal, number, 10);
		return BAD_CAST XPL_STRDUP(number);
	case VT_UI8:
		_ui64toa(value.ullVal, number, 10);
		return BAD_CAST XPL_STRDUP(number);
	case VT_CY: /* currency */
		sprintf(number, "%.4f", (double) value.cyVal.int64 / 10000);
		return BAD_CAST XPL_STRDUP(number);
	case VT_R4:
		sprintf(number, "%f", value.fltVal);
		_removeTrailingZeros(number);
		return BAD_CAST XPL_STRDUP(number);
	case VT_R8:
		sprintf(number, "%f", value.dblVal);
		_removeTrailingZeros(number);
		return BAD_CAST XPL_STRDUP(number);
	case VT_BOOL:
		return BAD_CAST XPL_STRDUP(value.boolVal? BAD_CAST "true": BAD_CAST "false"); /* true is -1! */
	case VT_INT: /* TODO. haven't met them yet */
	case VT_UINT:
		return BAD_CAST XPL_STRDUP(BAD_CAST "[machine int]");
	case VT_DECIMAL: /* 96 bit int with divider. TODO */
		if (value.decVal.Hi32)
			return BAD_CAST XPL_STRDUP(BAD_CAST "decimal > 2^64");
		if (value.decVal.sign && (value.decVal.Lo64 & 0x8000000000000000ULL))
			/*                                          7766554433221100 */ 
			return BAD_CAST XPL_STRDUP(BAD_CAST "negative decimal > 2^63");
		if (value.decVal.scale)
		{
			sprintf(number, "%f", value.decVal.Lo64*(value.decVal.sign? -1.0: 1.0) / pow(10, value.decVal.scale));
			_removeTrailingZeros(number);
		} else
			sprintf(number, "%I64d", (__int64) value.decVal.Lo64*(value.decVal.sign? -1: 1));
		return BAD_CAST XPL_STRDUP(number);
	case VT_BSTR: /* VT_LPSTR/VT_LPWSTR can't get here */
		if (!value.bstrVal)
			return NULL;
		if (xstrIconvString("utf-8", "utf-16le", (char*) value.bstrVal, (char*) value.bstrVal + wcslen(value.bstrVal)*sizeof(OLECHAR), (char**) &str, NULL) == -1)
			return BAD_CAST XPL_STRDUP("<incorrect string encoding>");
		if (cleanStream)
		{
			temp = _xefDbCleanTextStream(str, size, NULL);
			XPL_FREE(str);
			str = temp;
		}
		SysFreeString(value.bstrVal);
		return str;
	case VT_ARRAY | VT_UI1: /* binary/varbinary */
		SafeArrayAccessData(value.parray, &data);
		str = xstrBufferToHexAlloc(data, size, true);
		SafeArrayUnaccessData(value.parray);
		return str;
	case VT_DATE:
		VariantTimeToSystemTime(value.date, &st);
		sprintf(number, "%02d.%02d.%d %02d:%02d:%02d", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond);
		return BAD_CAST XPL_STRDUP(number);
	case 145: /* sql time type */
		return BAD_CAST XPL_STRDUP(BAD_CAST "[time]"); /* TODO */
	case 146: /* sql datetimeoffset type */
		return BAD_CAST XPL_STRDUP(BAD_CAST "[datetimeoffset]"); /* TODO */
	default: 
		printf("_xefDbConvertValueToString(): unsupported value type %d\n", value.vt);
	}
	return BAD_CAST XPL_STRDUP("[unknown]");
}

/*============== XEF initialization  ===============*/
bool xefStartupDatabase(xefStartupParamsPtr params)
{
	return true;
}

void xefShutdownDatabase()
{
}

/*================= internal structures management ====================*/
typedef struct xefDbContext 
{
	void *user_data;
	xmlChar *error;
	xefDbRowPtr row;
	xefDbStreamType stream_type;
	bool cleanup_stream;
	/* module-specific */
	xplDBPtr db;
	ADORecordset *rs;
	ADOStream *xml_stream;
	ADOCommand *command;
	VARIANT stream_data;
	xmlChar *stream_char_data;
} xefDbContext;

void _xefDbFreeRowDesc(xefDbRowDescPtr desc);

/* Row descriptor */
static void _xefDbCreateRowDesc(xefDbContextPtr ctxt)
{
	ADOFields *flds = NULL;
	ADOField *fld = NULL;
	xefDbRowDescPtr desc;
	xmlChar *error_text;
	VARIANT field_index;
	BSTR bstr_field_name;
	xmlChar *field_name;
	size_t i;
	LONG ado_count;

	if (!ctxt || ctxt->error)
		return;
	if (!ctxt->rs)
	{
		_xefDbSetContextError(ctxt, xplFormat("%s(): ctxt->rs is NULL", __FUNCTION__));
		return;
	}
	desc = (xefDbRowDescPtr) XPL_MALLOC(sizeof(xefDbRowDesc));
	if (!desc)
	{
		/* this will likely fail, too */
		_xefDbSetContextError(ctxt, xplFormat("%s(): insufficient memory for desc", __FUNCTION__));
		return;
	}
	memset(desc, 0, sizeof(xefDbRowDesc));

	if FAILED(_Recordset_get_Fields(ctxt->rs, &flds))
	{
		error_text = _xefDbDecodeComError();
		_xefDbSetContextError(ctxt, xplFormat("%s(): ctxt->rs->get_Fields() failed (%s)", __FUNCTION__, error_text));
		goto error;
	}
	if FAILED(Fields_get_Count(flds, &ado_count))
	{
		error_text = _xefDbDecodeComError();
		_xefDbSetContextError(ctxt, xplFormat("%s(): flds->get_Count() failed (%s)", __FUNCTION__, error_text));
		goto error;
	}
	desc->count = (size_t) ado_count;

	desc->names = (xmlChar**) XPL_MALLOC(desc->count * sizeof(xmlChar*));
	if (!desc->names)
	{
		_xefDbSetContextError(ctxt, xplFormat("%s(): insufficient memory for desc->fields", __FUNCTION__));
		goto error;
	}
	memset(desc->names, 0, (size_t) desc->count * sizeof(xmlChar*));
	desc->db_objects = (void**) XPL_MALLOC(desc->count * sizeof(void*));
	if (!desc->db_objects)
	{
		_xefDbSetContextError(ctxt, xplFormat("%s(): insufficient memory for desc->db_objects", __FUNCTION__));
		goto error;
	}
	memset(desc->db_objects, 0, desc->count * sizeof(void*));
	field_index.vt = VT_I2;
	for (i = 0; i < desc->count; i++)
	{
		field_index.iVal = (SHORT) i;
		if FAILED(Fields_get_Item(flds, field_index, &fld))
		{
			error_text = _xefDbDecodeComError();
			_xefDbSetContextError(ctxt, xplFormat("%s(): flds->get_Count() failed (%s)", __FUNCTION__, error_text));
			goto error;
		}
		if FAILED(Field_get_Name(fld, &bstr_field_name))
		{
			error_text = _xefDbDecodeComError();
			_xefDbSetContextError(ctxt, xplFormat("%s: fld->get_Name() failed (%s)", __FUNCTION__, error_text));
			goto error;
		}
		if (bstr_field_name)
		{
			field_name = NULL;
			xstrIconvString("utf-8", "utf-16le", (char*) bstr_field_name, (char*) bstr_field_name + wcslen(bstr_field_name)*sizeof(OLECHAR), (char**) &field_name, NULL);
			desc->names[i] = field_name;
			SysFreeString(bstr_field_name);
		}
		desc->db_objects[i] = fld;
	}
	Field_Release(flds); // ????
	goto done;
error:
	if (error_text)
		XPL_FREE(error_text);
	if (flds)
		Fields_Release(flds);
	_xefDbFreeRowDesc(desc);
	return;
done:
	ctxt->row_desc = desc;
}

static void _xefDbFreeRowDesc(xefDbRowDescPtr desc)
{
	size_t i;
	if (!desc)
		return;
	if (desc->names)
	{
		for (i = 0; i < desc->count; i++)
			if (desc->names[i])
				XPL_FREE(desc->names[i]);
		XPL_FREE(desc->names);
	}
	if (desc->db_objects)
	{
		for (i = 0; i < desc->count; i++)
			if (desc->db_objects[i])
				Field_Release((ADOField*) desc->db_objects[i]);
		XPL_FREE(desc->db_objects);
	}
	XPL_FREE(desc);
}

/* Row */
static xefDbRowPtr _xefDbCreateRow(xefDbRowDescPtr desc)
{
	xefDbRowPtr row;

	if (!desc)
		return NULL;
	row = (xefDbRowPtr) XPL_MALLOC(sizeof(xefDbRow));
	if (!row)
		return NULL;
	if (!desc->count)
	{
		row->fields = NULL;
		return row;
	}
	row->fields = (xefDbFieldPtr) XPL_MALLOC(desc->count * sizeof(xefDbField));
	if (!row->fields)
	{
		XPL_FREE(row);
		return NULL;
	}
	memset(row->fields, 0, desc->count*sizeof(xefDbField));
	return row;
}

static xefDbRowPtr _xefDbUpdateRow(xefDbRowDescPtr desc, xefDbRowPtr existing)
{
	if (!existing)
		return desc? _xefDbCreateRow(desc): NULL;
	if (!desc)
		return existing;
	if (!desc->count)
	{
		if (existing->fields)
			XPL_FREE(existing->fields);
		existing->fields = NULL;
		return existing;
	}
	existing->fields = (xefDbFieldPtr) XPL_REALLOC(existing->fields, desc->count * sizeof(xefDbField));
	memset(existing->fields, 0, desc->count*sizeof(xefDbField));	
	return existing;
}

static void _xefDbFreeRow(xefDbRowPtr row)
{
	if (!row)
		return;
	if (row->fields)
		XPL_FREE(row->fields);
	XPL_FREE(row);
}

/* =============== XEF context =============== */
static xefDbContextPtr _xefDbCreateContext()
{
	xefDbContextPtr ret = (xefDbContextPtr) XPL_MALLOC(sizeof(xefDbContext));
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
	if (ctxt->stream_type != XEF_DB_STREAM_TDS)
		return;
	if (!ctxt->rs)
		return;
	if (ctxt->row)
	{
		_xefDbFreeRow(ctxt->row);
		ctxt->row = NULL;
	}
	_xefDbCreateRow(ctxt);
}

void xefDbFreeContext(xefDbContextPtr ctxt)
{
	if (!ctxt)
		return;
	_xefDbFreeRow(ctxt->row);
	if (ctxt->error)
		XPL_FREE(ctxt->error);
	if (ctxt->rs)
	{
		_Recordset_Close(ctxt->rs);
		_Recordset_Release(ctxt->rs);
	}
	if (ctxt->xml_stream)
	{
		_Stream_Close(ctxt->xml_stream);
		_Stream_Release(ctxt->xml_stream);
	}
	if (ctxt->command)
	{
		_Command_Release(ctxt->command);
	}
	/* we don't close the connection here */
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
	return !!ctxt->rs;
}

bool xefDbGetStreamType(const xefDbContextPtr ctxt)
{
	if (!ctxt)
		return XEF_DB_STREAM_UNKNOWN;
	return ctxt->stream_type;
}

ssize_t xefDbGetRowCount(xefDbContextPtr ctxt)
{
	return -1; // TODO!!
}

/*=================== ADO internals ======================*/
static bool _xefDbCheckConnection(xefDbQueryParamsPtr params, ADOConnection *conn)
{
	long conn_state;
	xmlChar *error_text = NULL;
	if FAILED(_Connection_get_State(conn, &conn_state))
	{
		error_text = _xefDbDecodeComError();
		_xefDbSetParamsError(params, xplFormat("%s(): conn->get_State() failed (%s)", __FUNCTION__, error_text));
		goto error;
	}
	if (conn_state == adStateClosed)
		if FAILED(_Connection_Open(conn, NULL, NULL, NULL, adOptionUnspecified))
		{
			error_text = _xefDbDecodeComError();
			_xefDbSetParamsError(params, xplFormat("%s(): conn->Open() failed (%s)", __FUNCTION__, error_text));
			goto error;
		}	
	goto done;
error:
	if (error_text) 
		XPL_FREE(error_text);
	return false;
done:
	return true;
}

static ADOCommand* _xefDbCreateCommand(xefDbQueryParamsPtr params, ADOConnection *conn)
{
	ADOCommand *cmd = NULL;
	VARIANT conn_wrapper;
	wchar_t *wsz_query = NULL;
	BSTR bstr_query;
	xmlChar *error_text = NULL;

	if FAILED(CoCreateInstance(&CLSID_CADOCommand, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, &IID_IADOCommand, (LPVOID*) &cmd))
	{
		error_text = _xefDbDecodeComError();
		_xefDbSetParamsError(params, xplFormat("%s(): CoCreateInstance() failed (%s)", __FUNCTION__, error_text));
		goto error;
	}
	conn_wrapper.vt = VT_DISPATCH;
	conn_wrapper.pdispVal = (IDispatch*) conn;
	if FAILED(_Command_put_ActiveConnection(cmd, conn_wrapper))
	{
		error_text = _xefDbDecodeComError();
		_xefDbSetParamsError(params, xplFormat("%s(): put_ActiveConnection() failed (%s)", __FUNCTION__, error_text));
		goto error;
	}
	xstrIconvString("utf-16le", "utf-8", params->query, params->query + xmlStrlen(params->query), (char**) &wsz_query, NULL);
	bstr_query = SysAllocString(wsz_query);
	if FAILED(_Command_put_CommandText(cmd, bstr_query))
	{
		error_text = _xefDbDecodeComError();
		_xefDbSetParamsError(params, xplFormat("%s(): put_CommandText() failed (%s)", __FUNCTION__, error_text));
		goto error;
	}
	SysFreeString(bstr_query);
	XPL_FREE(wsz_query);
	wsz_query = NULL;
	if FAILED(_Command_put_CommandTimeout(cmd, 0))
	{
		error_text = _xefDbDecodeComError();
		_xefDbSetParamsError(params, xplFormat("%s(): put_CommandTimeout() failed (%s)", __FUNCTION__, error_text));
		goto error;
	}
	goto done;
error:
	if (error_text)
		XPL_FREE(error_text);
	if (bstr_query)
		SysFreeString(bstr_query);
	if (wsz_query)
		XPL_FREE(wsz_query);
	if (cmd)
		_Command_Release(cmd);
	return NULL;
done:
	return cmd;
}

static ADOStream* _xefDbCreateOutputStream(xefDbQueryParamsPtr params, ADOCommand *cmd)
{
	ADOStream *strm = NULL;
	ADOProperties *props = NULL;
	ADOProperty *prop = NULL;
	xmlChar *error_text = NULL;
	VARIANT six;

	if FAILED(CoCreateInstance(&CLSID_CADOStream, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, &IID_IADOStream, (LPVOID*) &strm))
	{
		error_text = _xefDbDecodeComError();
		_xefDbSetParamsError(params, xplFormat("%s(): CoCreateInstance() failed (%s)", __FUNCTION__, error_text));
		goto error;
	}
	six.vt = VT_ERROR;
	six.scode = DISP_E_PARAMNOTFOUND;
	if FAILED(_Stream_Open(strm, six, adModeUnknown, adOpenStreamUnspecified, NULL, NULL))
	{
		error_text = _xefDbDecodeComError();
		_xefDbSetParamsError(params, xplFormat("%s(): strm->Open() failed (%s)", __FUNCTION__, error_text));
		goto error;
	}
	if FAILED(_Command_get_Properties(cmd, &props))
	{
		error_text = _xefDbDecodeComError();
		_xefDbSetParamsError(params, xplFormat("%s(): cmd->get_Properties() failed (%s)", __FUNCTION__, error_text));
		goto error;
	}
	six.vt = VT_BSTR;
	six.bstrVal = SysAllocString(L"Output Stream");
	if FAILED(Properties_get_Item(props, six, &prop))
	{
		error_text = _xefDbDecodeComError();
		_xefDbSetParamsError(params, xplFormat("%s(): props->get_Item() failed (%s)", __FUNCTION__, error_text));
		SysFreeString(six.bstrVal);
		goto error;
	}
	six.vt = VT_DISPATCH;
	six.pdispVal = ((IDispatch*) strm);
	if FAILED(Property_put_Value(prop, six))
	{
		error_text = _xefDbDecodeComError();
		_xefDbSetParamsError(params, xplFormat("%s(): prop->put_Value() failed (%s)", __FUNCTION__, error_text));
		goto error;
	}
	Property_Release(prop);
	Properties_Release(props);
	goto done;
error:
	if (error_text)
		XPL_FREE(error_text);
	if (prop)
		Property_Release(prop);
	if (props)
		Properties_Release(props);
	if (strm)
		_Stream_Release(strm);
	return NULL;
done:
	return strm;
}

static bool _xefDbLocateNextNonemptyRecordset(xefDbContextPtr ctxt, bool advance)
{
	xmlChar *error_text;
	bool found = false;
	long state;
	ADORecordset *next;

	if (!ctxt)
		return false;
	while (!found && ctxt->rs)
	{
		if FAILED(_Recordset_get_State(ctxt->rs, &state))
		{
			error_text = _xefDbDecodeComError();
			_xefDbSetContextError(ctxt, xplFormat("%s(): ctxt->rs->get_State() failed (%s)", __FUNCTION__, error_text));
			goto error;
		}
		if (state != adStateClosed)
		{
			if (advance)
				advance = false; /* skip current record set */
			else {
				found = true;
				break;
			}
		}
		if FAILED(ctxt->rs->lpVtbl->NextADORecordset(ctxt->rs, NULL, &next))
		{
			error_text = _xefDbDecodeComError();
			_xefDbSetContextError(ctxt, xplFormat("%s(): ctxt->rs->NextADORecordset() failed (%s)", __FUNCTION__, error_text));
			goto error;
		}
		_Recordset_Close(ctxt->rs);
		_Recordset_Release(ctxt->rs);
		ctxt->rs = next;
	}
	goto done;
error:
	if (error_text)
		XPL_FREE(error_text);
	if (ctxt->rs)
	{
		_Recordset_Release(ctxt->rs);
		ctxt->rs = NULL;
	}
	return false;
done:
	return found;
}

static bool _xefDbNextRecord(xefDbContextPtr ctxt)
{
	xmlChar *error_text;

	if (!ctxt)
		return false;
	if FAILED(_Recordset_MoveNext(ctxt->rs))
	{
		error_text = _xefDbDecodeComError();
		_xefDbSetContextError(ctxt, xplFormat("%s(): ctxt->rs->MoveNext() failed (%s)", __FUNCTION__, error_text));
		XPL_FREE(error_text);
		return false;
	}
	return true;
}

static bool _xefDbCheckEof(xefDbContextPtr ctxt)
{
	VARIANT_BOOL eof;
	xmlChar *error_text;

	if (!ctxt)
		return true;
	if FAILED(_Recordset_get_EOF(ctxt->rs, &eof))
	{
		error_text = _xefDbDecodeComError();
		_xefDbSetContextError(ctxt, xplFormat("%s(): ctxt->rs->get_EOF() failed (%s)", __FUNCTION__, error_text));
		XPL_FREE(error_text);
		return true;
	}
	return eof? false: true;
}

static void _xefDbFillRow(xefDbContextPtr ctxt)
{
	size_t i;
	ADOField *field;
	xmlChar *error_text;
	VARIANT value;
	ADO_LONGPTR actual_size;

	if (!ctxt)
		return;
	if (!ctxt->row_desc)
	{
		_xefDbSetContextError(ctxt, xplFormat("%s(): INTERNAL ERROR ctxt->row_desc is NULL", __FUNCTION__));
		return;
	}
	if (!ctxt->row)
	{
		_xefDbSetContextError(ctxt, xplFormat("%s(): INTERNAL ERROR ctxt->row is NULL", __FUNCTION__));
		return;
	}
	for (i = 0; i < ctxt->row_desc->count; i++)
	{
		field = (ADOField*) ctxt->row_desc->db_objects[i];
		if FAILED(Field_get_ActualSize(field, &actual_size))
		{
			error_text = _xefDbDecodeComError();
			_xefDbSetContextError(ctxt, xplFormat("%s(): field->get_ActualSize() failed (%s)", __FUNCTION__, error_text));
			XPL_FREE(error_text);
			return;
		}
		if FAILED(Field_get_Value(field, &value))
		{
			error_text = _xefDbDecodeComError();
			_xefDbSetContextError(ctxt, xplFormat("%s(): field->get_Value() failed (%s)", __FUNCTION__, error_text));
			XPL_FREE(error_text);
			return;
		}
		ctxt->row->fields[i].value = _xefDbConvertValueToString(value, (size_t) actual_size, ctxt->cleanup_stream);
		ctxt->row->fields[i].value_size = (size_t) actual_size;
		ctxt->row->fields[i].is_null = (value.vt == VT_NULL);
		VariantClear(&value);
	}
}

static xmlChar* _xefDbAccessStreamData(xefDbContextPtr ctxt, size_t *size)
{
	ObjectStateEnum state;
	xmlChar *error_text, *ret, *temp;
	ADO_LONGPTR ado_size;

	if FAILED(_Stream_get_State(ctxt->xml_stream, &state))
	{
		error_text = _xefDbDecodeComError();
		_xefDbSetContextError(ctxt, xplFormat("%s(): ctxt->xml_stream->get_State() failed (%s)", __FUNCTION__, error_text));
		XPL_FREE(error_text);
		*size = 0;
		return NULL;
	}
	if (state != adStateOpen)
	{
		*size = 0;
		return NULL;
	}
	if FAILED(_Stream_put_Type(ctxt->xml_stream, adTypeBinary))
	{
		error_text = _xefDbDecodeComError();
		_xefDbSetContextError(ctxt, xplFormat("%s(): ctxt->xml_stream->get_State() failed (%s)", __FUNCTION__, error_text));
		XPL_FREE(error_text);
		*size = 0;
		return NULL;
	}
	if FAILED(_Stream_get_Size(ctxt->xml_stream, &ado_size))
	{
		error_text = _xefDbDecodeComError();
		_xefDbSetContextError(ctxt, xplFormat("%s(): ctxt->xml_stream->get_Size() failed (%s)", __FUNCTION__, error_text));
		XPL_FREE(error_text);
		*size = 0;
		return NULL;
	}
	if (!(*size = (size_t) ado_size))
		return NULL;
	if FAILED(_Stream_Read(ctxt->xml_stream, adReadAll, &ctxt->stream_data))
	{
		error_text = _xefDbDecodeComError();
		_xefDbSetContextError(ctxt, xplFormat("%s(): ctxt->xml_stream->Read() failed (%s)", __FUNCTION__, error_text));
		XPL_FREE(error_text);
		*size = 0;
		return NULL;
	}
	if FAILED(SafeArrayAccessData(ctxt->stream_data.parray, (void HUGEP* FAR*) &ret))
	{
		error_text = _xefDbDecodeComError();
		_xefDbSetContextError(ctxt, xplFormat("%s(): SafeArrayAccessData() failed (%s)", __FUNCTION__, error_text));
		XPL_FREE(error_text);
		*size = 0;
		return NULL;
	}
	if (ctxt->cleanup_stream)
	{
		temp = _xefDbCleanTextStream(ret, *size, size);
		ret = temp;
	}
	return ret;
}

static void _xefDbUnaccessStreamData(xefDbContextPtr ctxt, xmlChar *data)
{
	xmlChar *error_text;

	if (ctxt->cleanup_stream && data) 
		XPL_FREE(data);
	if FAILED(SafeArrayUnaccessData(ctxt->stream_data.parray))
	{
		error_text = _xefDbDecodeComError();
		_xefDbSetContextError(ctxt, xplFormat("%s(): SafeArrayUnaccessData() failed (%s)", __FUNCTION__, error_text));
		XPL_FREE(error_text);
		return;
	}
	if FAILED(VariantClear(&ctxt->stream_data))
	{
		error_text = _xefDbDecodeComError();
		_xefDbSetContextError(ctxt, xplFormat("%s(): VariantClear() failed (%s)", __FUNCTION__, error_text));
		XPL_FREE(error_text);
		return;
	}
}

/*=============== high level API ================*/
xefDbContextPtr xefDbQuery(xefDbQueryParamsPtr params)
{
	ADOConnection *conn = NULL;
	ADOCommand *cmd = NULL;
	ADOStream *stream = NULL;
	ADORecordset *rs = NULL;
	xplDBPtr db = NULL;
	xmlChar *error_text = NULL;
	xefDbContextPtr ctxt = NULL;

	if (!params)
		return NULL;
	if (!params->db_list)
	{
		_xefDbSetParamsError(params, xplFormat("%s(): params->db_list is NULL", __FUNCTION__));
		return NULL;
	}

	db = xplGetOrCreateDB(params->db_list, _xefDbEstablishConnection, &error_text);
	if (!db)
	{
		_xefDbSetParamsError(params, xplFormat("%s(): cannot connect to requested database", __FUNCTION__));
		return NULL;
	}
	conn = (ADOConnection*) db->connection;
	if (!_xefDbCheckConnection(params, conn))
		goto error;

	cmd = _xefDbCreateCommand(params, conn);
	if (!cmd)
		goto error;

	if (params->desired_stream_type == XEF_DB_STREAM_XML)
	{
		stream = _xefDbCreateOutputStream(params, cmd);
		if (!stream)
			goto error;
	}
	if FAILED(_Command_Execute(cmd, NULL, NULL, params->stream_type == XEF_DB_STREAM_XML? adExecuteStream: adOptionUnspecified, &rs))
	{
		error_text = _xefDbDecodeComError();
		_xefDbSetParamsError(params, xplFormat("%s(): cmd->Execute() failed (%s)", __FUNCTION__, error_text));
		goto error;
	}

	ctxt = _xefDbCreateContext();
	ctxt->db = db;
	ctxt->rs = rs;
	ctxt->command = cmd;
	ctxt->xml_stream = stream;
	ctxt->stream_type = params->desired_stream_type;
	ctxt->user_data = params->user_data;
	ctxt->cleanup_stream = params->cleanup_nonprintable;

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
	} else {
		if (rs)
			_Recordset_Release(rs);
		if (stream)
			_Stream_Release(stream);
		if (cmd)
			_Command_Release(cmd);
	}
	if (db)
		xplReleaseDB(db);
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
		_xefDbSetContextError(ctxt, xplFormat("%s(): ctxt->stream_type (%d) != XEF_DB_STREAM_TDS", __FUNCTION__, ctxt->stream_type));
		return false;
	}
	if (!ctxt->rs)
	{
		_xefDbSetContextError(ctxt, xplFormat("%s(): ctxt->rs is NULL", __FUNCTION__));
		return false;
	}
	if (!_xefDbLocateNextNonemptyRecordset(ctxt, true))
		return false;
	_xefDbRefreshContext(ctxt);
	return !!ctxt->rs;
}

void xefDbEnumRows(xefDbContextPtr ctxt, xefDbGetRowCallback cb)
{
	bool continue_flag;

	if (!ctxt)
		return;
	if (ctxt->error)
		return;
	if (!cb)
	{
		_xefDbSetContextError(ctxt, xplFormat("%s(): cb is NULL", __FUNCTION__));
		return;
	}
	if (ctxt->stream_type != XEF_DB_STREAM_TDS)
	{
		_xefDbSetContextError(ctxt, xplFormat("%s(): ctxt->stream_type (%d) != XEF_DB_STREAM_TDS", __FUNCTION__, ctxt->stream_type));
		return;
	}
	if (!ctxt->rs)
	{
		_xefDbSetContextError(ctxt, xplFormat("%s(): ctxt->rs is NULL", __FUNCTION__));
		return;
	}
	continue_flag = _xefDbCheckEof(ctxt);
	if (ctxt->error)
		return;
	while (continue_flag)
	{
		_xefDbFillRow(ctxt);
		if (ctxt->error)
			return;
		if (!cb(ctxt->row_desc, ctxt->row, ctxt->user_data))
			return; /* break */
		_xefDbNextRecord(ctxt);
		if (ctxt->error)
			return;
		continue_flag = _xefDbCheckEof(ctxt);
		if (ctxt->error)
			return;
	}
}

xmlChar* xefDbAccessStreamData(xefDbContextPtr ctxt, size_t *size)
{
	if (!ctxt)
		return NULL;
	if (ctxt->stream_type != XEF_DB_STREAM_XML)
	{
		_xefDbSetContextError(ctxt, xplFormat("%s(): ctxt->stream_type != XEF_DB_STREAM_XML", __FUNCTION__));
		return NULL;
	}
	if (!ctxt->xml_stream)
	{
		_xefDbSetContextError(ctxt, xplFormat("%s(): ctxt->xml_stream is NULL", __FUNCTION__));
		return NULL;
	}
	if (!size)
	{
		_xefDbSetContextError(ctxt, xplFormat("%s(): size parameter is NULL", __FUNCTION__));
		return NULL;
	}
	return _xefDbAccessStreamData(ctxt, size);
}

void xefDbUnaccessStreamData(xefDbContextPtr ctxt, xmlChar *data)
{
	if (!ctxt)
		return;
	if (ctxt->stream_type != XEF_DB_STREAM_XML)
	{
		_xefDbSetContextError(ctxt, xplFormat("%s(): ctxt->stream_type != XEF_DB_STREAM_XML", __FUNCTION__));
		return;
	}
	if (!ctxt->xml_stream)
	{
		_xefDbSetContextError(ctxt, xplFormat("%s(): ctxt->xml_stream is NULL", __FUNCTION__));
		return;
	}
	_xefDbUnaccessStreamData(ctxt, data);
}

void xefDbFreeParams(xefDbQueryParamsPtr params, bool freeCarrier)
{
	if (!params)
		return;
	if (params->query)
		XPL_FREE(params->query);
	if (params->error)
		XPL_FREE(params->error);
	if (freeCarrier)
		XPL_FREE(params);
}
