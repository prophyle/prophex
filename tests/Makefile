TESTS = $(wildcard */.)

.PHONY: all help clean $(TESTS) parallel

SHELL=/usr/bin/env bash -eo pipefail

.SECONDARY:

all: $(TESTS)

help: ## Print help message
	@echo "$$(grep -hE '^\S+:.*##' $(MAKEFILE_LIST) | sed -e 's/:.*##\s*/:/' -e 's/^\(.\+\):\(.*\)/\\x1b[36m\1\\x1b[m:\2/' | column -c2 -t -s : | sort)"

clean: ## Clean

.PHONY: all clean big $(ABC) A B C compile parallel

.NOTPARALLEL: B00_compile

V=1

all: A B

parallel:
	$(MAKE) -j

$(TESTS): compile
	@echo
	@echo "========================================="
	@echo "$@"
	@echo "========================================="
	@echo
	$(MAKE) -C $@

compile:
	$(MAKE) -C ../src

clean:
	@for a in $(TESTS); do \
		$(MAKE) -C $$a clean; \
	done;

