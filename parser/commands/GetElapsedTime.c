#include <time.h>
#include <libxpl/xplcore.h>
#include "commands/GetElapsedTime.h"

void xplCmdProfileCheckpointPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdGetElapsedTimeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlChar buf[32];
	time_t current, old;
	xmlNodePtr ret;

	time(&current);
	old = commandInfo->document->main->profile_checkpoint;
	sprintf((char*) buf, "%lu", current - old);
	ret = xmlNewDocText(commandInfo->document->document, buf);
	ASSIGN_RESULT(ret, false, true);
	commandInfo->document->main->profile_checkpoint = current;
}

xplCommand xplGetElapsedTimeCommand = { xplCmdProfileCheckpointPrologue, xplCmdGetElapsedTimeEpilogue };
