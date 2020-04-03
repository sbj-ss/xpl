#include "commands/Sleep.h"
#include "Messages.h"
#include "Utils.h"
#include <stdio.h>

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
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "both delay and until can't be specified at the same time"), TRUE, TRUE);
		goto done;
	}
	if (!delay_attr && !until_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "neyther delay nor until specified"), TRUE, TRUE);
		goto done;
	}
	if (delay_attr)
	{
		int delay;
		if (sscanf((const char*) delay_attr, "%d", &delay) != 1)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "xpl:sleep: non-integer delay value (%s)", delay_attr), TRUE, TRUE);
			return;
		}
		xprSleep(delay);
	} else if (until_attr) {
		/* ToDo */
	}
	ASSIGN_RESULT(NULL, FALSE, TRUE);
done:
	if (delay_attr) xmlFree(delay_attr);
	if (until_attr) xmlFree(until_attr);
}

xplCommand xplSleepCommand = { xplCmdSleepPrologue, xplCmdSleepEpilogue };
