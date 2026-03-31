# lsts: read -N counts characters not bytes, causing timeout on UTF-8 bodies

## Problem

Every test that receives an LSP response containing multibyte UTF-8 characters
takes `LSTS_TIMEOUT` seconds (default: 10s) to complete, even when the server
replies immediately.

## Root cause

`lsts_recv` reads the JSON-RPC message body with:

```bash
IFS= read -r -N "$content_length" -t "${LSTS_TIMEOUT:-10}" raw <&"$LSTS_READ_FD"
```

`Content-Length` is specified in **bytes** by the LSP specification.  However,
bash's `read -N` counts **characters** in the current locale.  When the
response body contains multibyte UTF-8 sequences (e.g. em dashes `—`, 3 bytes
each), the byte count exceeds the character count.  `read -N` waits for more
characters that never arrive, blocking until `LSTS_TIMEOUT` fires.

### Concrete example

The hover response for `#include` contains two em dashes:

```
Content-Length: 1007   ← byte count
...body (1003 chars, 1007 bytes: 2 × em dash = 2 × 3 bytes)
```

In a UTF-8 locale `read -N 1007` expects 1007 characters but only 1003 are in
the body, so it waits 10 seconds for 4 more characters that never arrive.

## Workaround applied here

`export LC_ALL=C` is set at the top of `tests/anakins-c-ls_tests.bats`, before
sourcing `lsts`.  In the C locale `read -N` counts bytes, matching
`Content-Length` exactly.  This reduces the hover test from ~10 s to ~55 ms.

## Proper fix (requires change in lsts)

`lsts_recv` in `tests/lsts/lsts` should force byte semantics for the body read:

```bash
IFS= LC_ALL=C read -r -N "$content_length" -t "${LSTS_TIMEOUT:-10}" raw <&"$LSTS_READ_FD"
```

Prefixing the `read` command with `LC_ALL=C` overrides the locale for that
single command without affecting the rest of the script.
