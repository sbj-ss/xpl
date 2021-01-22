/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/

#ifndef __xplcore_H
#define __xplcore_H

#include "Configuration.h"
#include <libxml/xpath.h>
#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplbuffer.h>
#include <libxpl/xplcommand.h>
#include <libxpl/xpldb.h>
#include <libxpl/xplmacro.h>
#include <libxpl/xplparams.h>
#include <libxpl/xplsession.h>
#include <libxpl/xplwrappers.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Error handling */
typedef enum _xplError
{
	XPL_ERR_FATAL_CALLED = 1,
	XPL_ERR_NO_ERROR = 0,
	XPL_ERR_INVALID_DOCUMENT = -1,
	XPL_ERR_DOC_NOT_CREATED = -2,
	XPL_ERR_NO_PARSER = -3,
	XPL_ERR_INVALID_INPUT = -4,
	XPL_ERR_NO_CONFIG_FILE = -5,
	XPL_ERR_NOT_YET_REACHED = -6
} xplError;

XPLPUBFUN const xmlChar* XPLCALL 
	xplErrorToString(xplError error);
XPLPUBFUN const xmlChar* XPLCALL
	xplErrorToShortString(xplError error);

/* XPL document forward declaration */
typedef struct _xplDocument xplDocument, *xplDocumentPtr;

/* XPL document */
struct _xplDocument
{
	xmlChar *app_path;				/* "application" root */
	xmlChar *filename;				/* full path to file the document was created from */
	xmlChar *origin;				/* initial document text if it was created from memory */
	xmlChar *error;					/* early parsing error */
	xplParamsPtr environment;		/* external parameters */
	xplSessionPtr session;			/* session variables */
	xmlDocPtr document;				/* underlying XML document */
	bool expand;					/* current expansion mode */
	int recursion_level;			/* protection from infinite loops in macros */
	xmlXPathContextPtr xpath_ctxt;	/* reusable XPath context */
	xmlNodePtr fatal_content;		/* for xpl:fatal */
	xplDocRole role;				/* role in "prologue-main-epilogue" model */
	xplDocumentPtr prologue;		/* prologue-main-epilogue parts */
	xplDocumentPtr main;
	xplDocumentPtr epilogue;
	xplMacroPtr current_macro;		/* ditto */
	xplError status;				/* processing status */
	xplDocSource source;			/* was the original document overridden by wrappers? */
	xmlChar *response;				/* for :set-response and app using the interpreter */
	xmlNodePtr stack;				/* for :stack-xx */
									/* threading support */
	rbBufPtr threads;				/* spawned threads */
	XPR_MUTEX thread_landing_lock;	/* ditto */
	xmlNodePtr landing_point;		/* landing point in spawning document */
	xplDocumentPtr parent;			/* spawning document */
	bool has_suspended_threads;		/* ditto */
	bool discard_suspended_threads;	/* discard threads that weren't ever started */
	bool thread_was_suspended;		/* current thread with document was suspended */
	int indent_spinlock;			/* for :text */
	xmlNsPtr root_xpl_ns;			/* for fast XPL namespace checking */
	time_t profile_checkpoint;		/* for :profile-xx */
	rbBufPtr deleted_nodes;		/* for deferred node deletion */
	int iterator_spinlock;			/* for :with */
};

XPLPUBFUN xplDocumentPtr XPLCALL
	xplDocumentInit(xmlChar *aAppPath, xplParamsPtr aEnvironment, xplSessionPtr aSession);
XPLPUBFUN xplDocumentPtr XPLCALL
	xplDocumentCreateFromFile(xmlChar *aAppPath, xmlChar *aFilename, xplParamsPtr aEnvironment, xplSessionPtr aSession);
XPLPUBFUN xplDocumentPtr XPLCALL
	xplDocumentCreateFromMemory(xmlChar* aAppPath, xmlChar *aOrigin,  xplParamsPtr aEnvironment, xplSessionPtr aSession, xmlChar *encoding);
/* Main method */
XPLPUBFUN xplError XPLCALL 
	xplDocumentApply(xplDocumentPtr doc);
XPLPUBFUN void XPLCALL
	xplDocumentFree(xplDocumentPtr doc);

#ifdef _THREADING_SUPPORT
XPLPUBFUN bool XPLCALL
	xplEnsureDocThreadSupport(xplDocumentPtr doc);
XPLPUBFUN void XPLCALL
	xplWaitForChildThreads(xplDocumentPtr doc);
XPLPUBFUN bool XPLCALL
	xplStartChildThread(xplDocumentPtr doc, xplDocumentPtr child, bool immediateStart);
XPLPUBFUN void XPLCALL
	xplStartDelayedThreads(xplDocumentPtr doc);
#endif

/* per-document node stack (for :stack-xx) */
XPLPUBFUN void XPLCALL
	xplPushToDocStack(xplDocumentPtr doc, xmlNodePtr node);
XPLPUBFUN xmlNodePtr XPLCALL
	xplPopFromDocStack(xplDocumentPtr doc, xmlNodePtr parent);
XPLPUBFUN void XPLCALL
	xplClearDocStack(xplDocumentPtr doc);
XPLPUBFUN bool XPLCALL
	xplDocStackIsEmpty(xplDocumentPtr doc);

XPLPUBFUN bool XPLCALL
	xplCheckNodeForXplNs(xplDocumentPtr doc, xmlNodePtr element);
XPLPUBFUN xmlNodePtr XPLCALL
	xplReplaceContentEntries(xplDocumentPtr doc, const xmlChar* id, xmlNodePtr oldElement, xmlNodePtr macroContent);
XPLPUBFUN void XPLCALL
	xplNodeApply(xplDocumentPtr doc, xmlNodePtr element, xplResultPtr result);
XPLPUBFUN void XPLCALL
	xplNodeListApply(xplDocumentPtr doc, xmlNodePtr children, xplResultPtr result);
XPLPUBFUN xplMacroPtr XPLCALL
	xplAddMacro(
		xplDocumentPtr doc, 
		xmlNodePtr macro,
		xplQName qname,
		xmlNodePtr destination, 
		xplMacroExpansionState expansionState,
		bool replace,
		xmlChar *id
	);

XPLPUBFUN void XPLCALL
	xplDeferNodeDeletion(rbBufPtr buf, xmlNodePtr cur);
XPLPUBFUN void XPLCALL
	xplDeferNodeListDeletion(rbBufPtr buf, xmlNodePtr cur);
XPLPUBFUN void XPLCALL
	xplDocDeferNodeDeletion(xplDocumentPtr doc, xmlNodePtr cur);
XPLPUBFUN void XPLCALL
	xplDocDeferNodeListDeletion(xplDocumentPtr doc, xmlNodePtr cur);
XPLPUBFUN void XPLCALL 
	xplDeleteDeferredNodes(rbBufPtr buf);

/* application hooks */

XPLPUBFUN void XPLCALL
	xplLockThreads(bool doLock);

typedef xmlChar* (*xplGetAppTypeFunc)(void);

XPLPUBFUN xplGetAppTypeFunc XPLCALL
	xplSetGetAppTypeFunc(xplGetAppTypeFunc f);
XPLPUBFUN xmlChar* XPLCALL
	xplGetAppType(void);

/* Функция берёт путь к конфигу из значения, переданного xplInitParser.
   Соответственно, вызывать её до инициализации парсера - бессмысленно.
   Назначение - ПЕРЕчитать конфиг после внесения в него изменений.
 */
XPLPUBFUN bool XPLCALL
	xplReadConfig(void);

/* High-level functions */
/* Init and done */
XPLPUBFUN xplError XPLCALL
	xplInitParser(xmlChar* cfgFile);
XPLPUBFUN void XPLCALL
	xplDoneParser(void);
XPLPUBFUN bool XPLCALL
	xplParserLoaded(void);
XPLPUBFUN xmlChar* XPLCALL
	xplGetDocRoot(void);
XPLPUBFUN void XPLCALL
	xplSetDocRoot(xmlChar *new_root);
/* Полное имя файла с учётом абсолютных путей. Результат необходимо удалить. */
XPLPUBFUN xmlChar* XPLCALL
	xplFullFilename(const xmlChar* file, const xmlChar* appPath);
/* Actual processing */
XPLPUBFUN xplError XPLCALL
	xplProcessFile(xmlChar *aAppPath, xmlChar *aFilename, xplParamsPtr environment, xplSessionPtr session, xplDocumentPtr *docOut);
/* Path format (e.g.)
   basePath = "d:\\Tomcat 4.1\\webapps\\ROOT"
   relativePath = "/Impulse/Developer.xpl" */
XPLPUBFUN bool XPLCALL
	xplGetDocByRole(xplDocumentPtr docIn, const xmlChar *strRole, xplDocumentPtr *docOut);
/* command params helper */
XPLPUBFUN xmlChar* XPLCALL
	xplDocByRoleGetter(xplCommandInfoPtr info, const xmlChar *raw_value, void **result);
XPLPUBFUN xplError XPLCALL
	xplProcessFileEx(xmlChar *basePath, xmlChar *relativePath, xplParamsPtr environment, xplSessionPtr session, xplDocumentPtr *docOut);

#ifdef __cplusplus
}
#endif
#endif
