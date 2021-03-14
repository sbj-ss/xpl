#******************************************
#* Polaris project: the pure C XPL engine *
#* (c) НИЛ ИТС, Подковырин, 2006-2020     *
#******************************************

INCLUDE = -I./include -I../libxpl/include -I ../civetweb/include -I/usr/include/libxml2
LIB = -L../libxpl
LIBS = -lxml2 -ldl -lxpl -lonig -lcrypto -ltidy -lidn -lodbc -lpthread -lcurl -lyajl
WARNINGS = -Wall -Wextra -Wmissing-prototypes -Wshadow
FEATURES = -D_XEF_TRANSPORT_CURL -D_XEF_DB_ODBC -D_XEF_HTML_CLEANER_TIDY -D_XEF_CRYPTO_OPENSSL
CFLAGS = -fno-common $(LIB) $(FEATURES) $(WARNINGS) -Werror $(INCLUDE)
CW_CFLAGS = $(WARNINGS) -DWIN32 -I../civetweb/include -DUSE_STACK_SIZE=102400 -DOPENSSL_API_1_1
ifeq ($(DEBUG), 1)
CFLAGS += -g -D_DEBUG -D_FORTIFY_SOURCE=2 -fsanitize=undefined -fno-sanitize-recover -fstack-protector
CW_CFLAGS += -g -DDEBUG
else
CFLAGS += -O2
CW_CFLAGS += -O2 -DNDEBUG
endif

BUILD_DIR = build
CW = ../civetweb/src/civetweb.c
SRC = cw_wrapper.c glue.c main.c
OBJS = $(SRC:%.c=$(BUILD_DIR)/%.o)
OBJS += $(BUILD_DIR)/civetweb.o

CC = gcc

.DEFAULT_GOAL = all

$(BUILD_DIR):
	mkdir $@

$(BUILD_DIR)/civetweb.o: $(CW)
	$(CC) $(CW_CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

xplweb: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	
clean:
	rm -rf $(BUILD_DIR) xplweb || /bin/true
	
all: $(BUILD_DIR) xplweb
