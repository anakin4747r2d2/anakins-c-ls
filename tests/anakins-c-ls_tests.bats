#!/usr/bin/env bats

source ./tests/lsts/lsts

lsts_set_cmd "anakins-c-ls"
lsts_set_root "$(dirname "$BATS_TEST_FILENAME")"
lsts_set_langId "c"

setup() {
    lsts_start
}

teardown() {
    lsts_stop
}

@test "initializes successfully" {
    lsts_initialize
}

@test "initialize response declares hoverProvider" {
    lsts_initialize
    echo "$LSTS_RESPONSE" | jq -e '.result.capabilities.hoverProvider == true' > /dev/null
}

@test "initialize response declares textDocumentSync openClose" {
    lsts_initialize
    echo "$LSTS_RESPONSE" | jq -e '.result.capabilities.textDocumentSync.openClose == true' > /dev/null
}

@test "hover on #include returns documentation" {
    lsts_hover \
        "linux/arch/parisc/kernel/smp.c:16:2" \
        "fixtures/hover_include.rpc.json"
}

