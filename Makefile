.PHONY: all build test filter lint install

FILTER ?= .
CC ?= cc
CFLAGS ?= -Wall -Wextra
TREE_SITTER_INC ?= /usr/include
TREE_SITTER_LIB ?= /usr/lib
TREE_SITTER_GRAMMARS ?= /usr/lib

all:
	nix develop --command make build test lint

build:
	mkdir -p out
	$(CC) $(CFLAGS) \
		-Isrc \
		-I$(TREE_SITTER_INC) \
		-o out/anakins-c-ls src/main.c \
		-L$(TREE_SITTER_LIB) \
		-L$(TREE_SITTER_GRAMMARS) \
		-ltree-sitter \
		-l:c.so

lint:
	shellcheck --external-sources --shell=bash tests/*_tests.bats

test:
	bats --formatter $(CURDIR)/tests/lsts/lsts-format-pretty tests/*_tests.bats

filter:
	bats --formatter $(CURDIR)/tests/lsts/lsts-format-pretty \
		--filter "$(FILTER)" tests/*_tests.bats

install:
	nix profile install .
