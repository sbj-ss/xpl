#include <libxpl/xplcore.h>
#include <time.h>

void xplCmdGetElapsedTimeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

xplCommand xplGetElapsedTimeCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdGetElapsedTimeEpilogue,
	.flags = 0,
	.params_stencil = NULL
};

void xplCmdGetElapsedTimeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlChar buf[32];
	time_t current, old;
	xmlNodePtr ret;

	time(&current);
	old = commandInfo->document->main->profile_checkpoint;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
	if (sizeof(current) == 4)
		sprintf((char*) buf, "%lu", current - old);
	else
		sprintf((char*) buf, "%I64u", current - old);
#pragma GCC diagnostic pop
	ret = xmlNewDocText(commandInfo->document->document, buf);
	ASSIGN_RESULT(ret, false, true);
	commandInfo->document->main->profile_checkpoint = current;
}
