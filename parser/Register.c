#include <commands/Append.h>
#include <commands/Assert.h>
#include <commands/Attribute.h>
#include <commands/Break.h>
#include <commands/Case.h>
#include <commands/Choose.h>
#include <commands/CleanValue.h>
#include <commands/CommandSupported.h>
#include <commands/Comment.h>
#include <commands/CommentNode.h>
#include <commands/Container.h>
#include <commands/ConvertToDefine.h>
#include <commands/CurrentMacro.h>
#include <commands/DBSession.h>
#include <commands/DebugBreak.h>
#include <commands/DebugPrint.h>
#include <commands/Default.h>
#include <commands/Delete.h>
#include <commands/Digest.h>
#include <commands/Edge.h>
#include <commands/Element.h>
#include <commands/Error.h>
#include <commands/Expand.h>
#include <commands/ExpandAfter.h>
#include <commands/Fatal.h>
#include <commands/FileExists.h>
#include <commands/ForEach.h>
#include <commands/GetAppType.h>
#include <commands/GetAttributes.h>
#include <commands/GetDocumentFilename.h>
#include <commands/GetDocumentRole.h>
#include <commands/GetDocumentSource.h>
#include <commands/GetElapsedTime.h>
#include <commands/GetOption.h>
#include <commands/GetOutputDocument.h>
#include <commands/GetParam.h>
#include <commands/GetProcessingStatus.h>
#include <commands/GetThreadId.h>
#include <commands/GetVersion.h>
#include <commands/If.h>
#include <commands/Include.h>
#include <commands/Inherit.h>
#include <commands/IsDefined.h>
#include <commands/Isolate.h>
#include <commands/ListMacros.h>
#include <commands/LoadModule.h>
#include <commands/ModuleLoaded.h>
#include <commands/Namespace.h>
#include <commands/NoExpand.h>
#include <commands/Otherwise.h>
#include <commands/ParseXml.h>
#include <commands/ProcessingInstruction.h>
#include <commands/RegexMatch.h>
#include <commands/RegexSplit.h>
#include <commands/Rename.h>
#include <commands/ReplaceIfUndefined.h>
#include <commands/Replicate.h>
#include <commands/Return.h>
#include <commands/Save.h>
#include <commands/Serialize.h>
#include <commands/SessionClear.h>
#include <commands/SessionGetId.h>
#include <commands/SessionGetObject.h>
#include <commands/SessionIsPresent.h>
#include <commands/SessionRemoveObject.h>
#include <commands/SessionSetObject.h>
#include <commands/SetLocal.h>
#include <commands/SetOutputDocument.h>
#include <commands/SetParam.h>
#include <commands/SetResponse.h>
#include <commands/Sleep.h>
#include <commands/SQL.h>
#include <commands/StackClear.h>
#include <commands/StackIsEmpty.h>
#include <commands/StackLocalize.h>
#include <commands/StackPop.h>
#include <commands/StackPush.h>
#include <commands/StartTimer.h>
#include <commands/Stringer.h>
#include <commands/SuppressMacros.h>
#include <commands/Switch.h>
#include <commands/Test.h>
#include <commands/Text.h>
#include <commands/UnloadModule.h>
#include <commands/Unstringer.h>
#include <commands/UriEncode.h>
#include <commands/UriEscapeParam.h>
#include <commands/ValueOf.h>
#include <commands/When.h>
#include <commands/With.h>
#include <commands/XJsonSerialize.h>
#include <libxpl/xplcommand.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include "Register.h"
#ifdef _DYNACONF_SUPPORT
#include <commands/AddDB.h>
#include <commands/ChangeDB.h>
#include <commands/GetDB.h>
#include <commands/GetSaMode.h>
#include <commands/Rehash.h>
#include <commands/RemoveDB.h>
#include <commands/Restart.h>
#include <commands/SetOption.h>
#include <commands/SetSaMode.h>
#include <commands/Shutdown.h>
#endif
#ifdef _THREADING_SUPPORT
#include <commands/StartThreadsAndWait.h>
#include <commands/WaitForThreads.h>
#endif
#ifdef _USE_CRASH_COMMAND
#include <commands/Crash.h>
#endif
#ifdef _FILEOP_SUPPORT
#include <commands/FileOp.h>
#endif

typedef struct _xplCommandSignature
{
	xmlChar *name;
	xplCommandPtr command;
	int flags;
} xplCommandSignature, *xplCommandSignaturePtr;

#define XPL_CMDSIG_COMPAT_ONLY 0x01

static xplCommandSignature builtinCommands[] =
{
	{ BAD_CAST "append", &xplAppendCommand, 0 },
	{ BAD_CAST "assert", &xplAssertCommand, 0 },
	{ BAD_CAST "attribute", &xplAttributeCommand, 0 },
	{ BAD_CAST "break", &xplBreakCommand, 0 },
	{ BAD_CAST "case", &xplCaseCommand, 0 },
	{ BAD_CAST "choose", &xplChooseCommand, 0 },
	{ BAD_CAST "clean-value", &xplCleanValueCommand, 0 },
	{ BAD_CAST "command-supported", &xplCommandSupportedCommand, 0 },
	{ BAD_CAST "comment", &xplCommentCommand, 0 },
	{ BAD_CAST "comment-node", &xplCommentNodeCommand, 0 },
	{ BAD_CAST "container", &xplContainerCommand, 0 },
	{ BAD_CAST "convert-to-define", &xplConvertToDefineCommand, 0 },
	{ BAD_CAST "current-macro", &xplCurrentMacroCommand, 0 },
	{ BAD_CAST "dbsession", &xplDBSessionCommand, 0 },
	{ BAD_CAST "debug-break", &xplDebugBreakCommand, 0 },
	{ BAD_CAST "debug-print", &xplDebugPrintCommand, 0 },
	{ BAD_CAST "default", &xplDefaultCommand, 0 },
	{ BAD_CAST "delete", &xplDeleteCommand, 0 },
	{ BAD_CAST "digest", &xplDigestCommand, 0 },
	{ BAD_CAST "edge", &xplEdgeCommand, 0 },
	{ BAD_CAST "element", &xplElementCommand, 0 },
	{ BAD_CAST "error", &xplErrorCommand, 0 },
	{ BAD_CAST "expand", &xplExpandCommand, 0 },
	{ BAD_CAST "expand-after", &xplExpandAfterCommand, 0 },
	{ BAD_CAST "fatal", &xplFatalCommand, 0 },
	{ BAD_CAST "file-exists", &xplFileExistsCommand, 0 },
	{ BAD_CAST "for-each", &xplForEachCommand, 0 },
	{ BAD_CAST "get-app-type", &xplGetAppTypeCommand, 0 },
	{ BAD_CAST "get-attributes", &xplGetAttributesCommand, 0 },
	{ BAD_CAST "get-document-filename", &xplGetDocumentFilenameCommand, 0 },
	{ BAD_CAST "get-document-role", &xplGetDocumentRoleCommand, 0 },
	{ BAD_CAST "get-document-source", &xplGetDocumentSourceCommand, 0 },
	{ BAD_CAST "get-elapsed-time", &xplGetElapsedTimeCommand, 0 },
	{ BAD_CAST "get-option", &xplGetOptionCommand, 0 },
	{ BAD_CAST "get-output-document", &xplGetOutputDocumentCommand, 0 },
	{ BAD_CAST "get-param", &xplGetParamCommand, 0 },
	{ BAD_CAST "get-processing-status", &xplGetProcessingStatusCommand, 0 },
	{ BAD_CAST "get-thread-id", &xplGetThreadIdCommand, 0 },
	{ BAD_CAST "get-version", &xplGetVersionCommand, 0 },
	{ BAD_CAST "if", &xplIfCommand, 0 },
	{ BAD_CAST "include", &xplIncludeCommand, 0 },
	{ BAD_CAST "inherit", &xplInheritCommand, 0 },
	{ BAD_CAST "is-defined", &xplIsDefinedCommand, 0 },
	{ BAD_CAST "is-file-exist", &xplFileExistsCommand, XPL_CMDSIG_COMPAT_ONLY },
	{ BAD_CAST "isolate", &xplIsolateCommand, 0 },
	{ BAD_CAST "list-macros", &xplListMacrosCommand, 0 },
	{ BAD_CAST "load-module", &xplLoadModuleCommand, 0 },
	{ BAD_CAST "module-loaded", &xplModuleLoadedCommand, 0 },
	{ BAD_CAST "namespace", &xplNamespaceCommand, 0 },
	{ BAD_CAST "no-expand", &xplNoExpandCommand, 0 },
	{ BAD_CAST "otherwise", &xplOtherwiseCommand, 0 },
	{ BAD_CAST "parse-xml", &xplParseXmlCommand, 0 },
	{ BAD_CAST "processing-instruction", &xplProcessingInstructionCommand, 0 },
	{ BAD_CAST "regex-match", &xplRegexMatchCommand, 0 },
	{ BAD_CAST "regex-split", &xplRegexSplitCommand, 0 },
	{ BAD_CAST "rename", &xplRenameCommand, 0 },
	{ BAD_CAST "replace-if-undefined", &xplReplaceIfUndefinedCommand, 0 },
	{ BAD_CAST "replicate", &xplReplicateCommand, 0 },
	{ BAD_CAST "return", &xplReturnCommand, 0 },
	{ BAD_CAST "save", &xplSaveCommand, 0 },
	{ BAD_CAST "serialize", &xplSerializeCommand, 0 },
	{ BAD_CAST "session-clear", &xplSessionClearCommand, 0 },
	{ BAD_CAST "session-get-id", &xplSessionGetIdCommand, 0 },
	{ BAD_CAST "session-get-object", &xplSessionGetObjectCommand, 0 },
	{ BAD_CAST "session-is-present", &xplSessionIsPresentCommand, 0 },
	{ BAD_CAST "session-remove-object", &xplSessionRemoveObjectCommand, 0 },
	{ BAD_CAST "session-set-object", &xplSessionSetObjectCommand, 0 },
	{ BAD_CAST "set-local", &xplSetLocalCommand, 0 },
	{ BAD_CAST "set-output-document", &xplSetOutputDocumentCommand, 0 },
	{ BAD_CAST "set-param", &xplSetParamCommand, 0 },
	{ BAD_CAST "set-response", &xplSetResponseCommand, 0 },
	{ BAD_CAST "sleep", &xplSleepCommand, 0 },
	{ BAD_CAST "sql", &xplSqlCommand, 0 },
	{ BAD_CAST "stack-is-empty", &xplStackIsEmptyCommand, 0 },
	{ BAD_CAST "stack-clear", &xplStackClearCommand, 0 },
	{ BAD_CAST "stack-localize", &xplStackLocalizeCommand, 0 },
	{ BAD_CAST "stack-pop", &xplStackPopCommand, 0 },
	{ BAD_CAST "stack-push", &xplStackPushCommand, 0 },
	{ BAD_CAST "start-timer", &xplStartTimerCommand, 0 },
	{ BAD_CAST "stringer", &xplStringerCommand, 0 },
	{ BAD_CAST "suppress-macros", &xplSuppressMacrosCommand, 0 },
	{ BAD_CAST "switch", &xplSwitchCommand, 0 },
	{ BAD_CAST "test", &xplTestCommand, 0 },
	{ BAD_CAST "text", &xplTextCommand, 0 },
	{ BAD_CAST "unload-module", &xplUnloadModuleCommand, 0 },
	{ BAD_CAST "unstringer", &xplUnstringerCommand, 0 },
	{ BAD_CAST "uri-encode", &xplUriEncodeCommand, 0 },
	{ BAD_CAST "uri-escape-param", &xplUriEscapeParamCommand, 0 },
	{ BAD_CAST "value-of", &xplValueOfCommand, 0 },
	{ BAD_CAST "when", &xplWhenCommand, 0 },
	{ BAD_CAST "with", &xplWithCommand, 0 },
	{ BAD_CAST "xjson-serialize", &xplXJsonSerializeCommand, 0 },
#ifdef _USE_CRASH_COMMAND
	{ BAD_CAST "crash", &xplCrashCommand, 0 },
#endif
#ifdef _DYNACONF_SUPPORT
	{ BAD_CAST "add-db", &xplAddDBCommand, 0 },
	{ BAD_CAST "change-db", &xplChangeDBCommand, 0 },
	{ BAD_CAST "get-db", &xplGetDBCommand, 0 },
	{ BAD_CAST "get-sa-mode", &xplGetSaModeCommand, 0 },
	{ BAD_CAST "rehash", &xplRehashCommand, 0 },
	{ BAD_CAST "remove-db", &xplRemoveDBCommand, 0 },
	{ BAD_CAST "restart", &xplRestartCommand, 0 },
	{ BAD_CAST "set-option", &xplSetOptionCommand, 0 },
	{ BAD_CAST "set-sa-mode", &xplSetSaModeCommand, 0 },
	{ BAD_CAST "shutdown", &xplShutdownCommand, 0 },
#endif
#ifdef _THREADING_SUPPORT
	{ BAD_CAST "start-threads-and-wait", &xplStartThreadsAndWaitCommand, 0 },
	{ BAD_CAST "wait-for-threads", &xplWaitForThreadsCommand, 0 },
#endif
#ifdef _FILEOP_SUPPORT
	{ BAD_CAST "file-op", &xplFileOpCommand, 0 },
#endif
};

#define BUILTIN_COMMAND_COUNT (sizeof(builtinCommands) / sizeof(builtinCommands[0]))

bool xplRegisterBuiltinCommands()
{
	unsigned int i;
	xmlChar *cmd_error;

	for (i = 0; i < BUILTIN_COMMAND_COUNT; i++)
	{
		if ((builtinCommands[i].flags & XPL_CMDSIG_COMPAT_ONLY) && !cfgLuciferCompat)
			continue;
		if (xplRegisterCommand(builtinCommands[i].name, builtinCommands[i].command, &cmd_error) != XPL_MODULE_CMD_OK)
		{
			xplDisplayMessage(xplMsgError, BAD_CAST "command \"%s\" failed to initialize, error \"%s\"",
				builtinCommands[i].name, cmd_error);
			if (cmd_error)
				XPL_FREE(cmd_error);
			return false;
		}
	}

	return true;
}

void xplUnregisterBuiltinCommands()
{
	unsigned int i;

	for (i = 0; i < BUILTIN_COMMAND_COUNT; i++)
	{
		if ((builtinCommands[i].flags & XPL_CMDSIG_COMPAT_ONLY) && !cfgLuciferCompat)
			continue;
		xplUnregisterCommand(builtinCommands[i].name);
	}
}
