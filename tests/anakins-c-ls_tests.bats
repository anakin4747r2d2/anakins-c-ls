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

@test "hover on break returns documentation" {
    lsts_hover \
        "linux/kernel/irq/msi.c:1190:3" \
        "fixtures/hover_break.rpc.json"
}

@test "hover on case returns documentation" {
    lsts_hover \
        "linux/kernel/irq/msi.c:170:2" \
        "fixtures/hover_case.rpc.json"
}

@test "hover on char returns documentation" {
    lsts_hover \
        "linux/kernel/irq/proc.c:315:2" \
        "fixtures/hover_char.rpc.json"
}

@test "hover on const returns documentation" {
    lsts_hover \
        "linux/kernel/irq/proc.c:50:2" \
        "fixtures/hover_const.rpc.json"
}

@test "hover on continue returns documentation" {
    lsts_hover \
        "linux/kernel/irq/msi.c:215:4" \
        "fixtures/hover_continue.rpc.json"
}

@test "hover on default returns documentation" {
    lsts_hover \
        "linux/kernel/irq/msi.c:1191:2" \
        "fixtures/hover_default.rpc.json"
}

@test "hover on do returns documentation" {
    lsts_hover \
        "linux/kernel/irq/spurious.c:70:2" \
        "fixtures/hover_do.rpc.json"
}

@test "hover on double returns documentation" {
    lsts_hover \
        "linux/arch/powerpc/kvm/book3s_paired_singles.c:1216:4" \
        "fixtures/hover_double.rpc.json"
}

@test "hover on else returns documentation" {
    lsts_hover \
        "linux/kernel/irq/msi.c:1074:2" \
        "fixtures/hover_else_kw.rpc.json"
}

@test "hover on enum returns documentation" {
    lsts_hover \
        "linux/kernel/irq/proc.c:40:1" \
        "fixtures/hover_enum.rpc.json"
}

@test "hover on extern returns documentation" {
    lsts_hover \
        "linux/arch/parisc/kernel/traps.c:806:2" \
        "fixtures/hover_extern.rpc.json"
}

@test "hover on float returns documentation" {
    lsts_hover \
        "linux/arch/powerpc/kvm/book3s_paired_singles.c:267:2" \
        "fixtures/hover_float.rpc.json"
}

@test "hover on for returns documentation" {
    lsts_hover \
        "linux/kernel/irq/msi.c:257:2" \
        "fixtures/hover_for.rpc.json"
}

@test "hover on goto returns documentation" {
    lsts_hover \
        "linux/kernel/irq/msi.c:119:4" \
        "fixtures/hover_goto.rpc.json"
}

@test "hover on if returns documentation" {
    lsts_hover \
        "linux/kernel/irq/msi.c:81:2" \
        "fixtures/hover_if_kw.rpc.json"
}

@test "hover on inline returns documentation" {
    lsts_hover \
        "linux/kernel/sched/core.c:2121:1" \
        "fixtures/hover_inline.rpc.json"
}

@test "hover on int returns documentation" {
    lsts_hover \
        "linux/kernel/irq/proc.c:102:1" \
        "fixtures/hover_int.rpc.json"
}

@test "hover on long returns documentation" {
    lsts_hover \
        "linux/kernel/umh.c:239:1" \
        "fixtures/hover_long.rpc.json"
}

@test "hover on register returns documentation" {
    lsts_hover \
        "linux/arch/parisc/kernel/traps.c:343:5" \
        "fixtures/hover_register.rpc.json"
}

@test "hover on restrict returns documentation" {
    lsts_hover \
        "linux/crypto/shash.c:110:1" \
        "fixtures/hover_restrict.rpc.json"
}

@test "hover on return returns documentation" {
    lsts_hover \
        "linux/kernel/irq/msi.c:93:2" \
        "fixtures/hover_return.rpc.json"
}

@test "hover on short returns documentation" {
    lsts_hover \
        "linux/kernel/debug/kdb/kdb_io.c:589:3" \
        "fixtures/hover_short.rpc.json"
}

@test "hover on signed returns documentation" {
    lsts_hover \
        "linux/kernel/time/sleep_timeout.c:61:1" \
        "fixtures/hover_signed.rpc.json"
}

@test "hover on sizeof returns documentation" {
    lsts_hover \
        "linux/kernel/irq/devres.c:315:5" \
        "fixtures/hover_sizeof.rpc.json"
}

@test "hover on static returns documentation" {
    lsts_hover \
        "linux/kernel/irq/dummychip.c:18:1" \
        "fixtures/hover_static.rpc.json"
}

@test "hover on variable name returns null" {
    lsts_hover \
        "linux/drivers/gpio/gpio-amd8111.c:61:35" \
        "fixtures/hover_null.rpc.json"
}

@test "hover on struct returns documentation" {
    lsts_hover \
        "linux/kernel/irq/dummychip.c:39:1" \
        "fixtures/hover_struct.rpc.json"
}

@test "hover on switch returns documentation" {
    lsts_hover \
        "linux/kernel/irq/msi.c:169:2" \
        "fixtures/hover_switch.rpc.json"
}

@test "hover on typedef returns documentation" {
    lsts_hover \
        "linux/kernel/sched/rt.c:491:1" \
        "fixtures/hover_typedef.rpc.json"
}

@test "hover on union returns documentation" {
    lsts_hover \
        "linux/kernel/irq/msi.c:1580:2" \
        "fixtures/hover_union.rpc.json"
}

@test "hover on unsigned returns documentation" {
    lsts_hover \
        "linux/kernel/irq/msi.c:33:2" \
        "fixtures/hover_unsigned.rpc.json"
}

@test "hover on void returns documentation" {
    lsts_hover \
        "linux/kernel/irq/msi.c:227:1" \
        "fixtures/hover_void.rpc.json"
}

@test "hover on volatile returns documentation" {
    lsts_hover \
        "linux/arch/arm/mach-exynos/platsmp.c:30:1" \
        "fixtures/hover_volatile.rpc.json"
}

@test "hover on while returns documentation" {
    lsts_hover \
        "linux/kernel/irq/proc.c:519:3" \
        "fixtures/hover_while.rpc.json"
}

@test "initialize response declares definitionProvider" {
    lsts_initialize
    echo "$LSTS_RESPONSE" | jq -e '.result.capabilities.definitionProvider == true' > /dev/null
}

@test "definition on goto label name jumps to label in same function" {
    lsts_definition \
        "linux/kernel/irq/msi.c:119:9" \
        "fixtures/definition_goto_fail.rpc.json"
}

@test "definition on goto label name in second function with same label name" {
    lsts_definition \
        "linux/kernel/irq/msi.c:263:9" \
        "fixtures/definition_goto_fail_second_fn.rpc.json"
}

@test "definition on goto label name in third function with same label name" {
    lsts_definition \
        "linux/kernel/irq/msi.c:543:9" \
        "fixtures/definition_goto_fail_third_fn.rpc.json"
}

@test "definition on goto fail_mem jumps to fail_mem label not fail label" {
    lsts_definition \
        "linux/kernel/irq/msi.c:260:9" \
        "fixtures/definition_goto_fail_mem.rpc.json"
}

@test "definition on local variable usage jumps to its declaration" {
    lsts_definition \
        "linux/kernel/irq/devres.c:68:2" \
        "fixtures/definition_local_var.rpc.json"
}

@test "definition on function call jumps to function definition in same file" {
    lsts_definition \
        "linux/kernel/irq/devres.c:53:20" \
        "fixtures/definition_function_call.rpc.json"
}

@test "definition on struct tag usage jumps to struct definition in same file" {
    lsts_definition \
        "linux/kernel/irq/devres.c:50:9" \
        "fixtures/definition_struct_same_file.rpc.json"
}

@test "definition on macro usage jumps to macro definition in same file" {
    lsts_definition \
        "linux/kernel/irq/msi.c:367:30" \
        "fixtures/definition_macro_same_file.rpc.json"
}

@test "definition on struct tag usage jumps to struct definition in included header" {
    lsts_definition \
        "linux/kernel/irq/proc.c:47:47" \
        "fixtures/definition_struct_header.rpc.json"
}

@test "definition on macro usage jumps to macro definition in included header" {
    lsts_definition \
        "linux/kernel/irq/msi.c:340:2" \
        "fixtures/definition_macro_header.rpc.json"
}

@test "definition on non-identifier token returns null" {
    lsts_definition \
        "linux/drivers/gpio/gpio-amd8111.c:61:66" \
        "fixtures/definition_null.rpc.json"
}

@test "initialize response declares documentSymbolProvider" {
    lsts_initialize
    echo "$LSTS_RESPONSE" | jq -e '.result.capabilities.documentSymbolProvider == true' > /dev/null
}


@test "initialize response declares callHierarchyProvider" {
    lsts_initialize
    echo "$LSTS_RESPONSE" | jq -e '.result.capabilities.callHierarchyProvider == true' > /dev/null
}

@test "callHierarchy/prepare on function name returns item" {
    lsts_initialize
    lsts_open "fixtures/call_hierarchy.c"
    lsts_call_hierarchy_prepare "fixtures/call_hierarchy.c:3:13"
    echo "$LSTS_RESPONSE" | jq -e '.result[0].name == "foo"' > /dev/null
    echo "$LSTS_RESPONSE" | jq -e '.result[0].kind == 12' > /dev/null
}

@test "callHierarchy/prepare on non-function returns null" {
    lsts_initialize
    lsts_open "fixtures/call_hierarchy.c"
    lsts_call_hierarchy_prepare "fixtures/call_hierarchy.c:1:1"
    echo "$LSTS_RESPONSE" | jq -e '.result == null' > /dev/null
}

@test "callHierarchy/incomingCalls returns callers of bar" {
    lsts_initialize
    lsts_open "fixtures/call_hierarchy.c"
    lsts_goto_incoming_calls "fixtures/call_hierarchy.c:1:13"
    echo "$LSTS_RESPONSE" | jq -e '[.result[].from.name] | sort == ["baz","foo"]' > /dev/null
}

@test "callHierarchy/outgoingCalls returns calls from foo" {
    lsts_initialize
    lsts_open "fixtures/call_hierarchy.c"
    lsts_goto_outgoing_calls "fixtures/call_hierarchy.c:3:13"
    echo "$LSTS_RESPONSE" | jq -e '[.result[].to.name] == ["bar"]' > /dev/null
}

@test "callHierarchy/outgoingCalls returns calls from baz" {
    lsts_initialize
    lsts_open "fixtures/call_hierarchy.c"
    lsts_goto_outgoing_calls "fixtures/call_hierarchy.c:7:13"
    echo "$LSTS_RESPONSE" | jq -e '([.result[].to.name] | sort) == ["bar","foo"]' > /dev/null
}
