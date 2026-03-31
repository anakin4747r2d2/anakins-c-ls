.PHONY: all dev test filter lint install build

PREFIX ?= /usr/local

INSTALL_DIR = $(PREFIX)/bin

FILTER ?= .

TERM_CMD = gnome-terminal --full-screen --

all:
	nix develop --command make build test lint

build:
	mkdir -p out
	cc -Wall -Wextra -Isrc -o out/anakins-c-ls src/main.c -ltree-sitter -l:c.so

dev: build
	rm -f *.log
	cd tests/linux && \
	$(TERM_CMD) bash -c 'exec nix develop $(CURDIR) --command \
		nvim \
			-u $(CURDIR)/init.lua \
			-V5$(CURDIR)/nvim.log \
			$(CURDIR)/tests/linux/drivers/gpio/gpio-amd8111.c'

lint:
	shellcheck --external-sources --shell=bash tests/*_tests.bats

test:
	PATH="$(CURDIR)/out:$(PATH)" bats --formatter $(CURDIR)/bats-format-pretty tests/*_tests.bats

filter:
	PATH="$(CURDIR)/out:$(PATH)" bats --formatter $(CURDIR)/bats-format-pretty \
		--filter "$(FILTER)" tests/*_tests.bats

install:
	nix profile install .
