#include <stdio.h>
#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplutils.h>
#include "commands/Sleep.h"

void xplCmdSleepPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdSleepEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define DELAY_ATTR (BAD_CAST "delay")
#define UNTIL_ATTR (BAD_CAST "until")
	xmlChar* delay_attr = NULL;
	xmlChar* until_attr = NULL;

	delay_attr = xmlGetNoNsProp(commandInfo->element, DELAY_ATTR);
	until_attr = xmlGetNoNsProp(commandInfo->element, UNTIL_ATTR);
	if (delay_attr && until_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "both delay and until can't be specified at the same time"), true, true);
		goto done;
	}
	if (!delay_attr && !until_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "neyther delay nor until specified"), true, true);
		goto done;
	}
	if (delay_attr)
	{
		int delay;
		if (sscanf((const char*) delay_attr, "%d", &delay) != 1)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "xpl:sleep: non-integer delay value (%s)", delay_attr), true, true);
			return;
		}
		xprSleep(delay);
	} else if (until_attr) {
		/* ToDo */
	}
	ASSIGN_RESULT(NULL, false, true);
done:
	if (delay_attr) xmlFree(delay_attr);
	if (until_attr) xmlFree(until_attr);
}

xplCommand xplSleepCommand = { xplCmdSleepPrologue, xplCmdSleepEpilogue };
