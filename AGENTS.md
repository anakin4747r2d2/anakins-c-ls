
# Anakin's C Language Server

## Build Environment

This project uses cqfd for containerized builds. Always load the cqfd skill at
the start of every session:

```
/skill cqfd
```

Never install build dependencies on the host or run build commands directly.
All compilation, testing, and linting must go through cqfd.

## Running Tests

Run the full test suite:

```
cqfd run make build test
```

Run a single test or a subset of tests by name using the `filter` target with
a regex matched against test descriptions:

```
cqfd run make filter FILTER="hover"
```

The `FILTER` value is passed to `bats --filter` and supports any regular
expression. Omitting `FILTER` defaults to `.` (all tests).

## Git Best Practices

- **Commit Often:** Make small, focused commits.
- **Descriptive Messages:** Use clear, imperative commit messages.
- **Test Driven Development:**
  - Write tests before implementing features or fixes.
  - After creating the test, stop and request approval before proceeding.
- **.gitignore:** Exclude build artifacts and sensitive files.
- **No Secrets:** Never commit passwords or API keys.
- **Whitespace:** Never leave trailing whitespace

Commit every logical atomic addition or change using git. Each commit should
represent one coherent unit of work (e.g. add a helper function, fix a bug,
update the Dockerfile). Do not batch unrelated changes into a single commit. Commit
messages should be concise and written in the imperative mood.

## Code review

After creating a commit code review your work. Ask the following questions to
see how the code can be made cleaner:
- Did your change create any dead code?
- Did your change invalidate some comments?
- Is your change in the style of the codebase?
- Can guard clauses be used to avoid indenting?
- Can this code be refactored into a function to improve readibility?
- Is this commit atomic and only focused on one topic?
- Did anything unrelated get included in the commit by accident?
- Does this code reuse functionality already present in the codebase?
- Did the commit duplicate any functionality already present in the codebase?
- Is there a simplier way to implement this solution?
- Did you add anything extra functionality that doesn't have a corresponding
  test?

Fix the code accordingly in new commits.

## Documentation

The C language server should pull documentation for C language features from
tests/gcc/

## Test Fixtures

Always use real source files from `tests/linux/` as test fixture inputs when
possible. Do not create synthetic `.c` files solely to act as test targets.
Prefer pointing `lsts_hover`, `lsts_definition`, etc. at actual Linux kernel
source files with real line/column coordinates.
