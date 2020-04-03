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
#include "Register.h"
#include "Command.h"
#include "Messages.h"
#include "Options.h"
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

#include "Messages.h"

typedef struct _xplCommandSignature
{
	xmlChar *name;
	xplCommandPtr command;
	int flags;
} xplCommandSignature, *xplCommandSignaturePtr;

#define XPL_CMDSIG_COMPAT_ONLY 0x01

static xplCommandSignature builtinCommands[] =
{
	{ BAD_CAST "append", &xplAppendCommand },
	{ BAD_CAST "assert", &xplAssertCommand },
	{ BAD_CAST "attribute", &xplAttributeCommand },
	{ BAD_CAST "break", &xplBreakCommand },
	{ BAD_CAST "case", &xplCaseCommand },
	{ BAD_CAST "choose", &xplChooseCommand },
	{ BAD_CAST "clean-value", &xplCleanValueCommand },
	{ BAD_CAST "command-supported", &xplCommandSupportedCommand },
	{ BAD_CAST "comment", &xplCommentCommand },
	{ BAD_CAST "comment-node", &xplCommentNodeCommand },
	{ BAD_CAST "container", &xplContainerCommand },
	{ BAD_CAST "convert-to-define", &xplConvertToDefineCommand },
	{ BAD_CAST "current-macro", &xplCurrentMacroCommand },
	{ BAD_CAST "dbsession", &xplDBSessionCommand },
	{ BAD_CAST "debug-break", &xplDebugBreakCommand },
	{ BAD_CAST "debug-print", &xplDebugPrintCommand },
	{ BAD_CAST "default", &xplDefaultCommand },
	{ BAD_CAST "delete", &xplDeleteCommand },
	{ BAD_CAST "digest", &xplDigestCommand },
	{ BAD_CAST "edge", &xplEdgeCommand },
	{ BAD_CAST "element", &xplElementCommand },
	{ BAD_CAST "error", &xplErrorCommand },
	{ BAD_CAST "fatal", &xplFatalCommand },
	{ BAD_CAST "file-exists", &xplFileExistsCommand },
	{ BAD_CAST "for-each", &xplForEachCommand },
	{ BAD_CAST "get-app-type", &xplGetAppTypeCommand },
	{ BAD_CAST "get-attributes", &xplGetAttributesCommand },
	{ BAD_CAST "get-document-filename", &xplGetDocumentFilenameCommand },
	{ BAD_CAST "get-document-role", &xplGetDocumentRoleCommand },
	{ BAD_CAST "get-document-source", &xplGetDocumentSourceCommand },
	{ BAD_CAST "get-elapsed-time", &xplGetElapsedTimeCommand },
	{ BAD_CAST "get-option", &xplGetOptionCommand },
	{ BAD_CAST "get-output-document", &xplGetOutputDocumentCommand },
	{ BAD_CAST "get-param", &xplGetParamCommand },
	{ BAD_CAST "get-processing-status", &xplGetProcessingStatusCommand },
	{ BAD_CAST "get-thread-id", &xplGetThreadIdCommand },
	{ BAD_CAST "get-version", &xplGetVersionCommand },
	{ BAD_CAST "if", &xplIfCommand },
	{ BAD_CAST "include", &xplIncludeCommand },
	{ BAD_CAST "inherit", &xplInheritCommand },
	{ BAD_CAST "is-defined", &xplIsDefinedCommand },
	{ BAD_CAST "is-file-exist", &xplFileExistsCommand, XPL_CMDSIG_COMPAT_ONLY },
	{ BAD_CAST "isolate", &xplIsolateCommand },
	{ BAD_CAST "list-macros", &xplListMacrosCommand },
	{ BAD_CAST "load-module", &xplLoadModuleCommand },
	{ BAD_CAST "module-loaded", &xplModuleLoadedCommand },
	{ BAD_CAST "namespace", &xplNamespaceCommand },
	{ BAD_CAST "otherwise", &xplOtherwiseCommand },
	{ BAD_CAST "parse-xml", &xplParseXmlCommand },
	{ BAD_CAST "processing-instruction", &xplProcessingInstructionCommand },
	{ BAD_CAST "regex-match", &xplRegexMatchCommand },
	{ BAD_CAST "regex-split", &xplRegexSplitCommand },
	{ BAD_CAST "rename", &xplRenameCommand },
	{ BAD_CAST "replace-if-undefined", &xplReplaceIfUndefinedCommand },
	{ BAD_CAST "replicate", &xplReplicateCommand },
	{ BAD_CAST "return", &xplReturnCommand },
	{ BAD_CAST "save", &xplSaveCommand },
	{ BAD_CAST "serialize", &xplSerializeCommand },
	{ BAD_CAST "session-clear", &xplSessionClearCommand },
	{ BAD_CAST "session-get-id", &xplSessionGetIdCommand },
	{ BAD_CAST "session-get-object", &xplSessionGetObjectCommand },
	{ BAD_CAST "session-is-present", &xplSessionIsPresentCommand },
	{ BAD_CAST "session-remove-object", &xplSessionRemoveObjectCommand },
	{ BAD_CAST "session-set-object", &xplSessionSetObjectCommand },
	{ BAD_CAST "set-local", &xplSetLocalCommand },
	{ BAD_CAST "set-output-document", &xplSetOutputDocumentCommand },
	{ BAD_CAST "set-param", &xplSetParamCommand },
	{ BAD_CAST "set-response", &xplSetResponseCommand },
	{ BAD_CAST "sleep", &xplSleepCommand },
	{ BAD_CAST "sql", &xplSqlCommand },
	{ BAD_CAST "stack-is-empty", &xplStackIsEmptyCommand },
	{ BAD_CAST "stack-clear", &xplStackClearCommand },
	{ BAD_CAST "stack-localize", &xplStackLocalizeCommand },
	{ BAD_CAST "stack-pop", &xplStackPopCommand },
	{ BAD_CAST "stack-push", &xplStackPushCommand },
	{ BAD_CAST "start-timer", &xplStartTimerCommand },
	{ BAD_CAST "stringer", &xplStringerCommand },
	{ BAD_CAST "suppress-macros", &xplSuppressMacrosCommand },
	{ BAD_CAST "switch", &xplSwitchCommand },
	{ BAD_CAST "test", &xplTestCommand },
	{ BAD_CAST "text", &xplTextCommand },
	{ BAD_CAST "unload-module", &xplUnloadModuleCommand },
	{ BAD_CAST "unstringer", &xplUnstringerCommand },
	{ BAD_CAST "uri-encode", &xplUriEncodeCommand },
	{ BAD_CAST "uri-escape-param", &xplUriEscapeParamCommand },
	{ BAD_CAST "value-of", &xplValueOfCommand },
	{ BAD_CAST "when", &xplWhenCommand },
	{ BAD_CAST "with", &xplWithCommand },
	{ BAD_CAST "xjson-serialize", &xplXJsonSerializeCommand },
#ifdef _USE_CRASH_COMMAND
	{ BAD_CAST "crash", &xplCrashCommand },
#endif
#ifdef _DYNACONF_SUPPORT
	{ BAD_CAST "add-db", &xplAddDBCommand },
	{ BAD_CAST "change-db", &xplChangeDBCommand },
	{ BAD_CAST "get-db", &xplGetDBCommand },
	{ BAD_CAST "get-sa-mode", &xplGetSaModeCommand },
	{ BAD_CAST "rehash", &xplRehashCommand },
	{ BAD_CAST "remove-db", &xplRemoveDBCommand },
	{ BAD_CAST "restart", &xplRestartCommand },
	{ BAD_CAST "set-option", &xplSetOptionCommand },
	{ BAD_CAST "set-sa-mode", &xplSetSaModeCommand },
	{ BAD_CAST "shutdown", &xplShutdownCommand },
#endif
#ifdef _THREADING_SUPPORT
	{ BAD_CAST "start-threads-and-wait", &xplStartThreadsAndWaitCommand },
	{ BAD_CAST "wait-for-threads", &xplWaitForThreadsCommand },
#endif
#ifdef _FILEOP_SUPPORT
	{ BAD_CAST "file-op", &xplFileOpCommand },
#endif
};

#define BUILTIN_COMMAND_COUNT (sizeof(builtinCommands) / sizeof(builtinCommands[0]))

BOOL xplRegisterBuiltinCommands()
{
	int i;
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
				xmlFree(cmd_error);
			return FALSE;
		}
	}

	return TRUE;
}
