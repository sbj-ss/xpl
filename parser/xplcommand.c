#include <string.h>
#include <libxpl/xplcommand.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>

static xmlHashTablePtr commands = NULL;
static xmlHashTablePtr loaded_modules = NULL;
static XPR_MUTEX module_locker;

bool xplInitCommands()
{
	xplCleanupCommands();
	commands = xmlHashCreate(128);
	return commands? true: false;
}

xplModuleCmdResult xplRegisterCommand(const xmlChar *name, xplCommandPtr cmd, xmlChar **error)
{
	if (!commands)
		return XPL_MODULE_CMD_NO_PARSER;
	/* Don't allow overrides */
	if (xmlHashAddEntry(commands, name, cmd) == -1)
		return XPL_MODULE_CMD_COMMAND_NAME_CLASH;
	if (cmd->initializer)
		if (!cmd->initializer(NULL, error))
		{
			xmlHashRemoveEntry(commands, name, NULL);
			return XPL_MODULE_CMD_COMMAND_INIT_FAILED;
		}
	return XPL_MODULE_CMD_OK;
}

void xplUnregisterCommand(const xmlChar *name)
{
	xplCommandPtr cmd;

	if (!commands)
		return;
	cmd = (xplCommandPtr) xmlHashLookup(commands, name);
	if (cmd)
	{
		if (cmd->finalizer)
			cmd->finalizer(NULL);
		xmlHashRemoveEntry(commands, name, NULL);
	}
}

xplCommandPtr xplGetCommand(xmlNodePtr el)
{
	return (xplCommandPtr) xmlHashLookup(commands, el->name);
}

static const xmlChar* builtins[] =
{
	BAD_CAST "expand",
	BAD_CAST "no-expand",
	BAD_CAST "expand-after",
	BAD_CAST "content",
	BAD_CAST "define"
};

bool xplCommandSupported(const xmlChar* name)
{
	unsigned int i;

	for (i = 0; i < sizeof(builtins) / sizeof(xmlChar*); i++)
		if (!xmlStrcmp(name, builtins[i]))
			return true;
	if (!commands)
		return false;
	return xmlHashLookup(commands, name)? true: false;
}

typedef struct _CommandListScannerContext
{
	xmlChar *name;
	xmlDocPtr doc;
	xmlNsPtr ns;
	xmlNodePtr head, tail;
} CommandListScannerContext, *PCommandListScannerContext;

static void commandListScanner(void *payload, void *data, xmlChar *name)
{
	xmlNodePtr cur;
	PCommandListScannerContext ctxt = (PCommandListScannerContext) data;
	if (ctxt->name)
		cur = xmlNewDocNode(ctxt->doc, ctxt->ns, ctxt->name, name);
	else
		cur = xmlNewDocNode(ctxt->doc, NULL, name, NULL);
	if (ctxt->head)
	{
		ctxt->tail->next = cur;
		cur->prev = ctxt->tail;
		ctxt->tail = cur;
	} else
		ctxt->head = ctxt->tail = cur;
}

xmlNodePtr xplSupportedCommandsToList(xmlDocPtr doc, xmlNodePtr parent, const xmlChar *tagName)
{
	CommandListScannerContext ctxt;
	unsigned int i;

	ctxt.doc = doc;
	ctxt.head = NULL;
	EXTRACT_NS_AND_TAGNAME(tagName, ctxt.ns, ctxt.name, parent)
	for (i = 0; i < sizeof(builtins) / sizeof(builtins[0]); i++)
		commandListScanner(NULL, &ctxt, BAD_CAST builtins[i]);
	xmlHashScan(commands, commandListScanner, &ctxt);
	return ctxt.head;
}

static void commandDeallocator(void *payload, xmlChar *name)
{
	xplCommandPtr cmd = (xplCommandPtr) payload;
	if (cmd->finalizer)
		cmd->finalizer(NULL);
}

void xplCleanupCommands()
{
	if (commands)
	{
		xmlHashFree(commands, commandDeallocator);
		commands = NULL;
	}
}

static void FreeModulesCallback(void *payload, xmlChar *name)
{
	xplUnloadModule(name);
}

int xplLoadableModulesCleanup(void)
{
	xmlHashFree(loaded_modules, FreeModulesCallback);
	if (!xprMutexCleanup(&module_locker))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	return 1;
}

xplModuleCmdResult xplLoadableModulesInit(void)
{
	if (loaded_modules)
		xplLoadableModulesCleanup();
	loaded_modules = xmlHashCreate(16);
	if (!loaded_modules)
		return XPL_MODULE_CMD_INSUFFICIENT_MEMORY;
	if (!xprMutexInit(&module_locker))
		return XPL_MODULE_CMD_LOCK_ERROR;
	return XPL_MODULE_CMD_OK;
}

xplModuleCmdResult xplLoadModule(xmlChar *name, xmlChar **error_data)
{
#define ASSIGN_ERR_DATA(x) if (error_data) *error_data = x;
	XPR_SHARED_OBJECT_HANDLE hmodule;
	xmlChar *path_with_ext;
	GetCommandsFunc get_commands_func;
	xplExternalCommandsPtr cmds;
	int i;
	xmlChar i_buffer[12];
	xplModuleCmdResult ret;
	xmlChar *cmd_error;
	void *prev;

	if (!loaded_modules)
		return XPL_MODULE_CMD_NO_PARSER;
	if (!xprMutexAcquire(&module_locker))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return XPL_MODULE_CMD_LOCK_ERROR;
	}
	prev = xmlHashLookup(loaded_modules, name);
	if (!xprMutexRelease(&module_locker))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return XPL_MODULE_CMD_LOCK_ERROR;
	}
	if (prev)
		return XPL_MODULE_CMD_MODULE_ALREADY_LOADED;
	ASSIGN_ERR_DATA(NULL);
	path_with_ext = (xmlChar*) XPL_MALLOC(xmlStrlen(name) + xmlStrlen(XPR_SHARED_OBJECT_EXT) + 1);
	if (!path_with_ext)
		return XPL_MODULE_CMD_INSUFFICIENT_MEMORY;
	*path_with_ext = 0;
	strcpy((char*) path_with_ext, (char*) name);
	strcat((char*) path_with_ext, (char*) XPR_SHARED_OBJECT_EXT);
	hmodule = xprLoadSharedObject(path_with_ext);
	if (!hmodule)
	{
		ASSIGN_ERR_DATA(path_with_ext);
		return XPL_MODULE_CMD_MODULE_NOT_FOUND;
	}
	XPL_FREE(path_with_ext);
	get_commands_func = (GetCommandsFunc) xprGetProcAddress(hmodule, "GetCommands");
	if (!get_commands_func)
	{
		xprUnloadSharedObject(hmodule);
		return XPL_MODULE_CMD_INVALID_MODULE_FORMAT;
	}
	cmds = get_commands_func();
	if (!cmds || cmds->magic != PLUGGABLE_MODULE_MAGIC)
	{
		xprUnloadSharedObject(hmodule);
		return XPL_MODULE_CMD_INVALID_MODULE_FORMAT;
	}
	if (cmds->version > PLUGGABLE_MODULE_VERSION)
	{
		snprintf((char*) i_buffer, 12, "%d", cmds->version);
		xprUnloadSharedObject(hmodule);
		ASSIGN_ERR_DATA(XPL_STRDUP(i_buffer));
		return XPL_MODULE_CMD_UNSUPPORTED_VERSION;
	}
	if (cmds->version < 2)
	{
		snprintf((char*) i_buffer, 12, "%d", cmds->version);
		xprUnloadSharedObject(hmodule);
		ASSIGN_ERR_DATA(XPL_STRDUP(i_buffer));
		return XPL_MODULE_CMD_VERSION_TOO_OLD;
	}
	cmds->handle = (void*) hmodule;
	if (!xprMutexAcquire(&module_locker))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		xprUnloadSharedObject(hmodule);
		return XPL_MODULE_CMD_LOCK_ERROR;
	}
	for (i = 0; i < cmds->count; i++)
	{
		if (cmds->commands[i].magic != PLUGGABLE_MODULE_MAGIC)
		{
			snprintf((char*) i_buffer, 12, "%d", i);
			ASSIGN_ERR_DATA(XPL_STRDUP(i_buffer));
			xplUnloadModule(name);
			if (!xprMutexRelease(&module_locker))
				DISPLAY_INTERNAL_ERROR_MESSAGE();
			return XPL_MODULE_CMD_INVALID_COMMAND_FORMAT;
		}
		if ((ret = xplRegisterCommand((const xmlChar*) cmds->commands[i].name, cmds->commands[i].cmd, &cmd_error)) != XPL_MODULE_CMD_OK)
		{
			ASSIGN_ERR_DATA(XPL_STRDUP(cmds->commands[i].name));
			/* ToDo: pass error to caller */
			xplDisplayMessage(xplMsgError, BAD_CAST "loading module \"%s\": command \"%s\" failed to initialize, error\"%s\"", name, cmds->commands[i].name, cmd_error);
			if (cmd_error) XPL_FREE(cmd_error);
			xplUnloadModule(name);
			if (!xprMutexRelease(&module_locker))
				DISPLAY_INTERNAL_ERROR_MESSAGE();
			return ret;
		}
	}
	xmlHashAddEntry(loaded_modules, name, cmds);
	if (!xprMutexRelease(&module_locker))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		xprUnloadSharedObject(hmodule);
		return XPL_MODULE_CMD_LOCK_ERROR;
	}
	return XPL_MODULE_CMD_OK;
#undef ASSIGN_ERR_DATA
}

void xplUnloadModule(xmlChar *name)
{
	xplExternalCommandsPtr cmds;
	int i;

	cmds = (xplExternalCommandsPtr) xmlHashLookup(loaded_modules, name);
	if (!cmds)
		return;
	if (!xprMutexAcquire(&module_locker))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	for (i = 0; i < cmds->count; i++)
		xplUnregisterCommand((const xmlChar*) cmds->commands[i].name);
	xprUnloadSharedObject(cmds->handle);
	xmlHashRemoveEntry(loaded_modules, name, NULL);
	if (!xprMutexRelease(&module_locker))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
}

xmlChar* xplModuleCmdResultToString(xplModuleCmdResult result, xmlChar *error_data)
{
	xmlChar *ret;
	int ret_len;
#define VAR_RET(s)\
	do {\
		ret_len = _scprintf(s, error_data); \
		ret = (xmlChar*) XPL_MALLOC(ret_len + 1); \
		if (!ret) \
			return NULL; \
		sprintf((char*) ret, s, error_data); \
		return ret;\
	} while(0)

	switch(result)
	{
		case XPL_MODULE_CMD_OK:
			return XPL_STRDUP("no error");
		case XPL_MODULE_CMD_MODULE_NOT_FOUND:
			return XPL_STRDUP("module not found");
		case XPL_MODULE_CMD_INVALID_MODULE_FORMAT:
			return XPL_STRDUP("invalid module format");
		case XPL_MODULE_CMD_INVALID_COMMAND_FORMAT:
			VAR_RET ("invalid command #%s format");
			break;
		case XPL_MODULE_CMD_COMMAND_NAME_CLASH:
			VAR_RET("command \"%s\" already exists");
			break;
		case XPL_MODULE_CMD_VERSION_TOO_OLD:
			VAR_RET("module version \"%s\" is no longer supported");
			break;
		case XPL_MODULE_CMD_UNSUPPORTED_VERSION:
			VAR_RET("module version \"%s\" is newer than the interpreter");
			break;
		case XPL_MODULE_CMD_INSUFFICIENT_MEMORY:
			return XPL_STRDUP("insufficient memory");
		case XPL_MODULE_CMD_COMMAND_INIT_FAILED:
			VAR_RET("command %s failed to initialize");
			break;
		case XPL_MODULE_CMD_NO_PARSER:
			return XPL_STRDUP("XPL interpreter not initialized");
		case XPL_MODULE_CMD_MODULE_ALREADY_LOADED:
			return XPL_STRDUP("module is already loaded");
		case XPL_MODULE_CMD_LOCK_ERROR:
			return XPL_STRDUP("locking error");
		default:
			DISPLAY_INTERNAL_ERROR_MESSAGE();
			return XPL_STRDUP("unknown error");
	}
}

bool xplIsModuleLoaded(const xmlChar *name)
{
	bool ret;
	if (!loaded_modules)
		return false;
	if (!xprMutexAcquire(&module_locker))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return false;
	}
	ret = xmlHashLookup(loaded_modules, name)? true: false;
	if (!xprMutexRelease(&module_locker))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	return ret;
}

typedef struct _LoadedModulesCountScannerCtxt
{
	size_t delim_len;
	size_t len;
} LoadedModulesCountScannerCtxt, *LoadedModulesCountScannerCtxtPtr;

static void loadedModulesCountScanner(void *payload, void *data, xmlChar *name)
{
	LoadedModulesCountScannerCtxtPtr ctxt = (LoadedModulesCountScannerCtxtPtr) data;
	ctxt->len += xmlStrlen(name);
	ctxt->len += ctxt->delim_len;
}

typedef struct _LoadedModulesStringScannerCtxt
{
	xmlChar *string_pos;
	const xmlChar *delimiter;
	size_t delim_len;
} LoadedModulesStringScannerCtxt, *LoadedModulesStringScannerCtxtPtr;

static void loadedModulesStringScanner(void *payload, void *data, xmlChar *name)
{
	LoadedModulesStringScannerCtxtPtr ctxt = (LoadedModulesStringScannerCtxtPtr) data;
	size_t len = xmlStrlen(name);
	memcpy(ctxt->string_pos, name, len);
	ctxt->string_pos += len;
	if (ctxt->delimiter)
	{
		memcpy(ctxt->string_pos, ctxt->delimiter, ctxt->delim_len);
		ctxt->string_pos += ctxt->delim_len;
	}
}

xmlChar* xplLoadedModulesToString(const xmlChar *delimiter)
{
	LoadedModulesCountScannerCtxt count_ctxt;
	LoadedModulesStringScannerCtxt string_ctxt;
	xmlChar *ret;

	if (!loaded_modules)
		return NULL;
	count_ctxt.len = 0;
	count_ctxt.delim_len = delimiter? xmlStrlen(delimiter): 0;
	if (!xprMutexAcquire(&module_locker))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return XPL_STRDUP("lock error");
	}
	xmlHashScan(loaded_modules, loadedModulesCountScanner, &count_ctxt);
	if (!count_ctxt.len)
	{
		if (!xprMutexRelease(&module_locker))
			DISPLAY_INTERNAL_ERROR_MESSAGE();
		return NULL; /* empty hash */
	}
	string_ctxt.delimiter = delimiter;
	string_ctxt.delim_len = count_ctxt.delim_len;
	ret = string_ctxt.string_pos = (xmlChar*) XPL_MALLOC(count_ctxt.len + 1);
	xmlHashScan(loaded_modules, loadedModulesStringScanner, &string_ctxt);
	if (!xprMutexRelease(&module_locker))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	*(string_ctxt.string_pos - count_ctxt.delim_len) = 0;
	return ret;
}

typedef struct _LoadedModulesListScannerCtxt
{
	xmlNodePtr first;
	xmlNodePtr last;
	xmlDocPtr doc;
	xmlNsPtr ns;
	xmlChar *tagname;
} LoadedModulesListScannerCtxt, *LoadedModulesListScannerCtxtPtr;

static void loadedModulesListScanner(void *payload, void *data, xmlChar *name)
{
	LoadedModulesListScannerCtxtPtr ctxt = (LoadedModulesListScannerCtxtPtr) data;
	xmlNodePtr cur;
	if (ctxt->tagname)
		cur = xmlNewDocNode(ctxt->doc, ctxt->ns, ctxt->tagname, name);
	else
		cur = xmlNewDocNode(ctxt->doc, NULL, name, NULL);
	if (ctxt->first)
	{
		cur->prev = ctxt->last;
		ctxt->last->next = cur;
		ctxt->last = cur;
	} else
		ctxt->last = ctxt->first = cur;
}

xmlNodePtr xplLoadedModulesToNodeList(const xmlChar *tagQName, xmlNodePtr parent)
{
	LoadedModulesListScannerCtxt ctxt;
	if (!loaded_modules)
		return NULL;
	if (tagQName)
	{
		EXTRACT_NS_AND_TAGNAME(tagQName, ctxt.ns, ctxt.tagname, parent)
	} else
		ctxt.tagname = NULL;
	ctxt.doc = parent->doc;
	ctxt.first = ctxt.last = NULL;
	xmlHashScan(loaded_modules, loadedModulesListScanner, &ctxt);
	return ctxt.first;
}

xmlNodePtr xplDecodeCmdBoolParam(xmlNodePtr cmd, const xmlChar *name, bool *value, bool defaultValue)
{
	xmlChar *attr = xmlGetNoNsProp(cmd, name);
	int dec_value;
	xmlNodePtr ret = NULL;

	if (!value)
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return NULL;
	}
	if (!attr)
	{
		*value = defaultValue;
		return NULL;
	}
	dec_value = xplGetBooleanValue(attr);
	switch(dec_value)
	{
	case 0:
		*value = false;
		break;
	case 1:
		*value = true;
		break;
	default:
		*value = defaultValue;
		ret = xplCreateErrorNode(cmd, BAD_CAST "invalid boolean parameter \"%s\" value \"%s\"", name, attr);
		break;
	}
	XPL_FREE(attr);
	return ret;
}
