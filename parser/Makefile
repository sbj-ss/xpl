#******************************************
#* Polaris project: the pure C XPL engine *
#* (c) НИЛ ИТС, Подковырин, 2006-2020     *
#******************************************

INCLUDE = -I./include -I./tests/include -I../xts/include -I/usr/include/libxml2
LIBS = -lxml2 -lxpl -lonig -ltidy -lcrypto -lidn -lodbc
CFLAGS = -fpic -D_IN_DLL -g

SRC = \
abstraction/xef.c\
abstraction/xpr.c\
abstraction/xpr_lin.c\
abstraction/xefhtmlcleanertidy.c\
abstraction/xefdbodbc.c\
xplcommand.c\
xplcore.c\
xpldb.c\
xplmacro.c\
xplmessages.c\
xploptions.c\
xplparams.c\
xplbuffer.c\
xplsave.c\
xplsession.c\
xplstring.c\
xpltree.c\
xplwrappers.c\
Register.c

COMMANDS = \
commands/Append.c\
commands/Assert.c\
commands/Attribute.c\
commands/Break.c\
commands/Case.c\
commands/Choose.c\
commands/CleanValue.c\
commands/CommandSupported.c\
commands/Comment.c\
commands/CommentNode.c\
commands/Container.c\
commands/ConvertToDefine.c\
commands/CurrentMacro.c\
commands/DBSession.c\
commands/DebugBreak.c\
commands/DebugPrint.c\
commands/Default.c\
commands/Delete.c\
commands/Digest.c\
commands/Edge.c\
commands/Element.c\
commands/Error.c\
commands/Fatal.c\
commands/FileExists.c\
commands/ForEach.c\
commands/GetAppType.c\
commands/GetAttributes.c\
commands/GetDocumentFilename.c\
commands/GetDocumentRole.c\
commands/GetDocumentSource.c\
commands/GetElapsedTime.c\
commands/GetOption.c\
commands/GetOutputDocument.c\
commands/GetParam.c\
commands/GetProcessingStatus.c\
commands/GetThreadId.c\
commands/GetVersion.c\
commands/If.c\
commands/Include.c\
commands/Inherit.c\
commands/IsDefined.c\
commands/Isolate.c\
commands/ListMacros.c\
commands/LoadModule.c\
commands/ModuleLoaded.c\
commands/Namespace.c\
commands/Otherwise.c\
commands/ParseXml.c\
commands/ProcessingInstruction.c\
commands/RegexMatch.c\
commands/RegexSplit.c\
commands/Rename.c\
commands/ReplaceIfUndefined.c\
commands/Replicate.c\
commands/Return.c\
commands/Save.c\
commands/Serialize.c\
commands/SessionClear.c\
commands/SessionGetId.c\
commands/SessionGetObject.c\
commands/SessionIsPresent.c\
commands/SessionRemoveObject.c\
commands/SessionSetObject.c\
commands/SetLocal.c\
commands/SetOutputDocument.c\
commands/SetParam.c\
commands/SetResponse.c\
commands/Sleep.c\
commands/SQL.c\
commands/StackClear.c\
commands/StackIsEmpty.c\
commands/StackLocalize.c\
commands/StackPop.c\
commands/StackPush.c\
commands/StartTimer.c\
commands/Stringer.c\
commands/SuppressMacros.c\
commands/Switch.c\
commands/Test.c\
commands/Text.c\
commands/UnloadModule.c\
commands/Unstringer.c\
commands/UriEncode.c\
commands/UriEscapeParam.c\
commands/ValueOf.c\
commands/When.c\
commands/With.c\
commands/XJsonSerialize.c

ifdef $(DYNACONF_SUPPORT)
CFLAGS += -D_DYNACONF_SUPPORT
COMMANDS += \
commands/AddDB.c\
commands/ChangeDB.c\
commands/GetDB.c\
commands/GetSaMode.c\
commands/Rehash.c\
commands/RemoveDB.c\
commands/Restart.c\
commands/SetOption.c\
commands/SetSaMode.c\
commands/Shutdown.c
endif

ifdef $(THREADING_SUPPORT)
CFLAGS += -D_THREADING_SUPPORT
COMMANDS +=\
commands/StartThreadAndWait.c\
commands/WaitForThreads.c
endif

ifdef $(USE_CRASH_COMMAND)
CFLAGS += -D_USE_CRASH_COMMAND
COMMANDS += commands/Crash.c
endif

ifdef $(FILEOP_SUPPORT)
CFLAGS += -D_FILEOP_SUPPORT
COMMANDS += commands/FileOp.c
endif

SRC += $(COMMANDS)

TEST_SRC=tests/Runner.c\
tests/test_xplbuffer.c\
tests/test_xpldb.c

BUILD_DIR = build
OBJS = $(SRC:%.c=$(BUILD_DIR)/%.o)
TEST_OBJS = $(TEST_SRC:%.c=$(BUILD_DIR)/%.o)
TEST_BIN = tests/Runner

.DEFAULT_GOAL = all

build_dirs: $(BUILD_DIR) $(BUILD_DIR)/abstraction $(BUILD_DIR)/commands
$(BUILD_DIR):
	mkdir $@
$(BUILD_DIR)/abstraction:
	mkdir -p $@
$(BUILD_DIR)/commands:
	mkdir -p $@

$(BUILD_DIR)/%.o: %.c
	gcc $(CFLAGS) $(INCLUDE) -c $< -o $@
	
$(TEST_OBJS): $(TEST_SRC)

libxpl.so: $(OBJS)
	gcc -shared -o $@ $(OBJS)

$(TEST_BIN): $(TEST_OBJS)
	gcc $(TEST_OBJS) -o $@ -L../xts -L. $(LIBS) -lxts

test: $(TEST_BIN)
	$(TEST_BIN) $$(if which tput > /dev/null 2>&1 && test $$(tput -T$$TERM colors) -ge 8; then echo -n '-c'; fi)
	
clean:
	rm -rf $(BUILD_DIR) *.so
	
all: build_dirs libxpl.so