.PHONY: all dev test filter lint install

FILTER ?= .

all:
	nix develop --command make test lint

dev:
	nix build
	rm -f *.log
	nvim -u $(CURDIR)/init.lua \
		-V5$(CURDIR)/nvim.log \
		$(CURDIR)/tests/linux/drivers/gpio/gpio-amd8111.c

lint:
	shellcheck --external-sources --shell=bash tests/*_tests.bats

test:
	bats --formatter $(CURDIR)/bats-format-pretty tests/*_tests.bats

filter:
	bats --formatter $(CURDIR)/bats-format-pretty \
		--filter "$(FILTER)" tests/*_tests.bats

install:
	nix profile install .
