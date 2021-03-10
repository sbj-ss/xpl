/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __glue_H
#define __glue_H

#include "Configuration.h"
#include <libxml/xmlstring.h>
#include <libxpl/xplparams.h>
#include <libxpl/xplsession.h>
#include <civetweb.h>

#ifdef __cplusplus
extern "C" {
#endif

/* app type returned by :get-app-type */
#define APP_TYPE (BAD_CAST "web")

#define DEFAULT_OUTPUT_ENC "utf-8"

#define CONTENT_TYPE_PARAM (BAD_CAST "ContentType")
#define DOC_ROOT_PARAM (BAD_CAST "DocRoot")
#define ENCODING_PARAM (BAD_CAST "Encoding")
#define OUTPUT_METHOD_PARAM (BAD_CAST "OutputMethod")
#define REMOTE_ADDRESS_PARAM (BAD_CAST "RemoteIP")
#define RESOURCE_PARAM (BAD_CAST "Resource")

#define SESSION_ID_COOKIE "session_id"

typedef enum _OutputSerializer
{
	OS_XML,
	OS_TEXT,
	OS_NONE
} OutputSerializer;

typedef struct _OutputMethodDesc
{
	xmlChar *name;
	xmlChar *content_type;
	int xml_format;
	OutputSerializer serializer;
} OutputMethodDesc, *OutputMethodDescPtr;

extern xmlChar *doc_root;

OutputMethodDescPtr XPLCALL /* ditto */
	getOutputMethod(xmlChar *name);
xplParamsPtr XPLCALL /* collect URL query and POST body params */
	buildParams(struct mg_connection *conn, const struct mg_request_info *request_info, xmlChar *session_id);
xmlChar* XPLCALL /* serialize output document using selected method */
	serializeDoc(xmlDocPtr doc, xmlChar *encoding, OutputMethodDescPtr om, size_t *size);
void XPLCALL /* send session id cookie to client */
	setSessionCookie(struct mg_connection *conn, xplSessionPtr session);
int XPLCALL /* handle a request to .xpl file */
	serveXpl(struct mg_connection *conn, void *user_data);

#ifdef __cplusplus
}
#endif
#endif
