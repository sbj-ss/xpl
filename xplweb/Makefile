#******************************************
#* Polaris project: the pure C XPL engine *
#* (c) НИЛ ИТС, Подковырин, 2006-2020     *
#******************************************

INCLUDE = -I./include -I../libxpl/include -I ../civetweb/include -I/usr/include/libxml2
LIBS = -lxml2 -lxpl -lonig -lcrypto -ltidy -lidn -lodbc -ldl -lpthread -lcivetweb -lcurl -lyajl
WARNINGS = -Wall -Werror
CFLAGS = -L../libxpl -L../civetweb $(WARNINGS)
ifeq ($(DEBUG), 1)
CFLAGS += -g -D_DEBUG -D_FORTIFY_SOURCE=2 -fsanitize=address -fsanitize=undefined -fno-sanitize-recover -fstack-protector
endif

BUILD_DIR = build
SRC = cw.c glue.c main.c
OBJS = $(SRC:%.c=$(BUILD_DIR)/%.o)

.DEFAULT_GOAL = all

$(BUILD_DIR):
	mkdir $@

$(BUILD_DIR)/%.o: %.c
	gcc $(CFLAGS) $(INCLUDE) -c $< -o $@
	
xplweb: $(OBJS)
	gcc $(CFLAGS) -o $@ $^ $(LIBS)
	
clean:
	rm -rf $(BUILD_DIR) xplweb || /bin/true
	
all: $(BUILD_DIR) xplweb
