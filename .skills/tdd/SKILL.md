---
name: tdd
description: Test-driven development workflow: write a failing test first, implement the minimal code to pass it, then refactor. Use when adding new features, fixing bugs, or refactoring existing code in this project.
---

## TDD Workflow

Follow the Red → Green → Refactor cycle strictly.

### 1. Red — Write a failing test

- Identify the smallest unit of behavior to implement.
- Write a test in `src/components/__tests__/` (or co-located `__tests__/`) using `@testing-library/react-native`.
- Use `testID` props for element queries, not text or style selectors.
- Run `npm test` and confirm the new test **fails** before writing any implementation code.

### 2. Green — Make it pass with minimal code

- Write only enough code to make the failing test pass.
- Do not add functionality that isn't covered by a test.
- Run `npm test` and confirm **all** tests pass.

### 3. Refactor — Clean up

- Remove duplication, improve naming, and tidy the implementation.
- Do **not** change behavior — the test suite must stay green throughout.
- Run `npm test` after every meaningful change.

## Example cycle

```
// 1. Red
test('shows track title', () => {
  render(<TrackRow testID="track-row" title="Duel of the Fates" />);
  expect(screen.getByTestId('track-title')).toHaveTextContent('Duel of the Fates');
});
// → npm test  (fails: TrackRow doesn't exist yet)

// 2. Green
export function TrackRow({ testID, title }) {
  return <Text testID="track-title">{title}</Text>;
}
// → npm test  (passes)

// 3. Refactor
// Extract styles, split concerns, etc. — keep tests green.
```
