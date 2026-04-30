static void bar(void) {}

static void foo(void) {
    bar();
}

static void baz(void) {
    foo();
    bar();
}
