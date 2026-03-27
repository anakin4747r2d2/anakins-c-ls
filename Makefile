.PHONY: all dev test lint install build

PREFIX ?= /usr/local

TERM_CMD = gnome-terminal --full-screen --

all:
	nix develop --command make test lint

build:
	mkdir -p out
	cc -Wall -Wextra -o out/anakins-c-ls src/main.c

dev:
	$(TERM_CMD) bash -c 'exec nix develop --command \
		nvim \
			-u $(CURDIR)/init.lua \
			-V5$(CURDIR)/nvim.log \
			$(CURDIR)/tests/linux/drivers/gpio/gpio-amd8111.c'

lint:
	shellcheck --external-sources --shell=bash tests/*_tests.bats

test:
	bats --formatter $(CURDIR)/bats-format-pretty tests/*_tests.bats

install:
	nix profile install .
