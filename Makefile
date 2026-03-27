.PHONY: all dev test lint install

PREFIX ?= /usr/local

TERM_CMD = gnome-terminal --full-screen --

all:
	nix develop --command make test lint

dev:
	$(TERM_CMD) bash -c 'exec nix develop --command \
		nvim --clean \
			-u $(CURDIR)/init.lua \
			$(CURDIR)/tests/linux/drivers/gpio/gpio-amd8111.c'

lint:
	shellcheck --external-sources --shell=bash tests/*_tests.bats

test:
	bats --formatter $(CURDIR)/bats-format-pretty tests/*_tests.bats

install:
	nix profile install .
