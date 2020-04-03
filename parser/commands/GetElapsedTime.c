#include "commands/GetElapsedTime.h"
#include "Core.h"
#include "Utils.h"
#include <time.h>

void xplCmdProfileCheckpointPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdGetElapsedTimeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlChar buf[32];
	struct tm tm;
	time_t current, old;
	xmlNodePtr ret;

	time(&current);
	old = commandInfo->document->main->profile_checkpoint;
	sprintf((char*) buf, "%lu", current - old);
	ret = xmlNewDocText(commandInfo->document->document, buf);
	ASSIGN_RESULT(ret, FALSE, TRUE);
	commandInfo->document->main->profile_checkpoint = current;
}

xplCommand xplGetElapsedTimeCommand = { xplCmdProfileCheckpointPrologue, xplCmdGetElapsedTimeEpilogue };
