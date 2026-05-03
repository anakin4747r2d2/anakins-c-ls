.PHONY: all test filter lint install

FILTER ?= .

all:
	nix develop --command make test lint
lint:
	shellcheck --external-sources --shell=bash tests/*_tests.bats

test:
	bats --formatter $(CURDIR)/tests/lsts/lsts-format-pretty tests/*_tests.bats

filter:
	bats --formatter $(CURDIR)/tests/lsts/lsts-format-pretty \
		--filter "$(FILTER)" tests/*_tests.bats

install:
	nix profile install .
