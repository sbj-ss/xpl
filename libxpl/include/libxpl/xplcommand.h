/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2021     */
/******************************************/
#ifndef __xplcommand_H
#define __xplcommand_H

#include "Configuration.h"
#include <stdbool.h>
#include <libxml/tree.h>
#include <libxml/xmlstring.h>
#include <libxml/xpath.h>
#include <libxpl/abstraction/xpr.h>
#include <libxpl/xpltree.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _xplDocument xplDocument;
typedef xplDocument* xplDocumentPtr;

/* Node parsing result */
typedef struct _xplResult
{
    xmlNodePtr list;
	bool repeat;
	bool has_list;
} xplResult, *xplResultPtr;

#define ASSIGN_RESULT(lst, rpt, has_lst) do {\
	result->list = lst;\
	result->repeat = rpt;\
	result->has_list = has_lst;\
} while(0)

/* XPL command info */
typedef struct _xplCommandInfo
{
    xmlNodePtr element;
    xplDocumentPtr document;
	void *params;
	xmlChar *content;
	xmlNodePtr prologue_error;		/* used for transferring values between prologue and epilogue */
	void *prologue_state;			/* -//- */
	xmlXPathContextPtr xpath_ctxt;
	unsigned char *required_params;
} xplCommandInfo, *xplCommandInfoPtr;

/* XPL command handler */

typedef void (*xplCommandPrologue) (xplCommandInfoPtr info);
typedef void (*xplCommandEpilogue) (xplCommandInfoPtr info, xplResultPtr result);
typedef void (*xplCommandRestoreState) (xplCommandInfoPtr info);
typedef bool (*xplCommandInitializer) (void*, xmlChar **error);
typedef void (*xplCommandFinalizer) (void*);

#define XPL_CMD_FLAG_PARAMS_FOR_PROLOGUE 0x0001UL
#define XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE 0x0002UL
#define XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE 0x0004UL
#define XPL_CMD_FLAG_REQUIRE_CONTENT 0x0008UL
#define XPL_CMD_FLAG_ALWAYS_EXPAND 0x0010UL
#define XPL_CMD_FLAG_HAS_CONTENT 0x0020UL
#define XPL_CMD_FLAG_INITIALIZED 0x1000UL

typedef enum _xplCmdParamType
{
	XPL_CMD_PARAM_TYPE_STRING,
	XPL_CMD_PARAM_TYPE_INT,
	XPL_CMD_PARAM_TYPE_BOOL,
	XPL_CMD_PARAM_TYPE_DICT,
	XPL_CMD_PARAM_TYPE_XPATH,
	XPL_CMD_PARAM_TYPE_QNAME,
	XPL_CMD_PARAM_TYPE_INT_CUSTOM_GETTER,
	XPL_CMD_PARAM_TYPE_PTR_CUSTOM_GETTER,
	XPL_CMD_PARAM_TYPE_NCNAME,
	XPL_CMD_PARAM_TYPE_MAX = XPL_CMD_PARAM_TYPE_NCNAME
} xplCmdParamType;

typedef enum _xplCmdParamXPathType
{
	XPL_CMD_PARAM_XPATH_TYPE_ANY,
	XPL_CMD_PARAM_XPATH_TYPE_NODESET,
	XPL_CMD_PARAM_XPATH_TYPE_SCALAR
} xplCmdParamXPathType;

typedef struct _xplCmdParamDictValue
{
	xmlChar *name;
	int value;
} xplCmdParamDictValue, *xplCmdParamDictValuePtr;

/* return: error message if any */
typedef xmlChar* (*xplCmdParamIntValueGetter)(xplCommandInfoPtr info, const xmlChar *raw_value, int *result);
typedef xmlChar* (*xplCmdParamPtrValueGetter)(xplCommandInfoPtr info, const xmlChar *raw_value, void **result);

typedef void (*xplCmdParamPtrValueDeallocator)(void *value);

typedef struct _xplCmdParamPtrFunctions
{
	xplCmdParamPtrValueGetter getter;
	xplCmdParamPtrValueDeallocator deallocator;
} xplCmdParamPtrFunctions, *xplCmdParamPtrFunctionsPtr;

typedef union _xplCmdParamExtraDesc
{
	xplCmdParamDictValuePtr dict_values;	/* XPL_CMD_PARAM_TYPE_DICT, (.name=NULL)-terminated */
	xplCmdParamXPathType xpath_type;		/* XPL_CMD_PARAM_TYPE_XPATH */
	bool allow_unknown_namespaces;			/* XPL_CMD_PARAM_TYPE_QNAME */
	xplCmdParamIntValueGetter int_getter;	/* XPL_CMD_PARAM_TYPE_INT_CUSTOM_GETTER */
	xplCmdParamPtrFunctions ptr_fn;			/* XPL_CMD_PARAM_TYPE_PTR_CUSTOM_GETTER */
} xplCmdParamExtraDesc;

typedef struct _xplCmdParam
{
	xmlChar *name;
	xplCmdParamType type;
	bool required;
	const void *value_stencil;
	xmlChar **aliases;					/* NULL-terminated */
	xplCmdParamExtraDesc extra;			/* depending on .type */
	int index;							/* managed by xplRegisterCommand */
	ptrdiff_t value_offset;				/* managed by xplRegisterCommand */
} xplCmdParam, *xplCmdParamPtr;

typedef struct _xplCommand
{
	xplCommandPrologue prologue;
	xplCommandEpilogue epilogue;
	xplCommandRestoreState restore_state;
	xplCommandInitializer initializer;
	xplCommandFinalizer finalizer;
	unsigned int flags;
	const void *params_stencil;
	size_t stencil_size;
	xmlHashTablePtr param_hash;			/* managed by xplRegisterCommand */
	int param_count;					/* managed by xplRegisterCommand */
	unsigned char *required_params;		/* managed by xplRegisterCommand */
	xplCmdParam parameters[];			/* (.name=NULL)-terminated */
} xplCommand, *xplCommandPtr;

/* pluggable command modules support */
#define PLUGGABLE_MODULE_VERSION 2
#define PLUGGABLE_MODULE_MAGIC 0x02706711

typedef struct _xplExternalCommand
{
	int magic;   /* = PLUGGABLE_MODULE_MAGIC */
	xmlChar *name;
	xplCommandPtr cmd;
	void *reserved[8];
} xplExternalCommand, *xplExternalCommandPtr;

typedef struct _xplExternalCommands
{
	int magic;   /* = PLUGGABLE_MODULE_MAGIC */
	int version; /* = PLUGGABLE_MODULE_VERSION */
	int count;
	XPR_SHARED_OBJECT_HANDLE handle;
	void *reserved[8];
	xplExternalCommandPtr commands;
} xplExternalCommands, *xplExternalCommandsPtr;

typedef xplExternalCommandsPtr (*GetCommandsFunc)(void);

typedef enum _xplLoadModuleResult
{
	XPL_MODULE_CMD_OK = 0,
	XPL_MODULE_CMD_MODULE_NOT_FOUND = -1,
	XPL_MODULE_CMD_INVALID_MODULE_FORMAT = -2,
	XPL_MODULE_CMD_INVALID_COMMAND_FORMAT = -3,
	XPL_MODULE_CMD_COMMAND_NAME_CLASH = -4,
	XPL_MODULE_CMD_VERSION_TOO_OLD = -5,
	XPL_MODULE_CMD_UNSUPPORTED_VERSION = -6,
	XPL_MODULE_CMD_INSUFFICIENT_MEMORY = -7,
	XPL_MODULE_CMD_COMMAND_INIT_FAILED = -8,
	XPL_MODULE_CMD_NO_PARSER = -9,
	XPL_MODULE_CMD_MODULE_ALREADY_LOADED = -10,
	// -11 is unused
	XPL_MODULE_CMD_WRONG_PARAMS = -12,
	XPL_MODULE_CMD_PARAM_NAME_CLASH = -13,
	XPL_MODULE_CMD_PARAM_DICT_NAME_CLASH = -14
} xplLoadModuleResult;

XPLPUBFUN bool XPLCALL
	xplInitCommands(void);
XPLPUBFUN xplLoadModuleResult XPLCALL
	xplRegisterCommand(const xmlChar* name, xplCommandPtr cmd, xmlChar **error);
XPLPUBFUN void XPLCALL
	xplUnregisterCommand(const xmlChar* name);
XPLPUBFUN xplCommandPtr XPLCALL
	xplGetCommand(xmlNodePtr el);
XPLPUBFUN bool XPLCALL
	xplCommandSupported(const xmlChar* name);
XPLPUBFUN xmlNodePtr XPLCALL
	xplSupportedCommandsToList(xmlNodePtr parent, const xplQName tagname);
XPLPUBFUN void XPLCALL
	xplCleanupCommands(void);

XPLPUBFUN xmlNodePtr XPLCALL
	xplGetCommandParams(xplCommandPtr command, xplCommandInfoPtr commandInfo);
XPLPUBFUN xmlNodePtr XPLCALL
	xplFillCommandInfo(xplCommandPtr command, xplCommandInfoPtr info, bool inPrologue);
XPLPUBFUN void XPLCALL
	xplClearCommandParams(xplCommandPtr command, void *values);
XPLPUBFUN void XPLCALL
	xplClearCommandInfo(xplCommandPtr command, xplCommandInfoPtr info);

XPLPUBFUN xmlXPathObjectPtr XPLCALL
	xplSelectNodes(xplCommandInfoPtr commandInfo, xmlNodePtr src, xmlChar *expr);

XPLPUBFUN bool XPLCALL
	xplLoadableModulesInit(void);
XPLPUBFUN xplLoadModuleResult XPLCALL
	xplLoadModule(xmlChar *name, xmlChar **error_data);
XPLPUBFUN void XPLCALL
	xplUnloadModule(const xmlChar *name);
XPLPUBFUN bool XPLCALL
	xplIsModuleLoaded(const xmlChar *name);
XPLPUBFUN xmlChar* XPLCALL
	xplLoadedModulesToString(const xmlChar *delimiter);
XPLPUBFUN xmlNodePtr XPLCALL
	xplLoadedModulesToNodeList(const xplQName tagname, xmlNodePtr parent);
XPLPUBFUN void XPLCALL
	xplLoadableModulesCleanup(void);
#ifdef __cplusplus
}
#endif
#endif
