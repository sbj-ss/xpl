#******************************************
#* Polaris project: the pure C XPL engine *
#* (c) НИЛ ИТС, Подковырин, 2006-2020     *
#******************************************

INCLUDE = -I./include -I./tests/include -I../xts/include -I/usr/include/libxml2
LIBS = -lxml2 -lxpl -lonig -ltidy -lcrypto -lidn -lodbc -lcurl -lyajl
WARNINGS = -Wall -Wmissing-prototypes -Wshadow -Werror -Wextra 
FEATURES = -D_XEF_TRANSPORT_CURL -D_XEF_DB_ODBC -D_XEF_HTML_CLEANER_TIDY -D_XEF_CRYPTO_OPENSSL
CFLAGS = -fpic -fno-common -D_IN_DLL $(FEATURES) $(WARNINGS)
ifeq ($(DEBUG), 1)
CFLAGS += -g -D_DEBUG -D_FORTIFY_SOURCE=2 -fsanitize=undefined -fno-sanitize-recover -fstack-protector
# -fsanitize=address 
else
CFLAGS += -O2
endif

CC = gcc
SO_EXT = .so
BIN_EXT =

SRC = \
abstraction/xefstart.c\
abstraction/xpr.c\
abstraction/xpr_lin.c\
abstraction/xeftransportcurl.c\
abstraction/xefdbodbc.c\
abstraction/xefhtmlcleanertidy.c\
abstraction/xefcryptoopenssl.c\
xplcommand.c\
xplcore.c\
xpldb.c\
xpljsonx.c\
xplmacro.c\
xplmessages.c\
xploptions.c\
xplparams.c\
xplbuffer.c\
xplsave.c\
xplsession.c\
xplstart.c\
xplstring.c\
xpltree.c\
xplversion.c\
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
commands/DebugPrint.c\
commands/Default.c\
commands/Define.c\
commands/Delete.c\
commands/Digest.c\
commands/Edge.c\
commands/Element.c\
commands/Error.c\
commands/Expand.c\
commands/ExpandAfter.c\
commands/Fatal.c\
commands/FileExists.c\
commands/ForEach.c\
commands/GetAppType.c\
commands/GetAttributes.c\
commands/GetDocumentFilename.c\
commands/GetElapsedTime.c\
commands/GetOption.c\
commands/GetParam.c\
commands/GetThreadId.c\
commands/GetVersion.c\
commands/If.c\
commands/Include.c\
commands/Inherit.c\
commands/IsDefined.c\
commands/Isolate.c\
commands/JsonXParse.c\
commands/JsonXSerialize.c\
commands/ListMacros.c\
commands/LoadModule.c\
commands/ModuleLoaded.c\
commands/Namespace.c\
commands/NoExpand.c\
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
commands/SessionContainsObject.c\
commands/SessionGetId.c\
commands/SessionGetObject.c\
commands/SessionRemoveObject.c\
commands/SessionSetObject.c\
commands/SetLocal.c\
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
commands/With.c

ifeq ($(DYNACONF_SUPPORT), 1)
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

ifeq ($(THREADING_SUPPORT), 1)
CFLAGS += -D_THREADING_SUPPORT
COMMANDS +=\
commands/StartThreadsAndWait.c\
commands/WaitForThreads.c
endif

ifeq ($(USE_CRASH_COMMAND), 1)
CFLAGS += -D_USE_CRASH_COMMAND
COMMANDS += commands/Crash.c
endif

ifeq ($(FILEOP_SUPPORT), 1)
CFLAGS += -D_FILEOP_SUPPORT
COMMANDS += commands/FileOp.c
endif

ifeq ($(DEBUG), 1)
COMMANDS +=\
commands/DbGarbageCollect.c\
commands/DebugBreak.c
endif

SRC += $(COMMANDS)

TEST_SRC=tests/Runner.c\
tests/test_xplbuffer.c\
tests/test_xpldb.c\
tests/test_xplsave.c

BUILD_DIR = build
OBJS = $(SRC:%.c=$(BUILD_DIR)/%.o)
TEST_OBJS = $(TEST_SRC:%.c=$(BUILD_DIR)/%.o)
TEST_BIN = tests/Runner$(BIN_EXT)

.DEFAULT_GOAL = all

build_dirs: $(BUILD_DIR) $(BUILD_DIR)/abstraction $(BUILD_DIR)/commands $(BUILD_DIR)/tests
$(BUILD_DIR):
	mkdir $@
$(BUILD_DIR)/abstraction:
	mkdir -p $@
$(BUILD_DIR)/commands:
	mkdir -p $@
$(BUILD_DIR)/tests:
	mkdir -p $@

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@
	
libxpl$(SO_EXT): $(OBJS)
	$(CC) -shared -o $@ $(OBJS)

$(TEST_BIN): libxpl$(SO_EXT) $(TEST_OBJS)
	$(CC) $(CFLAGS) $(TEST_OBJS) -o $@ -L../xts -L. $(LIBS) -lxts

test: $(TEST_BIN)
	LD_LIBRARY_PATH=.:$$LD_LIBRARY_PATH $(TEST_BIN) $$(if which tput > /dev/null 2>&1 && test $$(tput -T$$TERM colors) -ge 8; then echo -n '-c'; fi)
	
clean:
	rm -rf $(BUILD_DIR) *$(SO_EXT)
	
all: build_dirs libxpl$(SO_EXT)
