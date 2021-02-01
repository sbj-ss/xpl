#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>

void xplCmdAppendEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef enum _AppendPosition
{
	POS_BEFORE,
	POS_AFTER,
	POS_FIRST,
	POS_LAST
} AppendPosition;

typedef struct _xplCmdAppendParams
{
	xmlXPathObjectPtr source;
	xmlXPathObjectPtr destination;
	AppendPosition position;
} xplCmdAppendParams, *xplCmdAppendParamsPtr;

static const xplCmdAppendParams params_stencil =
{
	.destination = NULL,
	.source = NULL,
	.position = POS_LAST
};

static xplCmdParamDictValue position_dict_values[] =
{
	{ BAD_CAST "before", POS_BEFORE },
	{ BAD_CAST "after", POS_AFTER },
	{ BAD_CAST "first", POS_FIRST },
	{ BAD_CAST "last", POS_LAST },
	{ NULL, 0 }
};

static xmlChar* destination_aliases[] = { BAD_CAST "select", NULL };

xplCommand xplAppendCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdAppendEpilogue,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdAppendParams),
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.parameters = {
		{
			.name = BAD_CAST "destination",
			.type = XPL_CMD_PARAM_TYPE_XPATH,
			.extra.xpath_type = XPL_CMD_PARAM_XPATH_TYPE_NODESET,
			.required = true,
			.aliases = destination_aliases,
			.value_stencil = &params_stencil.destination
		}, {
			.name = BAD_CAST "source",
			.type = XPL_CMD_PARAM_TYPE_XPATH,
			.extra.xpath_type = XPL_CMD_PARAM_XPATH_TYPE_NODESET,
			.value_stencil = &params_stencil.source
		}, {
			.name = BAD_CAST "position",
			.type = XPL_CMD_PARAM_TYPE_DICT,
			.extra.dict_values = position_dict_values,
			.value_stencil = &params_stencil.position
		}, {
			.name = NULL
		}
	}
};

static void _doAppend(xmlNodePtr src, xmlNodePtr dst, AppendPosition mode)
{
	switch (mode)
	{
		case POS_BEFORE:
			xplPrependList(dst, src);
			break;
		case POS_AFTER:
			xplAppendList(dst, src);
			break;
		case POS_FIRST:
			xplPrependChildren(dst, src);
			break;
		case POS_LAST:
			xplAppendChildren(dst, src);
			break;
		default:
			DISPLAY_INTERNAL_ERROR_MESSAGE();
	}
}

void xplCmdAppendEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define PARENT(node) (((cmd_params->position == POS_AFTER) || (cmd_params->position == POS_BEFORE))? ((node)->parent): (node))
	xplCmdAppendParamsPtr cmd_params = (xplCmdAppendParamsPtr) commandInfo->params;
	xmlNodePtr parent, clone, cur_dst;
	ssize_t i, j;

	if (
		(cmd_params->source && (!cmd_params->source->nodesetval || !cmd_params->source->nodesetval->nodeNr))
		||
		(!cmd_params->source && !commandInfo->element->children)
	){
		/* no nodes to append */
		ASSIGN_RESULT(NULL, false, true);
		return;
	}

	if (cmd_params->destination && cmd_params->destination->nodesetval)
	{

		for (i = 0; i < (ssize_t) cmd_params->destination->nodesetval->nodeNr; i++)
		{
			cur_dst = cmd_params->destination->nodesetval->nodeTab[i];
			if ((cmd_params->position > POS_AFTER) && (cur_dst->type != XML_ELEMENT_NODE))
				/* don't assign content to non-elements */ // TODO warning
				continue;
			if (cur_dst->type == XML_ATTRIBUTE_NODE)
				/* can't add elements to attributes */ // TODO warning
				continue;
			parent = PARENT(cur_dst);
			if (!cmd_params->source) /* add own content */
			{
				clone = xplCloneNodeList(commandInfo->element->children, parent, commandInfo->element->doc);
				_doAppend(clone, cur_dst, cmd_params->position);
			} else if (cmd_params->source->nodesetval) {
				/* add source selection */
				if (cmd_params->position == POS_FIRST || cmd_params->position == POS_AFTER)
				{
					/* reverse loop to keep node order */
					for (j = (ssize_t) cmd_params->source->nodesetval->nodeNr - 1; j >= 0; j--)
						_doAppend(xplCloneAsNodeChild(cmd_params->source->nodesetval->nodeTab[j], parent), cur_dst, cmd_params->position);
				} else {
					for (j = 0; j < (ssize_t) cmd_params->source->nodesetval->nodeNr; j++)
						_doAppend(xplCloneAsNodeChild(cmd_params->source->nodesetval->nodeTab[j], parent), cur_dst, cmd_params->position);
				}
			}
		}
	}

	ASSIGN_RESULT(NULL, false, true);
}
