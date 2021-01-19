ifeq ($(QUIET), 1)
TAIL = 2> /dev/null
endif

verify-helpers:
	composite/verify-helpers.sh $(TAIL)

bootstrap-tests:
	cd bootstrap && ./run-tests.sh $(TAIL)

composite-tests:
	cd composite && ./run-tests.sh $(TAIL)

test: verify-helpers bootstrap-tests composite-tests
