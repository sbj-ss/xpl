#include "Messages.h"

#include "Options.h"
#include "Utils.h"
#include "abstraction/xpr.h"
#include <time.h>

static FILE *log_file = NULL;
static XPR_MUTEX console_interlock;

xplMsgType xplMsgTypeFromString(xmlChar *severity, bool allowInternalError)
{
	if (!xmlStrcasecmp(severity, BAD_CAST "debug"))
		return xplMsgDebug;
	if (!xmlStrcasecmp(severity, BAD_CAST "info"))
		return xplMsgInfo;
	if (!xmlStrcasecmp(severity, BAD_CAST "warning"))
		return xplMsgWarning;
	if (!xmlStrcasecmp(severity, BAD_CAST "error"))
		return xplMsgError;
	if (allowInternalError && !xmlStrcasecmp(severity, BAD_CAST "internal error"))
		return xplMsgInternalError;
	return xplMsgUnknown;
}

xmlChar* xplFormatMessage(xmlChar *fmt, ...)
{
	xmlChar *ret;
	size_t ret_size;
	va_list args;

	va_start(args, fmt);
	ret_size = _vscprintf((const char*) fmt, args);
	ret = (xmlChar*) xmlMalloc(ret_size + 1);
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
	ret = (xmlChar*) xmlMalloc(ret_size + 1);
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
	static char encoding_msg[] = "[cannot display message using console encoding]";

	switch (msgType)
	{
		case xplMsgDebug:   what = "DEBUG"; break;
		case xplMsgInfo:    what = "INFO"; break;
		case xplMsgWarning: what = "WARNING"; break;
		case xplMsgError:   what = "ERROR"; break;
		case xplMsgInternalError: what = "INTERNAL ERROR"; break;
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
		/* ToDo: может, настройку?.. */
		fflush(log_file);
	}

	encoded_msg = NULL;
	if (iconv_string(XPR_CONSOLE_ENCODING, "utf-8", (const char*) msg, (const char*) msg + xmlStrlen(msg), &encoded_msg, NULL) == -1)
		encoded_msg = encoding_msg;
	/* xplDisplayMessage() should be working even after the interpreter shutdown */
	(void) xprMutexAcquire(&console_interlock);
	if (cfgUseConsoleColors)
	{
		switch (msgType)
		{
			case xplMsgDebug: color = 0x06; break;
			case xplMsgInfo: color = XPR_DEFAULT_CONSOLE_COLOR; break;
			case xplMsgWarning: color = 0x0B; break;
			case xplMsgError: color = 0x0E; break;
			case xplMsgInternalError: color = 0x0C; break;
			default: color = XPR_DEFAULT_CONSOLE_COLOR;
		}
		xprSetConsoleColor(color);
	}
	xmlGenericError(xmlGenericErrorContext, "[%s] %s: %s\n", now, what, encoded_msg);
	if (cfgUseConsoleColors)
		xprSetConsoleColor(XPR_DEFAULT_CONSOLE_COLOR);
	(void) xprMutexRelease(&console_interlock);
	if (encoded_msg != encoding_msg)
		xmlFree(encoded_msg);
	xmlFree(msg);
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
	xmlFree(fmt);
	ret = xplCreateSimpleErrorNode(cmd->doc, msg, cmd->name);
	if (cfgErrorsToConsole)
		xplDisplayMessage(xplMsgError, msg);
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
			attr_value = getPropValue(attr);
			if (attr->ns)
				xmlGenericError(xmlGenericErrorContext, " %s:%s=\"%s\"", attr->ns->prefix, attr->name, attr_value);
			else
				xmlGenericError(xmlGenericErrorContext, " %s=\"%s\"", attr->name, attr_value);
			if (attr_value) xmlFree(attr_value);
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
		fclose(log_file);
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
			xplDisplayMessage(xplMsgWarning, BAD_CAST "cannot open log file \"%s\" for writing", cfgLogFileName);
		xmlFree(log_file_full_name);
		xmlFree(executable_path);
		return log_file? true: false;
	} else
		return true;
}

bool xplInitMessages()
{
	if (!xprMutexInit(&console_interlock))
		return false;
	return xplInitLogger();
}

void xplCleanupMessages()
{
	if (!xprMutexCleanup(&console_interlock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	xplCleanupLogger();
}
