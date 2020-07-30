/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __glue_H
#define __glue_H

#include <libxml/xmlstring.h>
#include <libxpl/xplparams.h>
#include <libxpl/xplsession.h>
#include <civetweb.h>

#ifdef __cplusplus
extern "C" {
#endif

/* app type returned by :get-app-type */
#define APP_TYPE (BAD_CAST "web")

#define OM_XML "xml"
#define OM_HTML "html"
#define OM_XHTML "xhtml"
#define OM_TEXT "text"
#define OM_NONE "none"

#define DEFAULT_OUTPUT_ENC "utf-8"
/* HTML is generally better than XHTML for browser compatibility */
#define DEFAULT_OUTPUT_METHOD OM_HTML
#define CONFIG_FILE "xplweb.conf"

#define CONTENT_TYPE_PARAM (BAD_CAST "ContentType")
#define DOC_ROOT_PARAM (BAD_CAST "DocRoot")
#define ENCODING_PARAM (BAD_CAST "Encoding")
#define OUTPUT_METHOD_PARAM (BAD_CAST "OutputMethod")
#define REMOTE_ADDRESS_PARAM (BAD_CAST "RemoteIP") // TODO modify code and change to "RemoteAddress"
#define RESOURCE_PARAM (BAD_CAST "Resource")

#define SESSION_ID_COOKIE "session_id"

typedef enum _OutputMethod
{
	OUTPUT_METHOD_XML,
	OUTPUT_METHOD_HTML,
	OUTPUT_METHOD_XHTML,
	OUTPUT_METHOD_TEXT,
	OUTPUT_METHOD_NONE
} OutputMethod;

extern xmlChar *app_path;

OutputMethod getOutputMethod(xmlChar *output_method);

xplParamsPtr buildParams(struct mg_connection *conn, const struct mg_request_info *request_info, xmlChar *session_id);
void serializeDoc(struct mg_connection *conn, xmlDocPtr doc, xmlChar *encoding, OutputMethod om);
void setSessionCookie(struct mg_connection *conn, xplSessionPtr session);
int serveXpl(struct mg_connection *conn, void *user_data);

#ifdef __cplusplus
}
#endif
#endif
