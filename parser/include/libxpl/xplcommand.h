/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __xplcommand_H
#define __xplcommand_H

#include "Configuration.h"
#include <stdbool.h>
#include <libxml/tree.h>
#include <libxml/xmlstring.h>
#include <libxpl/abstraction/xpr.h>

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
	void *_private;
} xplCommandInfo, *xplCommandInfoPtr;

/* XPL command handler */

typedef void (*xplCommandPrologue) (xplCommandInfoPtr info);
typedef void (*xplCommandEpilogue) (xplCommandInfoPtr info, xplResultPtr result);
typedef bool (*xplCommandInitializer) (void*, xmlChar **error);
typedef void (*xplCommandFinalizer) (void*);

#define XPL_CMD_FLAG_CONTENT_SAFE 0x0001UL
#define XPL_CMD_FLAG_INITIALIZED 0x0100UL

typedef enum _xplCmdParamType
{
	XPL_CMD_PARAM_TYPE_STRING,
	XPL_CMD_PARAM_TYPE_INTEGER,
	XPL_CMD_PARAM_TYPE_BOOL,
	XPL_CMD_PARAM_TYPE_DICT
} xplCmdParamType;

typedef struct _xplCmdParamDictValue
{
	xmlChar *name;
	int value;
} xplCmdParamDictValue, *xplCmdParamDictValuePtr;

typedef struct _xplCmdParam
{
	xmlChar *name;
	xplCmdParamType type;
	bool required;
	const void *value_stencil;
	int index;							/* managed by xplRegisterCommand */
	ptrdiff_t value_offset;				/* managed by xplRegisterCommand */
	xplCmdParamDictValuePtr dict_values;/* (.name=NULL)-terminated */
	xmlChar **aliases;					/* NULL-terminated */
} xplCmdParam, *xplCmdParamPtr;

typedef struct _xplCommand
{
	xplCommandPrologue prologue;
	xplCommandEpilogue epilogue;
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

typedef enum _xplModuleCmdResult
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
	XPL_MODULE_CMD_LOCK_ERROR = -11,
	XPL_MODULE_CMD_WRONG_PARAMS = -12,
	XPL_MODULE_CMD_PARAM_NAME_CLASH = -13,
	XPL_MODULE_CMD_PARAM_DICT_NAME_CLASH = -14
} xplModuleCmdResult;

XPLPUBFUN bool XPLCALL
	xplInitCommands();
XPLPUBFUN xplModuleCmdResult XPLCALL
	xplRegisterCommand(const xmlChar* name, xplCommandPtr cmd, xmlChar **error);
XPLPUBFUN void XPLCALL
	xplUnregisterCommand(const xmlChar* name);
XPLPUBFUN xplCommandPtr XPLCALL
	xplGetCommand(xmlNodePtr el);
XPLPUBFUN bool XPLCALL
	xplCommandSupported(const xmlChar* name);
XPLPUBFUN xmlNodePtr XPLCALL
	xplSupportedCommandsToList(xmlDocPtr doc, xmlNodePtr parent, const xmlChar *tagName);
XPLPUBFUN void XPLCALL
	xplCleanupCommands();

XPLPUBFUN xmlNodePtr XPLCALL
	xplDecodeCmdBoolParam(xmlNodePtr cmd, const xmlChar *name, bool *value, bool defaultValue);
XPLPUBFUN xmlNodePtr XPLCALL
	xplGetCommandParams(xplCommandPtr command, xmlNodePtr carrier, void *values);
XPLPUBFUN void XPLCALL
	xplClearCommandParams(xplCommandPtr command, void *values);

XPLPUBFUN xplModuleCmdResult XPLCALL
	xplLoadableModulesInit(void);
XPLPUBFUN xplModuleCmdResult XPLCALL
	xplLoadModule(xmlChar *name, xmlChar **error_data);
XPLPUBFUN void XPLCALL
	xplUnloadModule(xmlChar *name);
XPLPUBFUN bool XPLCALL
	xplIsModuleLoaded(const xmlChar *name);
XPLPUBFUN xmlChar* XPLCALL
	xplLoadedModulesToString(const xmlChar *delimiter);
XPLPUBFUN xmlNodePtr XPLCALL
	xplLoadedModulesToNodeList(const xmlChar *tagQName, xmlNodePtr parent);
XPLPUBFUN void XPLCALL
	xplLoadableModulesCleanup(void);
#ifdef __cplusplus
}
#endif
#endif
