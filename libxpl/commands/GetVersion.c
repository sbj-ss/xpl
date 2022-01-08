#include <stdio.h>
#include <libxpl/abstraction/xef.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>
#include <libxpl/xplversion.h>

void xplCmdGetVersionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef enum _xplCmdGetVersionPart
{
	XCGV_PART_MAJOR,
	XCGV_PART_MINOR,
	XCGV_PART_FULL,
	XCGV_PART_DETAILED,
	XCGV_PART_LIBS,
	XCGV_PART_XEF
} xplCmdGetVersionPart;

static xplCmdParamDictValue part_dict[] =
{
	{ BAD_CAST "minor", XCGV_PART_MINOR },
	{ BAD_CAST "major", XCGV_PART_MAJOR },
	{ BAD_CAST "full", XCGV_PART_FULL },
	{ BAD_CAST "detailed", XCGV_PART_DETAILED },
	{ BAD_CAST "libs", XCGV_PART_LIBS },
	{ BAD_CAST "xef", XCGV_PART_XEF },
	{ NULL, 0 }
};

typedef struct _xplCmdGetVersionParams
{
	xplCmdGetVersionPart part;
	xplQName tagname;
	bool repeat;
} xplCmdGetVersionParams, *xplCmdGetVersionParamsPtr;

static const xplCmdGetVersionParams params_stencil =
{
	.part = XCGV_PART_FULL,
	.tagname = { NULL, NULL },
	.repeat = true
};

xplCommand xplGetVersionCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdGetVersionEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdGetVersionParams),
	.parameters = {
		{
			.name = BAD_CAST "part",
			.type = XPL_CMD_PARAM_TYPE_DICT,
			.extra.dict_values = part_dict,
			.value_stencil = &params_stencil.part
		}, {
			.name = BAD_CAST "tagname",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.value_stencil = &params_stencil.tagname
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = NULL
		}
	}
};

static xmlNodePtr _getXplAtomicVersion(xmlDocPtr doc, xplCmdGetVersionPart part)
{
	xmlChar *ver;
	xmlNodePtr ret;

	switch (part)
	{
		case XCGV_PART_MAJOR:
			ver = (xmlChar*) XPL_MALLOC(12);
			sprintf((char*) ver, "%d", XPL_VERSION_MAJOR);
			break;
		case XCGV_PART_MINOR:
			ver = (xmlChar*) XPL_MALLOC(12);
			sprintf((char*) ver, "%d", XPL_VERSION_MINOR);
			break;
		case XCGV_PART_FULL:
			ver = BAD_CAST XPL_STRDUP((char*) XPL_VERSION_FULL);
			break;
		default:
			DISPLAY_INTERNAL_ERROR_MESSAGE();
			ver = BAD_CAST XPL_STRDUP("unknown part requested");
	}
	ret = xmlNewDocText(doc, NULL);
	ret->content = ver;
	return ret;
}

static xmlNodePtr _getXplDetailedVersion(xmlDocPtr doc, xplQName tagname)
{
	xmlNodePtr ret;
	xmlChar ver[12];

	ret = xmlNewDocNode(doc, tagname.ns, tagname.ncname, NULL);
	sprintf((char*) ver, "%d", XPL_VERSION_MAJOR);
	xmlNewProp(ret, BAD_CAST "major", ver);
	sprintf((char*) ver, "%d", XPL_VERSION_MINOR);
	xmlNewProp(ret, BAD_CAST "minor", ver);
	xmlNewProp(ret, BAD_CAST "debug", BAD_CAST (XPL_VERSION_EFFECTIVE_FLAG_DEBUG? "true": "false"));
	xmlNewProp(ret, BAD_CAST "beta", BAD_CAST (XPL_VERSION_EFFECTIVE_FLAG_BETA? "true": "false"));
	return ret;
}

static const xplQName default_library_name = { NULL, BAD_CAST "library" };
static const xplQName default_xef_name = { NULL, BAD_CAST "feature" };
static const xplQName default_detailed_name = { NULL, BAD_CAST "version" };

void xplCmdGetVersionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdGetVersionParamsPtr params = (xplCmdGetVersionParamsPtr) commandInfo->params;
	xplQName tagname;
	xmlNodePtr nodes;

	if (params->part == XCGV_PART_LIBS)
	{
		tagname = params->tagname.ncname? params->tagname: default_library_name;
		nodes = xplLibraryVersionsToNodeList(commandInfo->element->doc, tagname, xplGetCompiledLibraryVersions(), xplGetRunningLibraryVersions());
		ASSIGN_RESULT(nodes, params->repeat, true);
	} else if (params->part == XCGV_PART_XEF) {
		tagname = params->tagname.ncname? params->tagname: default_xef_name;
		nodes = xplXefImplementationsToNodeList(commandInfo->element->doc, tagname, xplGetXefImplementations());
		ASSIGN_RESULT(nodes, params->repeat, true);
	} else if (params->part == XCGV_PART_DETAILED) {
		tagname = params->tagname.ncname? params->tagname: default_detailed_name;
		nodes = _getXplDetailedVersion(commandInfo->element->doc, tagname);
		ASSIGN_RESULT(nodes, params->repeat, true);
	} else
		ASSIGN_RESULT(_getXplAtomicVersion(commandInfo->element->doc, params->part), false, true);
}
