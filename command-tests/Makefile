ifeq ($(QUIET), 1)
TAIL = 2> /dev/null
endif

verify-helpers:
	composite/verify-helpers.sh $(TAIL)

bootstrap-tests:
	cd bootstrap; ./run-tests.sh $(TAIL); cd ..

composite-tests:
	cd composite; ./run-tests.sh $(TAIL); cd ..

test: verify-helpers bootstrap-tests composite-tests
