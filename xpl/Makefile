#******************************************
#* Polaris project: the pure C XPL engine *
#* (c) НИЛ ИТС, Подковырин, 2006-2020     *
#******************************************

INCLUDE = -I../libxpl/include -I/usr/include/libxml2
LIBS = -lxml2 -lxpl -lonig -lcrypto -ltidy -lidn -lodbc
WARNINGS = -Wall -Werror
CFLAGS = -L../libxpl $(WARNINGS)
ifeq ($(DEBUG), 1)
CFLAGS += -g -D_DEBUG -D_FORTIFY_SOURCE=2 -fsanitize=address -fsanitize=undefined -fno-sanitize-recover -fstack-protector
endif

.DEFAULT_GOAL = all

.c.o:
	gcc $(CFLAGS) $(INCLUDE) -c $< -o $@
	
xpl: xpl.o
	gcc $(CFLAGS) -o $@ $< $(LIBS)
	
clean:
	rm *.o cmdline || /bin/true
	
all: xpl
