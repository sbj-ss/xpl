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
	xmlChar *app_path;					/* "application" root */
	xmlChar *filename;					/* full path to file the document was created from */
	xmlChar *origin;					/* initial document text if it was created from memory */
	xmlChar *error;						/* early parsing error */
	xplParamsPtr environment;			/* external parameters */
	xplSessionPtr shared_session;		/* session variables */
	xplSessionPtr local_session;		/* -//- */
	xmlDocPtr document;					/* underlying XML document */
	bool expand;						/* current expansion mode */
	int recursion_level;				/* protection from infinite loops in macros */
	xmlXPathContextPtr xpath_ctxt;		/* reusable XPath context */
	xmlNodePtr fatal_content;			/* for xpl:fatal */
	xplMacroPtr current_macro;			/* ditto */
	xplError status;					/* processing status */
	xmlChar *response;					/* for :set-response and app using the interpreter */
	xmlNodePtr stack;					/* for :stack-xx */
										/* threading support */
	rbBufPtr thread_handles;			/* spawned threads */
	rbBufPtr suspended_thread_docs; 	/* sub-documents waiting to be spawned */
	XPR_MUTEX thread_landing_lock;		/* the "master" doc owns this */
	xmlNodeSetPtr landing_point_path;	/* xmlNodePtr[] from landing point to doc root */
	xplDocumentPtr parent;				/* spawning document */
	bool async;							/* child running asynchronously */
	int indent_spin;					/* for :text */
	xmlNsPtr root_xpl_ns;				/* for fast XPL namespace checking */
	XPR_TIME profile_start_time;		/* for execution timing */
	rbBufPtr deleted_nodes;				/* for deferred node deletion */
	int iterator_spin;					/* for :with */
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
XPLPUBFUN bool XPLCALL
	xplStartDelayedThreads(xplDocumentPtr doc);
XPLPUBFUN void XPLCALL
	xplDiscardSuspendedThreadDocs(xplDocumentPtr doc);
XPLPUBFUN void XPLCALL
	xplFinalizeDocThreads(xplDocumentPtr doc, bool force_mutex_cleanup);
#endif

/* per-document node stack (for :stack-*) */
XPLPUBFUN void XPLCALL
	xplPushToDocStack(xplDocumentPtr doc, xmlNodePtr node);
XPLPUBFUN xmlNodePtr XPLCALL
	xplPopFromDocStack(xplDocumentPtr doc, xmlNodePtr parent);
XPLPUBFUN void XPLCALL
	xplClearDocStack(xplDocumentPtr doc);
XPLPUBFUN bool XPLCALL
	xplDocStackIsEmpty(xplDocumentPtr doc);

/* safe deletion for fool-proof mode */
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

/* session wrappers */
XPLPUBFUN xplSessionPtr XPLCALL
	xplDocSessionGetOrCreate(xplDocumentPtr doc, bool local);
XPLPUBFUN bool XPLCALL
	xplDocSessionExists(xplDocumentPtr doc, bool local);
XPLPUBFUN xmlChar* XPLCALL
	xplDocSessionGetId(xplDocumentPtr doc, bool local);
XPLPUBFUN xmlNodePtr XPLCALL
	xplDocSessionGetObject(xplDocumentPtr doc, bool local, const xmlChar *name, const xmlNodePtr parent, const xmlChar *select, bool *ok);
XPLPUBFUN xmlNodePtr XPLCALL
	xplDocSessionGetAllObjects(xplDocumentPtr doc, bool local, const xmlNodePtr parent, const xmlChar *select, bool *ok);
XPLPUBFUN bool XPLCALL
	xplDocSessionSetObject(xplDocumentPtr doc, bool local, const xmlNodePtr cur, const xmlChar *name);
XPLPUBFUN void XPLCALL
	xplDocSessionRemoveObject(xplDocumentPtr doc, bool local, const xmlChar *name);
XPLPUBFUN void XPLCALL
	xplDocSessionClear(xplDocumentPtr doc, bool local);
XPLPUBFUN bool XPLCALL
	xplDocSessionGetSaMode(xplDocumentPtr doc); // no local param here: any session will do
XPLPUBFUN bool XPLCALL
	xplDocSessionSetSaMode(xplDocumentPtr doc, bool local, bool enable, xmlChar *password);

XPLPUBFUN bool XPLCALL
	xplCheckNodeForXplNs(xplDocumentPtr doc, xmlNodePtr element);
XPLPUBFUN xmlNodePtr XPLCALL
	xplReplaceContentEntries(
		xplDocumentPtr doc,
		const xmlChar* id,
		xmlNodePtr oldElement,
		xmlNodePtr macroContent,
		xmlNodePtr parent
	);
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

/* application hooks */
XPLPUBFUN void XPLCALL
	xplLockThreads(bool doLock);

typedef xmlChar* (*xplGetAppTypeFunc)(void);

XPLPUBFUN xplGetAppTypeFunc XPLCALL
	xplSetGetAppTypeFunc(xplGetAppTypeFunc f);
XPLPUBFUN xmlChar* XPLCALL
	xplGetAppType(void);

/* This function uses the config file path set by xplInitParser.
 * It should only be called after the parser initialization to RE-read the config if it's changed externally.
 */
XPLPUBFUN bool XPLCALL
	xplReadConfig(void);

/* High-level functions */
/* Init and done */
XPLPUBFUN xplError XPLCALL
	xplInitParser(xmlChar* cfgFile, bool verbose);
XPLPUBFUN void XPLCALL
	xplDoneParser(void);
XPLPUBFUN bool XPLCALL
	xplParserLoaded(void);
XPLPUBFUN xmlChar* XPLCALL
	xplGetDocRoot(void);
XPLPUBFUN void XPLCALL
	xplSetDocRoot(xmlChar *new_root);
/* Full file name wrt "semi-absolute" (i.e. from app root) paths. Result must be freed by the caller. */
XPLPUBFUN xmlChar* XPLCALL
	xplFullFilename(const xmlChar* file, const xmlChar* appPath);
/* Actual processing */
XPLPUBFUN xplError XPLCALL
	xplProcessFile(xmlChar *aAppPath, xmlChar *aFilename, xplParamsPtr environment, xplSessionPtr session, xplDocumentPtr *docOut);
/* Path format (e.g.)
   basePath = "d:\\Tomcat 4.1\\webapps\\ROOT"
   relativePath = "/Impulse/Developer.xpl" */
XPLPUBFUN xplError XPLCALL
	xplProcessFileEx(xmlChar *basePath, xmlChar *relativePath, xplParamsPtr environment, xplSessionPtr session, xplDocumentPtr *docOut);

#ifdef __cplusplus
}
#endif
#endif
