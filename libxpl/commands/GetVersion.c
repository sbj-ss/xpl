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
	XCGV_PART_LIBS
} xplCmdGetVersionPart;

static xplCmdParamDictValue part_dict[] =
{
	{ BAD_CAST "minor", XCGV_PART_MINOR },
	{ BAD_CAST "major", XCGV_PART_MAJOR },
	{ BAD_CAST "full", XCGV_PART_FULL },
	{ BAD_CAST "libs", XCGV_PART_LIBS },
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
	.tagname = { NULL, BAD_CAST "library" },
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

static xmlNodePtr _getXplVersion(xmlDocPtr doc, xplCmdGetVersionPart part)
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

void xplCmdGetVersionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdGetVersionParamsPtr params = (xplCmdGetVersionParamsPtr) commandInfo->params;
	xmlNodePtr libs;

	if (params->part == XCGV_PART_LIBS)
	{
		libs = xplLibraryVersionsToNodeList(commandInfo->element->doc, params->tagname, xplGetCompiledLibraryVersions(), xplGetRunningLibraryVersions());
		ASSIGN_RESULT(libs, params->repeat, true);
	} else
		ASSIGN_RESULT(_getXplVersion(commandInfo->element->doc, params->part), false, true);
}
