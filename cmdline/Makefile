#******************************************
#* Polaris project: the pure C XPL engine *
#* (c) НИЛ ИТС, Подковырин, 2006-2020     *
#******************************************

INCLUDE = -I../parser/include -I/usr/include/libxml2
LIBS = -lxml2 -lxpl -lonig -lcrypto -ltidy -lidn -lodbc
WARNINGS = -Wall -Werror
CFLAGS = -L../parser -g $(WARNINGS)

.DEFAULT_GOAL = all

.c.o:
	gcc $(CFLAGS) $(INCLUDE) -c $< -o $@
	
cmdline: cmdline.o
	gcc $(CFLAGS) -o $@ $< $(LIBS)
	
clean:
	rm *.o cmdline || /bin/true
	
all: cmdline