#include <time.h>
#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>

static FILE *log_file = NULL;
static XPR_MUTEX console_interlock;
bool messages_initialized = false;

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

xmlChar* xplFormatMessage(xmlChar *fmt, ...)
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

xmlChar* xplVFormatMessage(xmlChar *fmt, va_list args)
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

void xplDisplayMessage(xplMsgType msgType, xmlChar *fmt, ... )
{
	char *encoded_msg;
	xmlChar *msg;
	va_list arg_list;
	char now[32];
	struct tm *_tm;
	time_t tnow;
	char *what;
	int color;
	static char encoding_error_msg[] = "[cannot display message using console encoding]";

	switch (msgType)
	{
		case XPL_MSG_DEBUG:   what = "DEBUG"; break;
		case XPL_MSG_INFO:    what = "INFO"; break;
		case XPL_MSG_WARNING: what = "WARNING"; break;
		case XPL_MSG_ERROR:   what = "ERROR"; break;
		case XPL_MSG_INTERNAL_ERROR: what = "INTERNAL ERROR"; break;
		default: what = "UNKNOWN MESSAGE";
	}

	va_start(arg_list, fmt);
	msg = xplVFormatMessage(fmt, arg_list);
	time(&tnow);
	if ((_tm = localtime(&tnow)))
		strftime(now, 32, "%d.%m.%Y %H:%M:%S", _tm);
	else
		*now = 0;

	if (log_file)
	{
		fprintf(log_file, "[%s] %s: %s\n", now, what, msg);
		/* ToDo: setting?.. */
		fflush(log_file);
	}

	encoded_msg = NULL;
	if (xstrIconvString(XPR_CONSOLE_ENCODING, "utf-8", (const char*) msg, (const char*) msg + xmlStrlen(msg), &encoded_msg, NULL) == -1)
		encoded_msg = encoding_error_msg;
	if (messages_initialized && !xprMutexAcquire(&console_interlock))
		/* prefer garbled messages over no messages at all */
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	if (cfgUseConsoleColors)
	{
		switch (msgType)
		{
			case XPL_MSG_DEBUG: color = 0x06; break; // TODO define these codes in XPR
			case XPL_MSG_INFO: color = XPR_DEFAULT_CONSOLE_COLOR; break;
			case XPL_MSG_WARNING: color = 0x0B; break;
			case XPL_MSG_ERROR: color = 0x0E; break;
			case XPL_MSG_INTERNAL_ERROR: color = 0x0C; break;
			default: color = XPR_DEFAULT_CONSOLE_COLOR;
		}
		xprSetConsoleColor(color);
	}
	xmlGenericError(xmlGenericErrorContext, "[%s] %s: %s\n", now, what, encoded_msg);
	if (cfgUseConsoleColors)
		xprResetConsoleColor();
	if (messages_initialized && !xprMutexRelease(&console_interlock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	if (encoded_msg != encoding_error_msg)
		XPL_FREE(encoded_msg);
	XPL_FREE(msg);
}

void xplDisplayWarning(xmlNodePtr carrier, xmlChar *fmt, ...)
{
	va_list arg_list;
	xmlChar *inner_message;

	va_start(arg_list, fmt);
	inner_message = xplVFormatMessage(fmt, arg_list);

	xplDisplayMessage(
		XPL_MSG_WARNING,
		BAD_CAST "%s%s%s: %s. file '%s', line %d",
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


xmlNodePtr xplCreateSimpleErrorNode(xmlDocPtr doc, xmlChar *msg, const xmlChar *src)
{
	xmlNodePtr txt, ret;
	ret = xmlNewDocNode(doc, NULL, ERROR_NODE_NAME, NULL);
	if (msg)
	{
		txt = xmlNewDocText(doc, NULL);
		txt->content = msg;
		txt->parent = ret;
		ret->children = ret->last = txt;
	}
	xmlNewProp(ret, ERROR_SOURCE_NAME, src);
	return ret;
}

xmlNodePtr xplCreateErrorNode(const xmlNodePtr cmd, const xmlChar *fmt_msg, ...)
{
	xmlNodePtr ret;
	xmlChar *fmt, *msg;
	va_list arg_list;

	if (cmd->ns)
		fmt = xplFormatMessage(BAD_CAST "%s:%s: %s (file \"%s\", line %d)",
			cmd->ns->prefix,
			cmd->name,
			fmt_msg,
			cmd->doc->URL,
			cmd->line);
	else
		fmt = xplFormatMessage(BAD_CAST "%s: %s (file \"%s\", line %d)",
			cmd->name,
			fmt_msg,
			cmd->doc->URL,
			cmd->line);
	va_start(arg_list, fmt_msg);
	msg = xplVFormatMessage(fmt, arg_list);
	XPL_FREE(fmt);
	ret = xplCreateSimpleErrorNode(cmd->doc, msg, cmd->name);
	if (cfgErrorsToConsole)
		xplDisplayMessage(XPL_MSG_ERROR, msg);
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
		if (cur->ns)
			xmlGenericError(xmlGenericErrorContext, "at <%s:%s", cur->ns->prefix, cur->name);
		else
			xmlGenericError(xmlGenericErrorContext, "at <%s", cur->name);
		ns = cur->nsDef;
		while (ns)
		{
			xmlGenericError(xmlGenericErrorContext, " xmlns:%s=\"%s\"", ns->prefix, ns->href);
			ns = ns->next;
		}
		attr = cur->properties;
		while (attr)
		{
			attr_value = xplGetPropValue(attr);
			if (attr->ns)
				xmlGenericError(xmlGenericErrorContext, " %s:%s=\"%s\"", attr->ns->prefix, attr->name, attr_value);
			else
				xmlGenericError(xmlGenericErrorContext, " %s=\"%s\"", attr->name, attr_value);
			if (attr_value) XPL_FREE(attr_value);
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
	xmlChar *log_file_full_name = NULL, *executable_path;

	xplCleanupLogger();
	if (cfgLogFileName)
	{
		executable_path = xprGetProgramPath();
		log_file_full_name = xmlStrcat(log_file_full_name, executable_path);
		log_file_full_name = xmlStrcat(log_file_full_name, BAD_CAST XPR_PATH_DELIM_STR);
		log_file_full_name = xmlStrcat(log_file_full_name, cfgLogFileName);
		log_file = xprFOpen(log_file_full_name, "a");
		if (!log_file)
			xplDisplayMessage(XPL_MSG_WARNING, BAD_CAST "cannot open log file \"%s\" for writing", log_file_full_name);
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
		return false;
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
