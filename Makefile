.PHONY: all build test filter lint install

FILTER ?= .
CC     ?= cc
CFLAGS ?= -Wall -Wextra

TREE_SITTER_CFLAGS  ?= $(shell pkg-config --cflags tree-sitter 2>/dev/null)
TREE_SITTER_LIBS    ?= $(shell pkg-config --libs tree-sitter 2>/dev/null) -ltree-sitter -l:c.so
TREE_SITTER_GRAMMARS ?=

all:
	nix develop --command make build test lint

build:
	mkdir -p out
	$(CC) $(CFLAGS) $(TREE_SITTER_CFLAGS) \
		-Isrc \
		-o out/anakins-c-ls src/main.c \
		$(if $(TREE_SITTER_GRAMMARS),-L$(TREE_SITTER_GRAMMARS)) \
		$(TREE_SITTER_LIBS)

lint:
	shellcheck --external-sources --shell=bash tests/*_tests.bats

test:
	bats --formatter $(CURDIR)/tests/lsts/lsts-format-pretty tests/*_tests.bats

filter:
	bats --formatter $(CURDIR)/tests/lsts/lsts-format-pretty \
		--filter "$(FILTER)" tests/*_tests.bats

install:
	nix profile install .
