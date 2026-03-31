# lsts: read -t timeout causes bats to wait after every test

## Problem

Every test that receives an LSP response takes `LSTS_TIMEOUT` seconds to
complete (default: 10s) even when the server replies immediately.

## Root cause

`lsts_recv` reads the JSON-RPC header with:

```bash
while IFS= read -r -t "${LSTS_TIMEOUT:-10}" line <&"$LSTS_READ_FD"; do
    ...
    [[ -z "$line" ]] && break
done
```

When the server sends a message, the first few iterations consume the header
lines, then the loop `break`s on the empty separator line.  At this point the
`read` call has exited normally — but bats runs each test body in a subshell
and waits for **all background timers** spawned within it to expire before
recording the test result.  Bash's `read -t N` uses an internal SIGALRM-based
timer; even after the `break`, bats sees the outstanding timer and waits for it
to fire.

The same applies to the `read -r -N $content_length -t "${LSTS_TIMEOUT:-10}"`
body read: if the body arrives quickly, the 10-second alarm still runs to
completion inside the bats subshell.

## Workaround applied here

`LSTS_TIMEOUT=2` is set in `tests/anakins-c-ls_tests.bats`.  The server
responds in under 10 ms; 2 seconds is a comfortable margin while keeping tests
fast.  This reduces per-test overhead from ~10 s to ~2 s.

## Proper fix (requires change in lsts)

The `lsts_recv` function in `tests/lsts/lsts` should close and reopen the read
file descriptor after successfully reading a complete message, so that any
in-flight `read -t` alarm is cancelled before bats waits on the subshell.  One
approach:

```bash
lsts_recv() {
    local line content_length=0 raw

    while IFS= read -r -t "${LSTS_TIMEOUT:-10}" line <&"$LSTS_READ_FD"; do
        line="${line%$'\r'}"
        [[ -z "$line" ]] && break
        if [[ "$line" =~ ^Content-Length:\ ([0-9]+)$ ]]; then
            content_length="${BASH_REMATCH[1]}"
        fi
    done

    [[ "$content_length" -eq 0 ]] && {
        echo "lsts_recv: no Content-Length header received" >&2
        return 1
    }

    IFS= read -r -N "$content_length" -t "${LSTS_TIMEOUT:-10}" raw <&"$LSTS_READ_FD"

    # Close and reopen the read FD so that any pending SIGALRM timer from the
    # read -t calls above is cancelled before returning to the caller.  Without
    # this, bats waits LSTS_TIMEOUT seconds per test for the alarm to fire.
    exec {LSTS_READ_FD}<&"$LSTS_READ_FD"

    LSTS_RESPONSE="$(printf '%s' "$raw" | jq -c .)"
}
```

Alternatively, lower the default value of `LSTS_TIMEOUT` in `lsts` itself —
the current default of 10 s is appropriate for slow servers but makes the
common case unnecessarily expensive.
