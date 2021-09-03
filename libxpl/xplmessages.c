#include <time.h>
#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>

static FILE *log_file = NULL;
static XPR_MUTEX console_interlock;
bool messages_initialized = false;

// TODO define color codes in XPR
static const xplMsgTypeDesc message_type_descs[] = {
	[XPL_MSG_UNKNOWN] = { XPR_DEFAULT_CONSOLE_COLOR, "UNKNOWN MESSAGE" },
	[XPL_MSG_DEBUG] = { 0x06, "DEBUG" },
	[XPL_MSG_INFO] = { XPR_DEFAULT_CONSOLE_COLOR, "INFO" },
	[XPL_MSG_WARNING] = { 0x08, "WARNING" },
	[XPL_MSG_ERROR] = { 0x0E, "ERROR" },
	[XPL_MSG_INTERNAL_ERROR] = { 0x0C, "INTERNAL ERROR" }
};

static const xplMsgTypeDesc unknown_message_desc = {
	XPR_DEFAULT_CONSOLE_COLOR, "MESSAGE TYPE OUT OF BOUNDS"
};

const xplMsgTypeDesc* xplMsgTypeDescByType(xplMsgType msgType)
{
	if ((int) msgType < 0 || (int) msgType > XPL_MSG_MAX)
		return &unknown_message_desc;
	return message_type_descs[msgType];
}

xplMsgType xplMsgTypeFromString(const xmlChar *severity, bool allowInternalError)
{
	if (!xmlStrcasecmp(severity, BAD_CAST "debug"))
		return XPL_MSG_DEBUG;
	if (!xmlStrcasecmp(severity, BAD_CAST "info"))
		return XPL_MSG_INFO;
	if (!xmlStrcasecmp(severity, BAD_CAST "warning"))
		return XPL_MSG_WARNING;
	if (!xmlStrcasecmp(severity, BAD_CAST "error"))
		return XPL_MSG_ERROR;
	if (allowInternalError && !xmlStrcasecmp(severity, BAD_CAST "internal error"))
		return XPL_MSG_INTERNAL_ERROR;
	return XPL_MSG_UNKNOWN;
}

xmlChar* xplFormat(const char *fmt, ...)
{
	xmlChar *ret;
	size_t ret_size;
	va_list args;

	va_start(args, fmt);
	ret_size = _vscprintf((const char*) fmt, args);
	ret = (xmlChar*) XPL_MALLOC(ret_size + 1);
	if (!ret)
		return NULL;
	vsnprintf((char*) ret, ret_size + 1, (const char*) fmt, args);
	return ret;
}

xmlChar* xplVFormatMessage(const char *fmt, va_list args)
{
	xmlChar *ret;
	size_t ret_size;

	ret_size = _vscprintf((const char*) fmt, args);
	ret = (xmlChar*) XPL_MALLOC(ret_size + 1);
	if (!ret)
		return NULL;
	vsnprintf((char*) ret, ret_size + 1, (const char*) fmt, args);
	return ret;
}

void xplDisplayMessage(xplMsgType msgType, const char *fmt, ... )
{
	char *encoded_msg;
	xmlChar *msg;
	va_list arg_list;
	char now[32];
	struct tm *_tm;
	time_t tnow;
	static char encoding_error_msg[] = "[cannot display message using console encoding]";
	const xplMsgTypeDesc *type_desc;

	type_desc = xplMsgTypeDescByType(msgType);
	va_start(arg_list, fmt);
	msg = xplVFormatMessage(fmt, arg_list);
	time(&tnow);
	if ((_tm = localtime(&tnow)))
		strftime(now, 32, "%d.%m.%Y %H:%M:%S", _tm);
	else
		*now = 0;

	if (log_file)
	{
		fprintf(log_file, "[%s] %s: %s\n", now, type_desc->name, msg);
		if (cfgFlushLogFile)
			fflush(log_file);
	}

	encoded_msg = NULL;
	if (xstrIconvString(XPR_CONSOLE_ENCODING, "utf-8", (const char*) msg, (const char*) msg + xmlStrlen(msg), &encoded_msg, NULL) == -1)
		encoded_msg = encoding_error_msg;
	if (messages_initialized && !xprMutexAcquire(&console_interlock))
		/* prefer garbled messages over no messages at all */
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	if (cfgUseConsoleColors)
		xprSetConsoleColor(type_desc->color);
	xmlGenericError(xmlGenericErrorContext, "[%s] %s: %s\n", now, type_desc->name, encoded_msg);
	if (cfgUseConsoleColors)
		xprResetConsoleColor();
	if (messages_initialized && !xprMutexRelease(&console_interlock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	if (encoded_msg != encoding_error_msg)
		XPL_FREE(encoded_msg);
	XPL_FREE(msg);
}

void xplDisplayWarning(const xmlNodePtr carrier, const char *fmt, ...)
{
	va_list arg_list;
	xmlChar *inner_message;

	va_start(arg_list, fmt);
	inner_message = xplVFormatMessage(fmt, arg_list);

	xplDisplayMessage(
		XPL_MSG_WARNING,
		"%s%s%s: %s. file '%s', line %d",
		(carrier->ns && carrier->ns->prefix)? (char*) carrier->ns->prefix: "",
		(carrier->ns && carrier->ns->prefix)? ":": "",
		carrier->name,
		inner_message,
		carrier->doc->URL,
		carrier->line
	);
	if (inner_message)
		XPL_FREE(inner_message);
}


xmlNodePtr xplCreateSimpleErrorNode(const xmlDocPtr doc, const char *msg, const xmlChar *src)
{
	xmlNodePtr txt, ret;
	ret = xmlNewDocNode(doc, NULL, ERROR_NODE_NAME, NULL);
	if (msg)
	{
		txt = xmlNewDocText(doc, NULL);
		txt->content = BAD_CAST msg;
		txt->parent = ret;
		ret->children = ret->last = txt;
	}
	xmlNewProp(ret, ERROR_SOURCE_NAME, src);
	return ret;
}

xmlNodePtr xplCreateErrorNode(const xmlNodePtr cmd, const char *fmt, ...)
{
	xmlNodePtr ret;
	xmlChar *orig_msg, *full_msg;
	va_list arg_list;

	va_start(arg_list, fmt);
	orig_msg = xplVFormatMessage(fmt, arg_list);
	full_msg = xplFormat(
		"%s%s%s: %s (file \"%s\", line %d)",
		cmd->ns && cmd->ns->prefix? cmd->ns->prefix: BAD_CAST "",
		cmd->ns && cmd->ns->prefix? ":": "",
		cmd->name,
		orig_msg,
		cmd->doc->URL,
		cmd->line);
	XPL_FREE(orig_msg);
	ret = xplCreateSimpleErrorNode(cmd->doc, (char*) full_msg, cmd->name);
	if (cfgErrorsToConsole)
		xplDisplayMessage(XPL_MSG_ERROR, "%s", (char*) full_msg);
	if (cfgStackTrace)
		xplStackTrace(cmd);
	return ret;
}

void xplStackTrace(const xmlNodePtr startPoint)
{
	xmlNodePtr cur;
	xmlAttrPtr attr;
	xmlChar *attr_value;
	xmlNsPtr ns;

	cur = startPoint;
	while (cur && (cur->type != XML_DOCUMENT_NODE))
	{
		xmlGenericError(
			xmlGenericErrorContext,
			"at <%s%s%s",
			cur->ns && cur->ns->prefix? cur->ns->prefix: BAD_CAST "",
			cur->ns && cur->ns->prefix? ":": "",
			cur->name);
		ns = cur->nsDef;
		while (ns)
		{
			xmlGenericError(
				xmlGenericErrorContext,
				" xmlns%s%s=\"%s\"",
				ns->prefix? ns->prefix: BAD_CAST "",
				ns->prefix? ":": "",
				ns->href);
			ns = ns->next;
		}
		attr = cur->properties;
		while (attr)
		{
			attr_value = xplGetPropValue(attr);
			xmlGenericError(
				xmlGenericErrorContext,
				" %s%s%s=\"%s\"",
				attr->ns && attr->ns->prefix? attr->ns->prefix: BAD_CAST "",
				attr->ns && attr->ns->prefix? ":": "",
				attr->name,
				attr_value);
			if (attr_value)
				XPL_FREE(attr_value);
			attr = attr->next;
		}
		xmlGenericError(xmlGenericErrorContext, ">\n");
		cur = cur->parent;
	}
	xmlGenericError(xmlGenericErrorContext, "-------\n");
}

static void xplCleanupLogger()
{
	if (log_file)
	{
		fclose(log_file);
		log_file = NULL;
	}
}

static bool xplInitLogger()
{
	xmlChar *log_file_full_name, *executable_path;

	xplCleanupLogger();
	if (cfgLogFileName)
	{
		executable_path = xprGetProgramPath();
		log_file_full_name = xplFormat("%s%s%s", executable_path, BAD_CAST XPR_PATH_DELIM_STR, cfgLogFileName);
		log_file = xprFOpen(log_file_full_name, "a");
		if (!log_file)
			xplDisplayMessage(XPL_MSG_WARNING, "cannot open log file \"%s\" for writing", log_file_full_name);
		XPL_FREE(log_file_full_name);
		XPL_FREE(executable_path);
		return !!log_file;
	} else
		return true;
}

bool xplInitMessages()
{
	if (messages_initialized)
		return true;
	if (!xprMutexInit(&console_interlock))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return false;
	}
	if (!xplInitLogger())
		return false;
	messages_initialized = true;
	return true;
}

void xplCleanupMessages()
{
	if (!messages_initialized)
		return;
	if (!xprMutexCleanup(&console_interlock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	xplCleanupLogger();
	messages_initialized = false;
}
