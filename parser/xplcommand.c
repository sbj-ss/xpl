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

static xplModuleCmdResult _registerCommandParams(xplCommandPtr cmd, xmlChar **error)
{
	int i;
	xplCmdParamPtr param;
	xmlChar **alias;

	// TODO error details in **error
	if (!(cmd->param_hash = xmlHashCreate(16)))
		return XPL_MODULE_CMD_INSUFFICIENT_MEMORY;
	for (i = 0; cmd->parameters[i].name; i++)
		NOOP();
	cmd->param_count = i;
	cmd->required_params = (unsigned char*) XPL_MALLOC(cmd->param_count);
	if (!cmd->required_params)
	{
		xmlHashFree(cmd->param_hash, NULL);
		cmd->param_hash = NULL;
		return XPL_MODULE_CMD_INSUFFICIENT_MEMORY;
	}
	for (i = 0; i < cmd->param_count; i++)
	{
		param = &(cmd->parameters[i]);
		if (xmlHashAddEntry(cmd->param_hash, param->name, param))
		{
			xmlHashFree(cmd->param_hash, NULL);
			cmd->param_hash = NULL;
			return XPL_MODULE_CMD_PARAM_NAME_CLASH;
		}
		for (alias = param->aliases; alias && *alias; alias++)
			if (xmlHashAddEntry(cmd->param_hash, *alias, param))
			{
				xmlHashFree(cmd->param_hash, NULL);
				cmd->param_hash = NULL;
				return XPL_MODULE_CMD_PARAM_NAME_CLASH;
			}
		param->value_offset = (uintptr_t) param->value_stencil - (uintptr_t) cmd->params_stencil;
		cmd->required_params[i] = param->required;
		param->index = i;
	}
	return XPL_MODULE_CMD_OK;
}

xplModuleCmdResult xplRegisterCommand(const xmlChar *name, xplCommandPtr cmd, xmlChar **error)
{
	xplModuleCmdResult ret;

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
	if (cmd->parameters && cmd->params_stencil)
		if ((ret = _registerCommandParams(cmd, error)) != XPL_MODULE_CMD_OK)
			return ret;
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
		if (cmd->param_hash)
		{
			xmlHashFree(cmd->param_hash, NULL);
			cmd->param_hash = NULL;
		}
		if (cmd->required_params)
			XPL_FREE(cmd->required_params);
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

xmlNodePtr xplGetCommandParams(xplCommandPtr command, xplCommandInfoPtr commandInfo, void *values)
{
	xmlAttrPtr attr = commandInfo->element->properties;
	xplCmdParamPtr param;
	xmlChar *value_text = NULL;
	char *end_ptr;
	int int_value, i;
	xplCmdParamDictValuePtr dict_value;
	unsigned char *required_params = NULL;
	xmlNodePtr ret = NULL;
	xmlXPathObjectPtr xpath_obj;

	if (!command->param_hash)
		return xplCreateErrorNode(commandInfo->element, BAD_CAST "command->param_hash is NULL");
	memcpy(values, command->params_stencil, command->stencil_size); /* set defaults */

	required_params = (unsigned char*) XPL_MALLOC(command->param_count);
	if (!required_params)
		return xplCreateErrorNode(commandInfo->element, BAD_CAST "insufficient memory");
	memcpy(required_params, command->required_params, command->param_count);

	while (attr)
	{
		if ((param = (xplCmdParamPtr) xmlHashLookup(command->param_hash, attr->name)))
		{
			value_text = xplGetPropValue(attr);
			switch (param->type)
			{
				case XPL_CMD_PARAM_TYPE_STRING:
					*((xmlChar**) ((uintptr_t) values + param->value_offset)) = value_text;
					break;
				case XPL_CMD_PARAM_TYPE_INTEGER:
					*((int*) ((uintptr_t) values + param->value_offset)) = strtol((char*) value_text, &end_ptr, 0);
					if (*end_ptr)
					{
						ret = xplCreateErrorNode(commandInfo->element, BAD_CAST "parameter '%s', value '%s': not a valid integer", attr->name, value_text);
						goto done;
					}
					break;
				case XPL_CMD_PARAM_TYPE_BOOL:
					int_value = xplGetBooleanValue(value_text);
					if (int_value < 0)
					{
						ret = xplCreateErrorNode(commandInfo->element, BAD_CAST "parameter '%s', value '%s': not a valid boolean", attr->name, value_text);
						goto done;
					}
					*((bool*) ((uintptr_t) values + param->value_offset)) = (bool) int_value;
					break;
				case XPL_CMD_PARAM_TYPE_DICT:
					dict_value = param->dict_values;
					while (dict_value->name)
					{
						if (!xmlStrcasecmp(dict_value->name, value_text))
						{
							int_value = dict_value->value;
							break;
						}
						dict_value++;
					}
					if (!dict_value->name) /* nothing matched */
					{
						ret = xplCreateErrorNode(commandInfo->element, BAD_CAST "parameter '%s', value '%s': not in the allowed list", attr->name, value_text);
						goto done;
					}
					*((int*) ((uintptr_t) values + param->value_offset)) = int_value;
					break;
				case XPL_CMD_PARAM_TYPE_XPATH:
					xpath_obj = xplSelectNodes(commandInfo, commandInfo->element, value_text);
					if (!xpath_obj)
					{
						ret = xplCreateErrorNode(commandInfo->element, BAD_CAST "parameter '%s': invalid XPath expression '%s'", attr->name, value_text);
						goto done;
					}
					*((xmlXPathObjectPtr*) ((uintptr_t) values + param->value_offset)) = xpath_obj;
					if (param->xpath_type == XPL_CMD_PARAM_XPATH_TYPE_NODESET)
					{
						if (xpath_obj->type != XPATH_NODESET)
						{
							ret = xplCreateErrorNode(commandInfo->element, BAD_CAST "XPath query '%s' evaluated to non-nodeset type", value_text);
							goto done;
						}
					} else if (param->xpath_type == XPL_CMD_PARAM_XPATH_TYPE_SCALAR)
						if (xpath_obj->type != XPATH_BOOLEAN && xpath_obj->type != XPATH_NUMBER && xpath_obj->type != XPATH_STRING)
						{
							ret = xplCreateErrorNode(commandInfo->element, BAD_CAST "XPath query '%s' evaluated to a nodeset but a scalar value is needed", value_text);
							goto done;
						}
					break;
				default:
					DISPLAY_INTERNAL_ERROR_MESSAGE();
					XPL_FREE(value_text);
			}
			required_params[param->index] = 0;
			XPL_FREE(value_text);
			value_text = NULL;
		}
		attr = attr->next;
	}
	for (i = 0; i < command->param_count; i++)
		if (required_params[i])
		{
			ret = xplCreateErrorNode(commandInfo->element, BAD_CAST "parameter %s must be set", command->parameters[i].name);
			goto done;
		}
done:
	if (value_text)
		XPL_FREE(value_text);
	if (required_params)
		XPL_FREE(required_params);
	return ret;
}

static void _paramCleanValueScanner(void *payload, void *data, xmlChar *name)
{
	xplCmdParamPtr param = (xplCmdParamPtr) payload;
	char **value, **default_value;
	xmlXPathObjectPtr *xpath_obj;

	if (param->type == XPL_CMD_PARAM_TYPE_STRING)
	{
		value = (char**) ((uintptr_t) data + param->value_offset);
		default_value = (char**) ((uintptr_t) data + param->value_offset);
		if (*value && *value != *default_value)
		{
			XPL_FREE(*value);
			*value = NULL;
		}
	} else if (param->type == XPL_CMD_PARAM_TYPE_XPATH) {
		xpath_obj = (xmlXPathObjectPtr*) ((uintptr_t) data + param->value_offset);
		if (*xpath_obj)
		{
			xmlXPathFreeObject(*xpath_obj);
			*xpath_obj = NULL;
		}
	}
}

void xplClearCommandParams(xplCommandPtr command, void *values)
{
	if (!command->param_hash || !values)
		return;
	xmlHashScan(command->param_hash, _paramCleanValueScanner, values);
}

xmlXPathObjectPtr xplSelectNodes(xplCommandInfoPtr commandInfo, xmlNodePtr src, xmlChar *expr)
{
	if (!src | !commandInfo)
		return NULL;
	return xplSelectNodesWithCtxt(commandInfo->xpath_ctxt, src, expr);
}

static void FreeModulesCallback(void *payload, xmlChar *name)
{
	xplUnloadModule(name);
}

void xplLoadableModulesCleanup(void)
{
	xmlHashFree(loaded_modules, FreeModulesCallback);
	if (!xprMutexCleanup(&module_locker))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
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
	xplModuleCmdResult ret;
	xmlChar *cmd_error;
	void *prev;

	if (!loaded_modules)
	{
		ASSIGN_ERR_DATA(BAD_CAST XPL_STRDUP("XPL interpreter not initialized"));
		return XPL_MODULE_CMD_NO_PARSER;
	}
	if (!xprMutexAcquire(&module_locker))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		ASSIGN_ERR_DATA(BAD_CAST XPL_STRDUP("locking error"));
		return XPL_MODULE_CMD_LOCK_ERROR;
	}
	prev = xmlHashLookup(loaded_modules, name);
	if (!xprMutexRelease(&module_locker))
	{
		// don't fail here
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	}
	if (prev)
	{
		ASSIGN_ERR_DATA(xplFormatMessage(BAD_CAST "module %s is already loaded", name));
		return XPL_MODULE_CMD_MODULE_ALREADY_LOADED;
	}
	ASSIGN_ERR_DATA(NULL);
	path_with_ext = (xmlChar*) XPL_MALLOC(xmlStrlen(name) + xmlStrlen(XPR_SHARED_OBJECT_EXT) + 1);
	if (!path_with_ext)
	{
		ASSIGN_ERR_DATA(BAD_CAST XPL_STRDUP("insufficient memory for module path"));
		return XPL_MODULE_CMD_INSUFFICIENT_MEMORY;
	}
	*path_with_ext = 0;
	strcpy((char*) path_with_ext, (char*) name);
	strcat((char*) path_with_ext, (char*) XPR_SHARED_OBJECT_EXT);
	hmodule = xprLoadSharedObject(path_with_ext);
	if (!hmodule)
	{
		ASSIGN_ERR_DATA(xplFormatMessage(BAD_CAST "module %s not found", path_with_ext));
		XPL_FREE(path_with_ext);
		return XPL_MODULE_CMD_MODULE_NOT_FOUND;
	}
	XPL_FREE(path_with_ext);
	get_commands_func = (GetCommandsFunc) xprGetProcAddress(hmodule, "GetCommands");
	if (!get_commands_func)
	{
		xprUnloadSharedObject(hmodule);
		ASSIGN_ERR_DATA(BAD_CAST XPL_STRDUP("function GetCommands not found in module"));
		return XPL_MODULE_CMD_INVALID_MODULE_FORMAT;
	}
	cmds = get_commands_func();
	if (!cmds || cmds->magic != PLUGGABLE_MODULE_MAGIC)
	{
		xprUnloadSharedObject(hmodule);
		ASSIGN_ERR_DATA(BAD_CAST XPL_STRDUP("wrong module signature"));
		return XPL_MODULE_CMD_INVALID_MODULE_FORMAT;
	}
	if (cmds->version > PLUGGABLE_MODULE_VERSION)
	{
		xprUnloadSharedObject(hmodule);
		ASSIGN_ERR_DATA(xplFormatMessage(BAD_CAST "module version (%d) is newer than the interpreter (%d)", cmds->version, PLUGGABLE_MODULE_VERSION));
		return XPL_MODULE_CMD_UNSUPPORTED_VERSION;
	}
	if (cmds->version < PLUGGABLE_MODULE_VERSION)
	{
		xprUnloadSharedObject(hmodule);
		ASSIGN_ERR_DATA(xplFormatMessage(BAD_CAST "module version (%d) is too old", cmds->version));
		return XPL_MODULE_CMD_VERSION_TOO_OLD;
	}
	cmds->handle = (void*) hmodule;
	if (!xprMutexAcquire(&module_locker))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		ASSIGN_ERR_DATA(BAD_CAST XPL_STRDUP("locking error"));
		xprUnloadSharedObject(hmodule);
		return XPL_MODULE_CMD_LOCK_ERROR;
	}
	for (i = 0; i < cmds->count; i++)
	{
		if (cmds->commands[i].magic != PLUGGABLE_MODULE_MAGIC)
		{
			ASSIGN_ERR_DATA(xplFormatMessage(BAD_CAST "command #%d: wrong signature", i));
			xplUnloadModule(name);
			if (!xprMutexRelease(&module_locker))
				DISPLAY_INTERNAL_ERROR_MESSAGE();
			return XPL_MODULE_CMD_INVALID_COMMAND_FORMAT;
		}
		if ((ret = xplRegisterCommand((const xmlChar*) cmds->commands[i].name, cmds->commands[i].cmd, &cmd_error)) != XPL_MODULE_CMD_OK)
		{
			ASSIGN_ERR_DATA(xplFormatMessage(BAD_CAST "command #%d (%s) failed to initialize: %s", i, cmds->commands[i].name, cmd_error));
			if (cmd_error)
				XPL_FREE(cmd_error);
			xplUnloadModule(name);
			if (!xprMutexRelease(&module_locker))
				DISPLAY_INTERNAL_ERROR_MESSAGE();
			return ret;
		}
	}
	xmlHashAddEntry(loaded_modules, name, cmds);
	if (!xprMutexRelease(&module_locker))
	{
		// don't fail here
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	}
	ASSIGN_ERR_DATA(BAD_CAST XPL_STRDUP("no error"));
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
		return BAD_CAST XPL_STRDUP("lock error");
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
