.PHONY: all help clean test clang-format

SHELL=/usr/bin/env bash -eo pipefail
IND=./prophex

.SECONDARY:

DEPS= $(wildcard src/*.h) $(wildcard src/*.c) $(wildcard src/bwa/.*.h) $(wildcard src/bwa/*.c)

all: prophex readme

prophex: $(DEPS)
	$(MAKE) -C src

help: ## Print help message
	    @echo "$$(grep -hE '^\S+:.*##' $(MAKEFILE_LIST) | sed -e 's/:.*##\s*/:/' -e 's/^\(.\+\):\(.*\)/\\x1b[36m\1\\x1b[m:\2/' | column -c2 -t -s : | sort)"

test:
	$(MAKE) -C tests

readme:
	f=$$(mktemp);\
		sed '/USAGE-BEGIN/q' README.md >> $$f; \
		printf -- '-->\n' >> $$f; \
		printf '```' >> $$f; \
		$(IND) 2>&1 | perl -pe 's/^(.*)$$/\1/g' >> $$f; \
		printf '```\n\n' >> $$f; \
		\
		printf '```' >> $$f; \
		$(IND) index -h 2>&1 | perl -pe 's/^(.*)$$/\1/g' >> $$f; \
		printf '```\n\n' >> $$f; \
		\
		printf '```' >> $$f; \
		$(IND) query -h 2>&1 | perl -pe 's/^(.*)$$/\1/g' >> $$f; \
		printf '```\n\n' >> $$f; \
		\
		printf '```' >> $$f; \
		$(IND) klcp -h 2>&1 | perl -pe 's/^(.*)$$/\1/g' >> $$f; \
		printf '```\n\n' >> $$f; \
		\
		printf '```' >> $$f; \
		$(IND) bwtdowngrade -h 2>&1 | perl -pe 's/^(.*)$$/\1/g' >> $$f; \
		printf '```\n\n' >> $$f; \
		\
		printf '```' >> $$f; \
		$(IND) bwt2fa -h 2>&1 | perl -pe 's/^(.*)$$/\1/g' >> $$f; \
		printf '```\n\n' >> $$f; \
		sed -n '/USAGE-END/,$$ p' README.md >> $$f;\
		cat $$f \
		| perl -pe 's/^[\s]+$$/\n/g' \
		| perl -pe 's/[\s]+$$/\n/g' \
		> README.md;

format:
	clang-format -verbose -i src/*.h src/*.c

clean: ## Clean
	$(MAKE) -C src clean
	$(MAKE) -C tests clean
	rm -f prophex
