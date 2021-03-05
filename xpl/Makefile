#******************************************
#* Polaris project: the pure C XPL engine *
#* (c) НИЛ ИТС, Подковырин, 2006-2020     *
#******************************************

INCLUDE = -I../libxpl/include -I/usr/include/libxml2
LIBS = -lxml2 -lxpl -lonig -lcrypto -ltidy -lidn -lodbc -lcurl -lyajl
WARNINGS = -Wall -Werror
CFLAGS = -fno-common -L../libxpl $(WARNINGS)
ifeq ($(DEBUG), 1)
CFLAGS += -g -D_DEBUG -D_FORTIFY_SOURCE=2 -fsanitize=undefined -fno-sanitize-recover -fstack-protector
# -fsanitize=address 
endif

CC = gcc

SRC = xpl.c

BUILD_DIR = build
OBJS = $(SRC:%.c=$(BUILD_DIR)/%.o)

.DEFAULT_GOAL = all

$(BUILD_DIR):
	mkdir $@

$(BUILD_DIR)/%.o: %.c $(BUILD_DIR)
	gcc $(CFLAGS) $(INCLUDE) -c $< -o $@
	
xpl: $(OBJS)
	gcc $(CFLAGS) -o $@ $< $(LIBS)
	
clean:
	rm $(BUILD_DIR)/*.o xpl || /bin/true
	
all: xpl
