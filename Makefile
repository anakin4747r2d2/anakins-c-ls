.PHONY: all dev test filter lint install build

PREFIX ?= /usr/local

INSTALL_DIR = $(PREFIX)/bin

FILTER ?= .

TERM_CMD = gnome-terminal --full-screen --

all:
	cqfd run make build test lint

build:
	mkdir -p out
	cc -Wall -Wextra -o out/anakins-c-ls src/main.c

dev: build
	rm -f *.log
	$(TERM_CMD) bash -c 'exec \
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
	install -Dm755 out/anakins-c-ls $(INSTALL_DIR)/anakins-c-ls
