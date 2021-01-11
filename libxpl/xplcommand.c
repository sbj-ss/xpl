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
	return !!commands;
}

static xplModuleCmdResult _registerCommandParams(xplCommandPtr cmd, xmlChar **error)
{
	int i;
	xplCmdParamPtr param;
	xmlChar **alias;

	for (i = 0; cmd->parameters[i].name; i++)
		NOOP();
	cmd->param_count = i;
	if (!(cmd->param_hash = xmlHashCreate(cmd->param_count*2)))
	{
		if (error)
			*error = BAD_CAST XPL_STRDUP("insufficient memory");
		return XPL_MODULE_CMD_INSUFFICIENT_MEMORY;
	}
	cmd->required_params = (unsigned char*) XPL_MALLOC(cmd->param_count);
	if (!cmd->required_params)
	{
		xmlHashFree(cmd->param_hash, NULL);
		cmd->param_hash = NULL;
		if (error)
			*error = BAD_CAST XPL_STRDUP("insufficient memory");
		return XPL_MODULE_CMD_INSUFFICIENT_MEMORY;
	}
	for (i = 0; i < cmd->param_count; i++)
	{
		param = &(cmd->parameters[i]);
		if (xmlHashAddEntry(cmd->param_hash, param->name, param))
		{
			xmlHashFree(cmd->param_hash, NULL);
			cmd->param_hash = NULL;
			if (error)
				*error = xplFormatMessage(BAD_CAST "duplicate parameter name '%'s", param->name);
			return XPL_MODULE_CMD_PARAM_NAME_CLASH;
		}
		for (alias = param->aliases; alias && *alias; alias++)
			if (xmlHashAddEntry(cmd->param_hash, *alias, param))
			{
				xmlHashFree(cmd->param_hash, NULL);
				cmd->param_hash = NULL;
				if (error)
					*error = xplFormatMessage(BAD_CAST "duplicate alias name '%s'", *alias);
				return XPL_MODULE_CMD_PARAM_NAME_CLASH;
			}
		param->value_offset = (uintptr_t) param->value_stencil - (uintptr_t) cmd->params_stencil;
		cmd->required_params[i] = param->required;
		param->index = i;
	}
	if (error)
		*error = NULL;
	return XPL_MODULE_CMD_OK;
}

xplModuleCmdResult xplRegisterCommand(const xmlChar *name, xplCommandPtr cmd, xmlChar **error)
{
	xplModuleCmdResult ret;

	if (!commands)
		return XPL_MODULE_CMD_NO_PARSER;
	if (!cmd)
		return XPL_MODULE_CMD_WRONG_PARAMS;
	if (cmd->flags & XPL_CMD_FLAG_INITIALIZED) /* already initialized with another name */
		return XPL_MODULE_CMD_OK;
	/* Don't allow overrides */
	if (xmlHashAddEntry(commands, name, cmd) == -1)
		return XPL_MODULE_CMD_COMMAND_NAME_CLASH;
	if (cmd->initializer)
		if (!cmd->initializer(NULL, error))
		{
			xmlHashRemoveEntry(commands, name, NULL);
			return XPL_MODULE_CMD_COMMAND_INIT_FAILED;
		}
	if (cmd->params_stencil)
		if ((ret = _registerCommandParams(cmd, error)) != XPL_MODULE_CMD_OK)
			return ret;
	cmd->flags |= XPL_CMD_FLAG_INITIALIZED;
	return XPL_MODULE_CMD_OK;
}

void xplUnregisterCommand(const xmlChar *name)
{
	xplCommandPtr cmd;

	if (!commands)
		return;
	if (!name)
		return;
	cmd = (xplCommandPtr) xmlHashLookup(commands, name);
	if (!cmd)
		return;
	if (!(cmd->flags & XPL_CMD_FLAG_INITIALIZED)) /* aliased command */
		return;
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
	cmd->flags &= ~XPL_CMD_FLAG_INITIALIZED;
}

xplCommandPtr xplGetCommand(xmlNodePtr el)
{
	return (xplCommandPtr) xmlHashLookup(commands, el->name);
}

static const xmlChar* builtins[] =
{
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
	return !!xmlHashLookup(commands, name);
}

typedef struct _CommandListScannerContext
{
	xmlDocPtr doc;
	xplQName tagname;
	xmlNodePtr head, tail;
} CommandListScannerContext, *PCommandListScannerContext;

static void commandListScanner(void *payload, void *data, XML_HCBNC xmlChar *name)
{
	xmlNodePtr cur;
	PCommandListScannerContext ctxt = (PCommandListScannerContext) data;
	UNUSED_PARAM(payload);

	if (ctxt->tagname.ncname)
		cur = xmlNewDocNode(ctxt->doc, ctxt->tagname.ns, ctxt->tagname.ncname, name);
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

xmlNodePtr xplSupportedCommandsToList(xmlNodePtr parent, const xplQName tagname)
{
	CommandListScannerContext ctxt;
	unsigned int i;

	ctxt.doc = parent->doc;
	ctxt.tagname = tagname;
	ctxt.head = NULL;
	for (i = 0; i < sizeof(builtins) / sizeof(builtins[0]); i++)
		commandListScanner(NULL, &ctxt, BAD_CAST builtins[i]);
	xmlHashScan(commands, commandListScanner, &ctxt);
	return ctxt.head;
}

static void commandDeallocator(void *payload, XML_HCBNC xmlChar *name)
{
	xplCommandPtr cmd = (xplCommandPtr) payload;

	UNUSED_PARAM(name);
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

typedef xmlChar* (*xplParamValueGetter)(xplCommandInfoPtr info, xplCmdParamPtr param, xmlChar **raw_value);

static xmlChar* xplGetParamStringValue(xplCommandInfoPtr info, xplCmdParamPtr param, xmlChar **raw_value)
{
	*((xmlChar**) ((uintptr_t) info->params + param->value_offset)) = *raw_value;
	*raw_value = NULL;
	return NULL;
}

static xmlChar* xplGetParamIntValue(xplCommandInfoPtr info, xplCmdParamPtr param, xmlChar **raw_value)
{
	char *end_ptr;

	*((int*) ((uintptr_t) info->params + param->value_offset)) = strtol((char*) *raw_value, &end_ptr, 0);
	if (*end_ptr)
		return xplFormatMessage(BAD_CAST "'%s': not a valid integer", *raw_value);
	else
		return NULL;
}

static xmlChar* xplGetParamBoolValue(xplCommandInfoPtr info, xplCmdParamPtr param, xmlChar **raw_value)
{
	int int_value;

	int_value = xplGetBooleanValue(*raw_value);
	if (int_value < 0)
		return xplFormatMessage(BAD_CAST "'%s': not a valid boolean", *raw_value);
	else {
		*((bool*) ((uintptr_t) info->params + param->value_offset)) = (bool) int_value;
		return NULL;
	}
}

static xmlChar* xplGetParamDictValue(xplCommandInfoPtr info, xplCmdParamPtr param, xmlChar **raw_value)
{
	xplCmdParamDictValuePtr dict_value;
	int int_value;

	dict_value = param->extra.dict_values;
	while (dict_value->name)
	{
		if (!xmlStrcasecmp(dict_value->name, *raw_value))
		{
			int_value = dict_value->value;
			break;
		}
		dict_value++;
	}
	if (!dict_value->name) /* nothing matched */
		return xplFormatMessage(BAD_CAST "'%s': not in the allowed list", *raw_value);
	else {
		*((int*) ((uintptr_t) info->params + param->value_offset)) = int_value;
		return NULL;
	}
}

static xmlChar* xplGetParamXPathValue(xplCommandInfoPtr info, xplCmdParamPtr param, xmlChar **raw_value)
{
	xmlXPathObjectPtr xpath_obj;

	xpath_obj = xplSelectNodes(info, info->element, *raw_value);
	if (!xpath_obj)
		return xplFormatMessage(BAD_CAST "invalid XPath expression '%s'", *raw_value);
	*((xmlXPathObjectPtr*) ((uintptr_t) info->params + param->value_offset)) = xpath_obj;
	if (param->extra.xpath_type == XPL_CMD_PARAM_XPATH_TYPE_NODESET)
	{
		if (xpath_obj->type != XPATH_NODESET)
			return xplFormatMessage(BAD_CAST "XPath query '%s' evaluated to non-nodeset type", *raw_value);
	} else if (param->extra.xpath_type == XPL_CMD_PARAM_XPATH_TYPE_SCALAR)
		if (xpath_obj->type != XPATH_BOOLEAN && xpath_obj->type != XPATH_NUMBER && xpath_obj->type != XPATH_STRING)
			return xplFormatMessage(BAD_CAST "XPath query '%s' evaluated to a non-scalar value", *raw_value);
	xpath_obj->user = *raw_value;
	*raw_value = NULL;
	return NULL;
}

static xmlChar* xplGetParamQNameValue(xplCommandInfoPtr info, xplCmdParamPtr param, xmlChar **raw_value)
{
	xplQNamePtr dst;
	xmlChar *prefix, *error = NULL;

	if (xmlValidateQName(*raw_value, 0))
		return xplFormatMessage(BAD_CAST "invalid QName/NCName '%s'", *raw_value);
	dst = (xplQNamePtr) ((uintptr_t) info->params + param->value_offset);
	dst->ncname = xmlSplitQName2(*raw_value, &prefix);
	if (!dst->ncname) /* not a QName */
	{
		dst->ncname = *raw_value;
		*raw_value = NULL;
		dst->ns = NULL;
	} else {
		dst->ns = xmlSearchNs(info->element->doc, info->element->parent, prefix);
		if (!dst->ns)
		{
			dst->ns = xmlSearchNs(info->element->doc, info->element, prefix);
			if (!dst->ns)
				error = xplFormatMessage(BAD_CAST "invalid namespace prefix '%s'", prefix);
			else /* namespace is declared on the command element and it will be gone soon */
				dst->ns = xmlNewNs(info->element->parent, dst->ns->href, dst->ns->prefix);
		}
	}
	XPL_FREE(prefix);
	return error;
}

static xmlChar* xplGetParamCustomIntValue(xplCommandInfoPtr info, xplCmdParamPtr param, xmlChar **raw_value)
{
	xmlChar *error;
	int value;

	if (!param->extra.int_getter)
		return BAD_CAST XPL_STRDUP("int_getter is NULL");
	if (!(error = param->extra.int_getter(info, *raw_value, &value)))
		*((int*) ((uintptr_t) info->params + param->value_offset)) = value;
	return error;
}

static xmlChar* xplGetParamCustomPtrValue(xplCommandInfoPtr info, xplCmdParamPtr param, xmlChar **raw_value)
{
	xmlChar *error;
	void* value;

	if (!param->extra.ptr_fn.getter)
		return BAD_CAST XPL_STRDUP("ptr_fn.getter is NULL");
	if (!(error = param->extra.ptr_fn.getter(info, *raw_value, &value)))
		*((void**) ((uintptr_t) info->params + param->value_offset)) = value;
	return error;
}

static xmlChar* xplGetParamNCNameValue(xplCommandInfoPtr info, xplCmdParamPtr param, xmlChar **raw_value)
{
	if (xmlValidateNCName(*raw_value, 0))
		return xplFormatMessage(BAD_CAST "invalid NCName '%s'", *raw_value);
	*((xmlChar**) ((uintptr_t) info->params + param->value_offset)) = *raw_value;
	*raw_value = NULL;
	return NULL;
}

static const xplParamValueGetter value_getters[] =
{
	[XPL_CMD_PARAM_TYPE_STRING] = xplGetParamStringValue,
	[XPL_CMD_PARAM_TYPE_INT] = xplGetParamIntValue,
	[XPL_CMD_PARAM_TYPE_BOOL] = xplGetParamBoolValue,
	[XPL_CMD_PARAM_TYPE_DICT] = xplGetParamDictValue,
	[XPL_CMD_PARAM_TYPE_XPATH] = xplGetParamXPathValue,
	[XPL_CMD_PARAM_TYPE_QNAME] = xplGetParamQNameValue,
	[XPL_CMD_PARAM_TYPE_INT_CUSTOM_GETTER] = xplGetParamCustomIntValue,
	[XPL_CMD_PARAM_TYPE_PTR_CUSTOM_GETTER] = xplGetParamCustomPtrValue,
	[XPL_CMD_PARAM_TYPE_NCNAME] = xplGetParamNCNameValue
};


xmlNodePtr xplGetCommandParams(xplCommandPtr command, xplCommandInfoPtr commandInfo)
{
	xmlAttrPtr attr = commandInfo->element->properties;
	xplCmdParamPtr param;
	xmlChar *value_text = NULL, *error = NULL;
	int i;
	unsigned char *required_params = NULL;
	xmlNodePtr ret = NULL;

	if (!command->param_hash)
		return xplCreateErrorNode(commandInfo->element, BAD_CAST "command->param_hash is NULL");
	if (!commandInfo->params)
		commandInfo->params = XPL_MALLOC(command->stencil_size);
	if (!commandInfo->params)
		return xplCreateErrorNode(commandInfo->element, BAD_CAST "out of memory");
	memcpy(commandInfo->params, command->params_stencil, command->stencil_size); /* set defaults */

	required_params = (unsigned char*) XPL_MALLOC(command->param_count);
	if (!required_params)
		return xplCreateErrorNode(commandInfo->element, BAD_CAST "insufficient memory");
	memcpy(required_params, command->required_params, command->param_count);

	while (attr)
	{
		if ((param = (xplCmdParamPtr) xmlHashLookup(command->param_hash, attr->name)))
		{
			if ((int) param->type < 0 || (int) param->type > XPL_CMD_PARAM_TYPE_MAX)
			{
				DISPLAY_INTERNAL_ERROR_MESSAGE();
				error = xplFormatMessage(BAD_CAST "unknown parameter type '%d'", (int) param->type);
			} else {
				value_text = xplGetPropValue(attr);
				error = value_getters[param->type](commandInfo, param, &value_text);
			}
			required_params[param->index] = 0;
			if (value_text)
			{
				XPL_FREE(value_text);
				value_text = NULL;
			}
			if (error)
			{
				ret = xplCreateErrorNode(commandInfo->element, BAD_CAST "parameter '%s': %s", attr->name, error);
				XPL_FREE(error);
				goto done;
			}
		}
		attr = attr->next;
	}
	for (i = 0; i < command->param_count; i++)
		if (required_params[i])
		{
			ret = xplCreateErrorNode(commandInfo->element, BAD_CAST "parameter '%s' must be specified", command->parameters[i].name);
			goto done;
		}
done:
	if (required_params)
		XPL_FREE(required_params);
	return ret;
}

xmlNodePtr xplFillCommandInfo(xplCommandPtr command, xplCommandInfoPtr info, bool inPrologue)
{
	xmlNodePtr error;

	if (((command->flags & XPL_CMD_FLAG_PARAMS_FOR_PROLOGUE) && inPrologue)
		||
		((command->flags & XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE) && !inPrologue)
	) {
		if (info->params)
			xplClearCommandParams(command, info->params);
		if ((error = xplGetCommandParams(command, info)))
			return error;
	}
	if ((command->flags & XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE) && !inPrologue)
	{
		if (info->element->children)
		{
			if (!xplCheckNodeListForText(info->element->children))
				return xplCreateErrorNode(info->element, BAD_CAST "command content is non-text");
			info->content = xmlNodeListGetString(info->element->doc, info->element->children, 1);
			if (!info->content && (command->flags & XPL_CMD_FLAG_REQUIRE_CONTENT))
				return xplCreateErrorNode(info->element, BAD_CAST "command content is empty");
		} else
			info->content = NULL;
	}
	return NULL;
}

static void _paramClearValueScanner(void *payload, void *data, XML_HCBNC xmlChar *name)
{
	xplCmdParamPtr param = (xplCmdParamPtr) payload;
	char **value, **default_value;
	xmlXPathObjectPtr *xpath_obj;
	xplQNamePtr qname, default_qname;

	UNUSED_PARAM(name);
	switch (param->type)
	{
		case XPL_CMD_PARAM_TYPE_STRING:
		case XPL_CMD_PARAM_TYPE_PTR_CUSTOM_GETTER:
			value = (char**) ((uintptr_t) data + param->value_offset);
			default_value = (char**) param->value_stencil;
			if (*value && *value != *default_value)
			{
				if (param->type == XPL_CMD_PARAM_TYPE_STRING)
					XPL_FREE(*value);
				else if (param->type == XPL_CMD_PARAM_TYPE_PTR_CUSTOM_GETTER && param->extra.ptr_fn.deallocator)
					param->extra.ptr_fn.deallocator(*value);
				*value = NULL;
			}
			break;
		case XPL_CMD_PARAM_TYPE_XPATH:
			xpath_obj = (xmlXPathObjectPtr*) ((uintptr_t) data + param->value_offset);
			if (*xpath_obj)
			{
				if ((*xpath_obj)->user)
					XPL_FREE((*xpath_obj)->user);
				xmlXPathFreeObject(*xpath_obj);
				*xpath_obj = NULL;
			}
			break;
		case XPL_CMD_PARAM_TYPE_QNAME:
			qname = (xplQNamePtr) ((uintptr_t) data + param->value_offset);
			default_qname = (xplQNamePtr) param->value_stencil;
			if (qname->ncname && qname->ncname != default_qname->ncname)
				XPL_FREE(qname->ncname);
			break;
		default:
			break;
	}
}

void xplClearCommandParams(xplCommandPtr command, void *values)
{
	if (!command->param_hash || !values)
		return;
	xmlHashScan(command->param_hash, _paramClearValueScanner, values);
}

void xplClearCommandInfo(xplCommandPtr command, xplCommandInfoPtr info)
{
	if (info->content)
		XPL_FREE(info->content);
	if (info->params)
	{
		xplClearCommandParams(command, info->params);
		XPL_FREE(info->params);
	}
	if (info->required_params)
		XPL_FREE(info->required_params);
}

xmlXPathObjectPtr xplSelectNodes(xplCommandInfoPtr commandInfo, xmlNodePtr src, xmlChar *expr)
{
	if (!src | !commandInfo)
		return NULL;
	return xplSelectNodesWithCtxt(commandInfo->xpath_ctxt, src, expr);
}

static void FreeModulesCallback(void *payload, XML_HCBNC xmlChar *name)
{
	UNUSED_PARAM(payload);
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

void xplUnloadModule(const xmlChar *name)
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
	return loaded_modules && xmlHashLookup(loaded_modules, name);
}

typedef struct _LoadedModulesCountScannerCtxt
{
	size_t delim_len;
	size_t len;
} LoadedModulesCountScannerCtxt, *LoadedModulesCountScannerCtxtPtr;

static void loadedModulesCountScanner(void *payload, void *data, XML_HCBNC xmlChar *name)
{
	UNUSED_PARAM(payload);
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

static void loadedModulesStringScanner(void *payload, void *data, XML_HCBNC xmlChar *name)
{
	LoadedModulesStringScannerCtxtPtr ctxt = (LoadedModulesStringScannerCtxtPtr) data;
	size_t len = xmlStrlen(name);

	UNUSED_PARAM(payload);
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
	xplQName tagname;
} LoadedModulesListScannerCtxt, *LoadedModulesListScannerCtxtPtr;

static void loadedModulesListScanner(void *payload, void *data, XML_HCBNC xmlChar *name)
{
	LoadedModulesListScannerCtxtPtr ctxt = (LoadedModulesListScannerCtxtPtr) data;
	xmlNodePtr cur;

	UNUSED_PARAM(payload);
	if (ctxt->tagname.ncname)
		cur = xmlNewDocNode(ctxt->doc, ctxt->tagname.ns, ctxt->tagname.ncname, name);
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

xmlNodePtr xplLoadedModulesToNodeList(const xplQName tagname, xmlNodePtr parent)
{
	LoadedModulesListScannerCtxt ctxt;

	if (!loaded_modules)
		return NULL;
	ctxt.tagname = tagname;
	ctxt.doc = parent->doc;
	ctxt.first = ctxt.last = NULL;
	xmlHashScan(loaded_modules, loadedModulesListScanner, &ctxt);
	return ctxt.first;
}

xmlNodePtr xplDecodeCmdBoolParam(xmlNodePtr cmd, const xmlChar *name, bool *value, bool defaultValue) // TODO remove
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
