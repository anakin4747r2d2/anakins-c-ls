#!/usr/bin/env bats

# lsts uses read -N $content_length to read LSP message bodies.  bash's read -N
# counts characters in the current locale, but Content-Length is a byte count.
# Setting LC_ALL=C makes read -N count bytes, matching what the server sends.
export LC_ALL=C

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

@test "hover on #define returns documentation" {
    lsts_hover \
        "linux/arch/parisc/kernel/smp.c:52:1" \
        "fixtures/hover_define.rpc.json"
}

@test "hover on #undef returns documentation" {
    lsts_hover \
        "linux/arch/parisc/kernel/smp.c:49:1" \
        "fixtures/hover_undef.rpc.json"
}

@test "hover on #ifdef returns documentation" {
    lsts_hover \
        "linux/arch/parisc/kernel/smp.c:50:1" \
        "fixtures/hover_ifdef.rpc.json"
}

@test "hover on #ifndef returns documentation" {
    lsts_hover \
        "linux/arch/parisc/kernel/parisc_ksyms.c:39:1" \
        "fixtures/hover_ifndef.rpc.json"
}

@test "hover on #if returns documentation" {
    lsts_hover \
        "linux/arch/parisc/kernel/ptrace.c:503:1" \
        "fixtures/hover_if.rpc.json"
}

@test "hover on #elif returns documentation" {
    lsts_hover \
        "linux/arch/parisc/kernel/asm-offsets.c:281:1" \
        "fixtures/hover_elif.rpc.json"
}

@test "hover on #else returns documentation" {
    lsts_hover \
        "linux/arch/parisc/kernel/smp.c:55:1" \
        "fixtures/hover_else.rpc.json"
}

@test "hover on #endif returns documentation" {
    lsts_hover \
        "linux/arch/parisc/kernel/smp.c:57:1" \
        "fixtures/hover_endif.rpc.json"
}

@test "hover on #error returns documentation" {
    lsts_hover \
        "linux/arch/parisc/kernel/smp.c:90:1" \
        "fixtures/hover_error.rpc.json"
}

@test "hover on #warning returns documentation" {
    lsts_hover \
        "linux/arch/arm/kernel/unwind.c:18:1" \
        "fixtures/hover_warning.rpc.json"
}

@test "hover on #pragma returns documentation" {
    lsts_hover \
        "linux/arch/arm/lib/xor-neon.c:25:1" \
        "fixtures/hover_pragma.rpc.json"
}

