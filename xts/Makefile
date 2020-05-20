#******************************************
#* Polaris project: the pure C XPL engine *
#* (c) НИЛ ИТС, Подковырин, 2006-2020     *
#******************************************

INCLUDE = -I./include -I/usr/include/libxml2
LIBS = -lxml2
CFLAGS = -g

SRC = xts.c
TEST_SRC = $(SRC) Sanity.c Runner.c 

OBJS = $(SRC:.c=.o)
TEST_OBJS = $(TEST_SRC:.c=.o)
TEST_BIN = ./Runner

.DEFAULT_GOAL = all

.c.o:
	gcc $(CFLAGS) $(INCLUDE) -c $< -o $@
	
$(OBJS): $(SRC)

$(TEST_OBJS): $(TEST_SRC)

libxts.a: $(OBJS)
	ar rcs $@ $(OBJS)

$(TEST_BIN): $(TEST_OBJS)
	gcc -L. $(TEST_OBJS) -o $@ $(LIBS) -lxts 

test: $(TEST_BIN)
	$(TEST_BIN) $$(if which tput > /dev/null 2>&1 && test $$(tput -T$$TERM colors) -ge 8; then echo -n '-c'; fi)
	
clean:
	rm -f *.o *.a $(TEST_BIN)
	
all: libxts.a