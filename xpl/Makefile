#******************************************
#* Polaris project: the pure C XPL engine *
#* (c) НИЛ ИТС, Подковырин, 2006-2020     *
#******************************************

INCLUDE = -I../libxpl/include -I/usr/include/libxml2
LIBS = -lxml2 -lxpl -lonig -lcrypto -ltidy -lidn -lodbc -lcurl -lyajl
WARNINGS = \
-Wall\
-Wextra\
-Wmissing-prototypes\
-Wshadow\
-Wformat=2\
-Wformat-overflow=2\
-Wformat-truncation=2\
-Wformat-security\
-Wformat-signedness\
-Wnull-dereference\
-Warray-bounds=2\
-Wimplicit-fallthrough=3\
-Wshift-overflow=2\
-Wvla\
-Walloca\
-Wstringop-overflow=4\
-Wlogical-op\
-Wduplicated-cond\
-Wduplicated-branches\
-Wswitch-default\
-Wundef\
-Wstrict-prototypes\
-Wcast-align\
-Wjump-misses-init\
-Wtrampolines\
-Wstack-protector\
-Werror
CFLAGS = -fno-common $(WARNINGS)
ifeq ($(DEBUG), 1)
CFLAGS += -g -D_DEBUG -D_FORTIFY_SOURCE=2 -fsanitize=undefined -fno-sanitize-recover -fstack-protector
# -fsanitize=address 
endif
LINKER_FLAGS = -L../libxpl -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -Wl,-z,separate-code

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
	gcc $(CFLAGS) $(LINKER_FLAGS) -o $@ $< $(LIBS)
	
clean:
	rm $(BUILD_DIR)/*.o xpl || /bin/true
	
all: xpl
