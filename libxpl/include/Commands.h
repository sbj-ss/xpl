/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __commands_H
#define __commands_H

#include <libxpl/xplcommand.h>

#ifdef __cplusplus
extern "C" {
#endif

extern xplCommand xplAppendCommand;
extern xplCommand xplAssertCommand;
extern xplCommand xplAttributeCommand;
extern xplCommand xplBreakCommand;
extern xplCommand xplCaseCommand;
extern xplCommand xplChooseCommand;
extern xplCommand xplCleanValueCommand;
extern xplCommand xplCommandSupportedCommand;
extern xplCommand xplCommentCommand;
extern xplCommand xplCommentNodeCommand;
extern xplCommand xplContainerCommand;
extern xplCommand xplConvertToDefineCommand;
extern xplCommand xplCurrentMacroCommand;
extern xplCommand xplDBSessionCommand;
extern xplCommand xplDebugBreakCommand;
extern xplCommand xplDebugPrintCommand;
extern xplCommand xplDefaultCommand;
extern xplCommand xplDefineCommand;
extern xplCommand xplDeleteCommand;
extern xplCommand xplDigestCommand;
extern xplCommand xplEdgeCommand;
extern xplCommand xplElementCommand;
extern xplCommand xplErrorCommand;
extern xplCommand xplExpandCommand;
extern xplCommand xplExpandAfterCommand;
extern xplCommand xplFatalCommand;
extern xplCommand xplFileExistsCommand;
extern xplCommand xplForEachCommand;
extern xplCommand xplGetAppTypeCommand;
extern xplCommand xplGetAttributesCommand;
extern xplCommand xplGetDocumentFilenameCommand;
extern xplCommand xplGetElapsedTimeCommand;
extern xplCommand xplGetOptionCommand;
extern xplCommand xplGetParamCommand;
extern xplCommand xplGetThreadIdCommand;
extern xplCommand xplGetVersionCommand;
extern xplCommand xplIfCommand;
extern xplCommand xplIncludeCommand;
extern xplCommand xplInheritCommand;
extern xplCommand xplIsDefinedCommand;
extern xplCommand xplIsolateCommand;
extern xplCommand xplListMacrosCommand;
extern xplCommand xplLoadModuleCommand;
extern xplCommand xplModuleLoadedCommand;
extern xplCommand xplNamespaceCommand;
extern xplCommand xplNoExpandCommand;
extern xplCommand xplOtherwiseCommand;
extern xplCommand xplParseXmlCommand;
extern xplCommand xplProcessingInstructionCommand;
extern xplCommand xplRegexMatchCommand;
extern xplCommand xplRegexSplitCommand;
extern xplCommand xplRenameCommand;
extern xplCommand xplReplaceIfUndefinedCommand;
extern xplCommand xplReplicateCommand;
extern xplCommand xplReturnCommand;
extern xplCommand xplSaveCommand;
extern xplCommand xplSerializeCommand;
extern xplCommand xplSessionClearCommand;
extern xplCommand xplSessionContainsObjectCommand;
extern xplCommand xplSessionGetIdCommand;
extern xplCommand xplSessionGetObjectCommand;
extern xplCommand xplSessionRemoveObjectCommand;
extern xplCommand xplSessionSetObjectCommand;
extern xplCommand xplSetLocalCommand;
extern xplCommand xplSetParamCommand;
extern xplCommand xplSetResponseCommand;
extern xplCommand xplSleepCommand;
extern xplCommand xplSqlCommand;
extern xplCommand xplStackClearCommand;
extern xplCommand xplStackIsEmptyCommand;
extern xplCommand xplStackLocalizeCommand;
extern xplCommand xplStackPopCommand;
extern xplCommand xplStackPushCommand;
extern xplCommand xplStartTimerCommand;
extern xplCommand xplStringerCommand;
extern xplCommand xplSuppressMacrosCommand;
extern xplCommand xplSwitchCommand;
extern xplCommand xplTestCommand;
extern xplCommand xplTextCommand;
extern xplCommand xplUnloadModuleCommand;
extern xplCommand xplUnstringerCommand;
extern xplCommand xplUriEncodeCommand;
extern xplCommand xplUriEscapeParamCommand;
extern xplCommand xplValueOfCommand;
extern xplCommand xplWhenCommand;
extern xplCommand xplWithCommand;
extern xplCommand xplXJsonSerializeCommand;

#ifdef _USE_CRASH_COMMAND
extern xplCommand xplCrashCommand;
#endif

#ifdef _THREADING_SUPPORT
extern xplCommand xplStartThreadsAndWaitCommand;
extern xplCommand xplWaitForThreadsCommand;
#endif

#ifdef _DYNACONF_SUPPORT
extern xplCommand xplAddDBCommand;
extern xplCommand xplChangeDBCommand;
extern xplCommand xplGetDBCommand;
extern xplCommand xplGetSaModeCommand;
extern xplCommand xplRehashCommand;
extern xplCommand xplRemoveDBCommand;
extern xplCommand xplRestartCommand;
extern xplCommand xplSetOptionCommand;
extern xplCommand xplSetSaModeCommand;
extern xplCommand xplShutdownCommand;
#endif

#ifdef _FILEOP_SUPPORT
extern xplCommand xplFileOpCommand;
#endif

#ifdef _DEBUG
extern xplCommand xplDbGarbageCollectCommand;
#endif

#ifdef __cplusplus
}
#endif
#endif
