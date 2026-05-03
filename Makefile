.PHONY: all dev test filter lint install

FILTER ?= .

all:
	nix develop --command make test lint

dev:
	nix build
	rm -f *.log
	gnome-terminal --full-screen -- \
		env PATH="$(CURDIR)/result/bin:$(PATH)" \
		nvim -u $(CURDIR)/init.lua \
			-V5$(CURDIR)/nvim.log \
			$(CURDIR)/tests/linux/drivers/gpio/gpio-amd8111.c

lint:
	shellcheck --external-sources --shell=bash tests/*_tests.bats

test:
	bats --formatter $(CURDIR)/tests/lsts/lsts-format-pretty tests/*_tests.bats

filter:
	bats --formatter $(CURDIR)/tests/lsts/lsts-format-pretty \
		--filter "$(FILTER)" tests/*_tests.bats

install:
	nix profile install .
