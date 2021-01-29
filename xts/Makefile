#******************************************
#* Polaris project: the pure C XPL engine *
#* (c) НИЛ ИТС, Подковырин, 2006-2020     *
#******************************************

INCLUDE = -I./include -I/usr/include/libxml2 -I ../libxpl/include
LIB = -L.
LIBS = -lxml2 -lxts
WARNINGS = -Wall -Werror
CFLAGS = $(WARNINGS)
ifeq ($(DEBUG), 1)
CFLAGS += -D_DEBUG -D_FORTIFY_SOURCE=2 -fsanitize=undefined -fno-sanitize-recover -fstack-protector
endif

SRC = xts.c
TEST_SRC = $(SRC) Sanity.c Runner.c

CC = gcc
AR = ar

BUILD_DIR = build
OBJS = $(SRC:%.c=$(BUILD_DIR)/%.o)
TEST_OBJS = $(TEST_SRC:%.c=$(BUILD_DIR)/%.o)
TEST_BIN = ./Runner

.DEFAULT_GOAL = all

$(BUILD_DIR):
	echo $(OBJS)
	mkdir $@

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@
	
$(OBJS): $(SRC)

$(TEST_OBJS): $(TEST_SRC)

libxts.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

$(TEST_BIN): $(TEST_OBJS) libxts.a
	$(CC) $(CFLAGS) $(TEST_OBJS) -o $@ $(LIB) $(LIBS)

test: $(TEST_BIN)
	$(TEST_BIN) $$(if which tput > /dev/null 2>&1 && test $$(tput -T$$TERM colors) -ge 8; then echo -n '-c'; fi)
	
clean:
	rm -f $(BUILD_DIR)/*.o *.a $(TEST_BIN)
	
all: $(BUILD_DIR) libxts.a
