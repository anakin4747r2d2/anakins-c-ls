#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Documentation for CPP directives, sourced from
 * tests/gcc/gcc/doc/cpp.texi.
 * Documentation for C keywords, sourced from
 * tests/gcc/gcc/doc/extend.texi and
 * tests/gcc/gcc/doc/implement-c.texi. */

/* Include Syntax section */
#define INCLUDE_DOC \
    "## `#include`\n" \
    "\n" \
    "Both user and system header files are included using the preprocessing directive `#include`. It has two variants:\n" \
    "\n" \
    "- `#include <file>` \xe2\x80\x94 Used for system header files. Searches for a file named *file* in a standard list of system directories. You can prepend directories to this list with the `-I` option.\n" \
    "\n" \
    "- `#include \"file\"` \xe2\x80\x94 Used for header files of your own program. Searches for a file named *file* first in the directory containing the current file, then in the quote directories and then the same directories used for `<file>`. You can prepend directories to the list of quote directories with the `-iquote` option.\n" \
    "\n" \
    "The argument of `#include`, whether delimited with quote marks or angle brackets, behaves like a string constant in that comments are not recognized, and macro names are not expanded.\n" \
    "\n" \
    "It is an error if there is anything (other than comments) on the line after the file name."

/* Object-like Macros section */
#define DEFINE_DOC \
    "## `#define`\n" \
    "\n" \
    "You create macros with the `#define` directive. `#define` is followed by the name of the macro and then the token sequence it should be an abbreviation for, which is variously referred to as the macro's *body*, *expansion* or *replacement list*.\n" \
    "\n" \
    "The macro's body ends at the end of the `#define` line. You may continue the definition onto multiple lines, if necessary, using backslash-newline. When the macro is expanded, however, it will all come out on one line.\n" \
    "\n" \
    "By convention, macro names are written in uppercase. Programs are easier to read when it is possible to tell at a glance which names are macros."

/* Undefining and Redefining Macros section */
#define UNDEF_DOC \
    "## `#undef`\n" \
    "\n" \
    "If a macro ceases to be useful, it may be undefined with the `#undef` directive. `#undef` takes a single argument, the name of the macro to undefine. You use the bare macro name, even if the macro is function-like. It is an error if anything appears on the line after the macro name. `#undef` has no effect if the name is not a macro.\n" \
    "\n" \
    "Once a macro has been undefined, that identifier may be redefined as a macro by a subsequent `#define` directive. The new definition need not have any resemblance to the old definition."

/* Ifdef subsection */
#define IFDEF_DOC \
    "## `#ifdef`\n" \
    "\n" \
    "`#ifdef MACRO` is the simplest form of conditional. The controlled text is included in the output if and only if `MACRO` is defined. The conditional group must be closed with `#endif`.\n" \
    "\n" \
    "Controlled text inside a conditional can include preprocessing directives, executed only if the conditional succeeds. Conditional groups may be nested but must be completely nested."

/* Ifdef subsection (#ifndef portion) */
#define IFNDEF_DOC \
    "## `#ifndef`\n" \
    "\n" \
    "`#ifndef` is used to include code only when a macro is *not* defined. You can use `#ifndef` instead of `#ifdef` to invert the sense of the test.\n" \
    "\n" \
    "One common use of `#ifndef` is to include code only the first time a header file is included (include guards)."

/* If subsection */
#define IF_DOC \
    "## `#if`\n" \
    "\n" \
    "The `#if` directive allows you to test the value of an arithmetic expression, rather than the mere existence of one macro. The expression may contain integer constants, character constants, arithmetic and logical operators, macros, and uses of the `defined` operator.\n" \
    "\n" \
    "If the value of the expression comes out to be nonzero, the `#if` succeeds and the controlled text is included; otherwise it is skipped."

/* Elif subsection */
#define ELIF_DOC \
    "## `#elif`\n" \
    "\n" \
    "`#elif` stands for \"else if\". Like `#else`, it goes in the middle of a conditional group and subdivides it; it does not require a matching `#endif` of its own. Like `#if`, the `#elif` directive includes an expression to be tested.\n" \
    "\n" \
    "The text following `#elif` is processed only if the original `#if`-condition failed and the `#elif` condition succeeds. More than one `#elif` can go in the same conditional group."

/* Else subsection */
#define ELSE_DOC \
    "## `#else`\n" \
    "\n" \
    "The `#else` directive can be added to a conditional to provide alternative text to be used if the condition fails. If the expression in the opening `#if` (or `#ifdef`/`#ifndef`) is nonzero, the text before `#else` is included and the text after is skipped. If the expression is zero, the opposite happens.\n" \
    "\n" \
    "You can use `#else` with `#ifdef` and `#ifndef`, too."

/* Ifdef subsection (#endif portion) */
#define ENDIF_DOC \
    "## `#endif`\n" \
    "\n" \
    "`#endif` closes a conditional group opened by `#if`, `#ifdef`, or `#ifndef`. Every conditional must be terminated by a matching `#endif`.\n" \
    "\n" \
    "A comment following `#endif` naming the tested macro is good practice when there is a lot of controlled text, because it helps readers match the `#endif` to the corresponding opening directive."

/* Diagnostics chapter */
#define ERROR_DOC \
    "## `#error`\n" \
    "\n" \
    "The directive `#error` causes the preprocessor to report a fatal error. The tokens forming the rest of the line following `#error` are used as the error message.\n" \
    "\n" \
    "You would use `#error` inside of a conditional that detects a combination of parameters which you know the program does not properly support."

/* Diagnostics chapter (#warning portion) */
#define WARNING_DOC \
    "## `#warning`\n" \
    "\n" \
    "The directive `#warning` is like `#error`, but causes the preprocessor to issue a warning and continue preprocessing. The tokens following `#warning` are used as the warning message.\n" \
    "\n" \
    "You might use `#warning` in obsolete header files, with a message directing the user to the header file which should be used instead."

/* Pragmas chapter */
#define PRAGMA_DOC \
    "## `#pragma`\n" \
    "\n" \
    "The `#pragma` directive is the method specified by the C standard for providing additional information to the compiler, beyond what is conveyed in the language itself. The forms specified by the C standard are prefixed with `STDC`. Most GNU-defined pragmas have been given a `GCC` prefix.\n" \
    "\n" \
    "C99 introduced the `_Pragma` operator, which addresses a major problem with `#pragma`: being a directive, it cannot be produced as the result of macro expansion. `_Pragma` is an operator that can be embedded in a macro."

/* C keywords — implement-c.texi: Hints implementation */
#define BREAK_DOC \
    "## `break`\n" \
    "\n" \
    "`break` terminates the nearest enclosing `for`, `while`, `do`, or `switch` statement. Execution continues with the statement that follows the terminated statement."

#define CASE_DOC \
    "## `case`\n" \
    "\n" \
    "`case` introduces a labeled statement inside a `switch` body. The expression must be an integer constant expression. Execution jumps to the matching `case` label when the `switch` expression equals its value.\n" \
    "\n" \
    "A `switch` statement may contain any number of `case` labels. Duplicate `case` values in the same `switch` are not permitted."

/* implement-c.texi: Characters implementation */
#define CHAR_DOC \
    "## `char`\n" \
    "\n" \
    "`char` is the basic character type. It is large enough to hold any member of the basic execution character set. Whether plain `char` is signed or unsigned is implementation-defined; GCC follows the ABI, and the behavior can be changed with `-funsigned-char` or `-fsigned-char`."

/* implement-c.texi: Qualifiers implementation */
#define CONST_DOC \
    "## `const`\n" \
    "\n" \
    "`const` is a type qualifier that declares an object whose value may not be changed after initialization. Attempts to modify a `const`-qualified object result in undefined behavior.\n" \
    "\n" \
    "`const` is commonly used to protect objects passed by pointer from modification and to declare read-only global data."

#define CONTINUE_DOC \
    "## `continue`\n" \
    "\n" \
    "`continue` skips the remainder of the body of the nearest enclosing `for`, `while`, or `do` statement and jumps to the loop's continuation expression (in `for`) or condition test."

/* implement-c.texi: Statements implementation */
#define DEFAULT_DOC \
    "## `default`\n" \
    "\n" \
    "`default` introduces the default label inside a `switch` body. If no `case` label matches the `switch` expression, execution jumps to the `default` label. A `switch` statement may have at most one `default` label."

#define DO_DOC \
    "## `do`\n" \
    "\n" \
    "`do` introduces a do-while loop. The body is executed at least once; the controlling expression is evaluated after each execution of the body. The loop continues as long as the expression is nonzero.\n" \
    "\n" \
    "```c\n" \
    "do {\n" \
    "    /* body */\n" \
    "} while (condition);\n" \
    "```"

/* implement-c.texi: Floating point implementation */
#define DOUBLE_DOC \
    "## `double`\n" \
    "\n" \
    "`double` is a floating-point type. It typically provides at least as much precision as `float`. GCC follows C99 Annex F for floating-point rounding and conversion rules.\n" \
    "\n" \
    "`long double` provides extended precision on many platforms."

#define ELSE_KW_DOC \
    "## `else`\n" \
    "\n" \
    "`else` provides an alternative branch for an `if` statement. If the controlling expression is zero, the statement following `else` is executed instead of the `if` body.\n" \
    "\n" \
    "`else` is always associated with the nearest preceding `if` that lacks an `else` branch."

/* implement-c.texi: Structures unions enumerations and bit-fields implementation */
#define ENUM_DOC \
    "## `enum`\n" \
    "\n" \
    "`enum` declares an enumerated type — a set of named integer constants. The underlying integer type is normally `unsigned int` if there are no negative values, otherwise `int`. With `-fshort-enums`, smaller types may be chosen.\n" \
    "\n" \
    "Enumerated types and their members may be used in any integer expression."

#define EXTERN_DOC \
    "## `extern`\n" \
    "\n" \
    "`extern` declares an identifier as having external linkage — its definition is expected in another translation unit or later in the same translation unit. For an object, `extern` suppresses storage allocation at the declaration site.\n" \
    "\n" \
    "`extern` combined with `inline` has the effect of making the definition available only for inlining; calls that cannot be inlined reference an external definition."

/* implement-c.texi: Floating point implementation */
#define FLOAT_DOC \
    "## `float`\n" \
    "\n" \
    "`float` is a single-precision floating-point type. It typically corresponds to the IEEE 754 binary32 format. GCC follows C99 Annex F for floating-point rounding and conversion rules."

#define FOR_DOC \
    "## `for`\n" \
    "\n" \
    "`for` introduces an iteration statement with three optional clauses separated by semicolons: an initialization expression, a controlling expression, and a post-iteration expression.\n" \
    "\n" \
    "```c\n" \
    "for (init; condition; update) {\n" \
    "    /* body */\n" \
    "}\n" \
    "```\n" \
    "\n" \
    "If the controlling expression is omitted it is treated as nonzero. The loop executes as long as the controlling expression is nonzero."

#define GOTO_DOC \
    "## `goto`\n" \
    "\n" \
    "`goto` transfers control unconditionally to a labeled statement in the same function. The target label must be visible in the function scope.\n" \
    "\n" \
    "Jumping into the scope of a variable-length array or a variably modified type is not permitted."

#define IF_KW_DOC \
    "## `if`\n" \
    "\n" \
    "`if` introduces a conditional statement. The controlling expression is evaluated; if it is nonzero the `if` body is executed. An optional `else` branch is executed when the expression is zero.\n" \
    "\n" \
    "`if`-`else` chains can be used to test a sequence of conditions."

/* extend.texi: Inline section */
#define INLINE_DOC \
    "## `inline`\n" \
    "\n" \
    "By declaring a function `inline`, you can direct GCC to make calls to that function faster. One way GCC can achieve this is to integrate that function's code into the code for its callers, eliminating function-call overhead.\n" \
    "\n" \
    "When `inline` is used on a `static` function, GCC will not output assembler code for that function unless its address is taken or it cannot be inlined. When `inline` and `extern` are both specified, the definition is used only for inlining and is never compiled on its own.\n" \
    "\n" \
    "GCC does not inline any functions when not optimizing unless the `always_inline` attribute is used."

/* implement-c.texi: Integers implementation */
#define INT_DOC \
    "## `int`\n" \
    "\n" \
    "`int` is the basic signed integer type. Its width is at least 16 bits; on most platforms it is 32 bits. GCC uses two's complement representation for all signed integer types, and all bit patterns are ordinary values."

#define LONG_DOC \
    "## `long`\n" \
    "\n" \
    "`long` is a signed integer type that is at least 32 bits wide and at least as wide as `int`. The `long long` extension provides a type of at least 64 bits.\n" \
    "\n" \
    "GCC uses two's complement representation for all signed integer types."

/* implement-c.texi: Hints implementation */
#define REGISTER_DOC \
    "## `register`\n" \
    "\n" \
    "The `register` storage-class specifier is a hint that the variable should be placed in a processor register. Under `-O0`, variables without `register` are always placed on the stack; with `register`, the variable may have a shorter lifespan and may never be placed in memory.\n" \
    "\n" \
    "On some x86 targets `register` is required for variables to be saved across `setjmp`."

/* extend.texi: Restricted Pointers section */
#define RESTRICT_DOC \
    "## `restrict`\n" \
    "\n" \
    "`restrict` (C99) is a type qualifier applicable to pointer types. It asserts that the object pointed to is accessed in the function only through that pointer, allowing the compiler to make optimizations that would be forbidden if the pointer could alias other objects.\n" \
    "\n" \
    "GCC also accepts `__restrict__` and `__restrict` as synonyms, which can be used in C90 mode and in C++."

#define RETURN_DOC \
    "## `return`\n" \
    "\n" \
    "`return` terminates execution of the current function and optionally returns a value to the caller. A `return` with no expression may only appear in a function whose return type is `void`.\n" \
    "\n" \
    "The returned expression is implicitly converted to the function's return type."

/* implement-c.texi: Integers implementation */
#define SHORT_DOC \
    "## `short`\n" \
    "\n" \
    "`short` (or `short int`) is a signed integer type that is at least 16 bits wide and no wider than `int`. GCC uses two's complement representation for all signed integer types."

#define SIGNED_DOC \
    "## `signed`\n" \
    "\n" \
    "`signed` explicitly marks an integer type as signed. For `char`, whether plain `char` is signed or unsigned is implementation-defined; `signed char` is always signed. For `int`, `short`, and `long`, `signed` is the default and may be omitted."

/* implement-c.texi: Architecture implementation */
#define SIZEOF_DOC \
    "## `sizeof`\n" \
    "\n" \
    "`sizeof` is an operator that yields the size in bytes of its operand, which may be a type name in parentheses or an expression. The result is a constant of type `size_t`.\n" \
    "\n" \
    "For a variable-length array, `sizeof` is evaluated at run time. For other types, it is a compile-time constant determined by the ABI."

#define STATIC_DOC \
    "## `static`\n" \
    "\n" \
    "For a variable at block scope, `static` gives it static storage duration — it is initialized once and persists for the lifetime of the program. For a variable or function at file scope, `static` gives it internal linkage — it is not visible outside the translation unit.\n" \
    "\n" \
    "For a `static inline` function, GCC will not emit object code for the function unless it is needed; all calls will be inlined if possible."

/* implement-c.texi: Structures unions enumerations and bit-fields implementation */
#define STRUCT_DOC \
    "## `struct`\n" \
    "\n" \
    "`struct` declares an aggregate type whose members are laid out sequentially in memory. Each member has its own storage. The layout — padding between members and overall alignment — is determined by the ABI.\n" \
    "\n" \
    "GCC permits flexible array members (a trailing array member with no specified size) and unnamed struct/union members as extensions."

/* implement-c.texi: Statements implementation */
#define SWITCH_DOC \
    "## `switch`\n" \
    "\n" \
    "`switch` transfers control to one of several statements depending on the value of an integer expression. The expression is compared against the constant expressions in `case` labels; if no match is found, control transfers to the `default` label if one exists.\n" \
    "\n" \
    "A `switch` body may contain any number of `case` labels and at most one `default` label."

/* implement-c.texi: Types implementation */
#define TYPEDEF_DOC \
    "## `typedef`\n" \
    "\n" \
    "`typedef` introduces an alias name for a type. The new name may be used wherever the original type name could be used. `typedef` does not create a new type — it merely provides an alternative name for an existing type."

/* implement-c.texi: Structures unions enumerations and bit-fields implementation */
#define UNION_DOC \
    "## `union`\n" \
    "\n" \
    "`union` declares a type whose members share the same storage. The size of the union is the size of its largest member. Only one member can hold a value at a time.\n" \
    "\n" \
    "Accessing a union member of a different type than the last stored member — type-punning — treats the stored bytes as an object of the accessed type. This may produce a trap representation."

/* implement-c.texi: Integers implementation */
#define UNSIGNED_DOC \
    "## `unsigned`\n" \
    "\n" \
    "`unsigned` marks an integer type as unsigned — it can represent only non-negative values but with twice the positive range of the corresponding signed type. For `char`, `unsigned char` is always unsigned regardless of platform defaults."

#define VOID_DOC \
    "## `void`\n" \
    "\n" \
    "`void` as a return type indicates that a function returns no value. As a parameter list `(void)` explicitly declares that the function takes no arguments.\n" \
    "\n" \
    "`void *` is a generic pointer type that can hold any data pointer and may be converted to and from any other data pointer type without a cast."

/* extend.texi: Volatiles section */
#define VOLATILE_DOC \
    "## `volatile`\n" \
    "\n" \
    "`volatile` is a type qualifier that prevents the compiler from optimizing away accesses to the qualified object. Volatile objects are normally used for memory-mapped hardware registers or for variables shared with signal handlers or other threads.\n" \
    "\n" \
    "The standard encourages compilers to refrain from reordering or combining volatile accesses across sequence points, but does not prevent reordering with respect to non-volatile accesses. For ordering guarantees between threads, a stronger memory barrier is required."

#define WHILE_DOC \
    "## `while`\n" \
    "\n" \
    "`while` introduces a loop that executes its body as long as the controlling expression is nonzero. The expression is evaluated before each execution of the body; if it is zero on the first evaluation, the body is never executed."

#define MAX_DOCS   64
#define MAX_URI    1024
#define MAX_MSG    (1 << 20)  /* 1 MiB */

/* ---------- document store ---------- */

typedef struct {
    char  uri[MAX_URI];
    char *text;      /* heap-allocated */
} Doc;

static Doc   docs[MAX_DOCS];
static int   ndocs = 0;

static Doc *doc_find(const char *uri)
{
    for (int i = 0; i < ndocs; i++)
        if (strcmp(docs[i].uri, uri) == 0)
            return &docs[i];
    return NULL;
}

static void doc_store(const char *uri, const char *text)
{
    Doc *d = doc_find(uri);
    if (!d) {
        if (ndocs >= MAX_DOCS) return;
        d = &docs[ndocs++];
        strncpy(d->uri, uri, MAX_URI - 1);
        d->uri[MAX_URI - 1] = '\0';
        d->text = NULL;
    }
    free(d->text);
    d->text = strdup(text);
}

/* ---------- I/O ---------- */

static char *read_message(void)
{
    int content_length = 0;
    char line[256];

    while (fgets(line, sizeof(line), stdin)) {
        /* strip \r\n */
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\r' || line[len-1] == '\n'))
            line[--len] = '\0';

        if (len == 0) break; /* blank line = end of headers */

        if (strncasecmp(line, "content-length:", 15) == 0) {
            content_length = atoi(line + 15);
        }
    }

    if (content_length <= 0) return NULL;

    char *buf = malloc(content_length + 1);
    if (!buf) return NULL;

    if ((int)fread(buf, 1, content_length, stdin) != content_length) {
        free(buf);
        return NULL;
    }
    buf[content_length] = '\0';
    return buf;
}

static void send_message(const char *body)
{
    printf("Content-Length: %zu\r\n\r\n%s", strlen(body), body);
    fflush(stdout);
}

/* ---------- tiny JSON helpers ---------- */

/* Return the value of a string field in a flat JSON object, written into
 * out (size outsz).  Returns 1 on success, 0 if not found. */
static int json_get_string(const char *json, const char *key,
                           char *out, size_t outsz)
{
    char needle[128];
    snprintf(needle, sizeof(needle), "\"%s\"", key);
    const char *p = strstr(json, needle);
    if (!p) return 0;
    p += strlen(needle);
    while (*p == ' ' || *p == ':' || *p == '\t') p++;
    if (*p != '"') return 0;
    p++; /* skip opening quote */
    size_t i = 0;
    while (*p && *p != '"' && i + 1 < outsz) {
        if (*p == '\\') {
            p++;
            switch (*p) {
            case '"':  out[i++] = '"';  break;
            case '\\': out[i++] = '\\'; break;
            case '/':  out[i++] = '/';  break;
            case 'n':  out[i++] = '\n'; break;
            case 'r':  out[i++] = '\r'; break;
            case 't':  out[i++] = '\t'; break;
            default:   out[i++] = *p;   break;
            }
        } else {
            out[i++] = *p;
        }
        p++;
    }
    out[i] = '\0';
    return 1;
}

/* Return the integer value of a field, or -1 if not found. */
static int json_get_int(const char *json, const char *key)
{
    char needle[128];
    snprintf(needle, sizeof(needle), "\"%s\"", key);
    const char *p = strstr(json, needle);
    if (!p) return -1;
    p += strlen(needle);
    while (*p == ' ' || *p == ':' || *p == '\t') p++;
    if (*p < '0' || *p > '9') return -1;
    return atoi(p);
}

/* Return the raw id token (number or quoted string) from a JSON-RPC message.
 * Written into out; returns 1 on success. */
static int json_get_id(const char *json, char *out, size_t outsz)
{
    const char *p = strstr(json, "\"id\"");
    if (!p) return 0;
    p += 4;
    while (*p == ' ' || *p == ':' || *p == '\t') p++;
    size_t i = 0;
    if (*p == '"') {
        out[i++] = '"';
        p++;
        while (*p && *p != '"' && i + 1 < outsz) out[i++] = *p++;
        if (*p == '"' && i + 1 < outsz) out[i++] = '"';
        p++;
    } else {
        while ((*p >= '0' && *p <= '9') && i + 1 < outsz) out[i++] = *p++;
    }
    out[i] = '\0';
    return i > 0;
}

/* ---------- LSP helpers ---------- */

static void send_null_result(const char *id)
{
    char body[256];
    snprintf(body, sizeof(body),
             "{\"jsonrpc\":\"2.0\",\"id\":%s,\"result\":null}", id);
    send_message(body);
}

static void send_initialize_result(const char *id)
{
    char body[256];
    snprintf(body, sizeof(body),
             "{\"jsonrpc\":\"2.0\",\"id\":%s,"
             "\"result\":{\"capabilities\":{"
             "\"hoverProvider\":true,"
             "\"textDocumentSync\":{\"openClose\":true}"
             "}}}", id);
    send_message(body);
}

static void send_hover_result(const char *id, const char *markdown)
{
    /* Build the body dynamically; markdown may be long. */
    size_t mdlen = strlen(markdown);
    size_t bufsz = mdlen * 2 + 256; /* generous room for escaping */
    char *body = malloc(bufsz);
    if (!body) return;

    /* JSON-encode the markdown value */
    char *enc = malloc(mdlen * 2 + 4);
    if (!enc) { free(body); return; }
    size_t j = 0;
    enc[j++] = '"';
    for (size_t i = 0; i < mdlen; i++) {
        switch (markdown[i]) {
        case '"':  enc[j++] = '\\'; enc[j++] = '"';  break;
        case '\\': enc[j++] = '\\'; enc[j++] = '\\'; break;
        case '\n': enc[j++] = '\\'; enc[j++] = 'n';  break;
        case '\r': enc[j++] = '\\'; enc[j++] = 'r';  break;
        case '\t': enc[j++] = '\\'; enc[j++] = 't';  break;
        default:   enc[j++] = markdown[i]; break;
        }
    }
    enc[j++] = '"';
    enc[j]   = '\0';

    snprintf(body, bufsz,
             "{\"jsonrpc\":\"2.0\",\"id\":%s,"
             "\"result\":{\"contents\":{\"kind\":\"markdown\","
             "\"value\":%s}}}",
             id, enc);

    send_message(body);
    free(enc);
    free(body);
}

/* ---------- handlers ---------- */

static void handle_did_open(const char *msg)
{
    /* Extract uri and text from the nested params.textDocument object. */
    const char *td = strstr(msg, "\"textDocument\"");
    if (!td) return;

    char uri[MAX_URI];
    if (!json_get_string(td, "uri", uri, sizeof(uri))) return;

    /* text can be large; find it manually */
    const char *tp = strstr(td, "\"text\"");
    if (!tp) return;
    tp += 6; /* skip "text" */
    while (*tp == ' ' || *tp == ':' || *tp == '\t') tp++;
    if (*tp != '"') return;
    tp++;

    char *text = malloc(MAX_MSG);
    if (!text) return;
    size_t i = 0;
    while (*tp && i + 1 < MAX_MSG) {
        if (*tp == '\\') {
            tp++;
            switch (*tp) {
            case '"':  text[i++] = '"';  break;
            case '\\': text[i++] = '\\'; break;
            case '/':  text[i++] = '/';  break;
            case 'n':  text[i++] = '\n'; break;
            case 'r':  text[i++] = '\r'; break;
            case 't':  text[i++] = '\t'; break;
            default:   text[i++] = *tp;  break;
            }
        } else if (*tp == '"') {
            break;
        } else {
            text[i++] = *tp;
        }
        tp++;
    }
    text[i] = '\0';

    doc_store(uri, text);
    free(text);
}

static void handle_hover(const char *msg, const char *id)
{
    char uri[MAX_URI];
    if (!json_get_string(msg, "uri", uri, sizeof(uri))) {
        send_null_result(id);
        return;
    }

    int line = json_get_int(msg, "line");
    if (line < 0) {
        send_null_result(id);
        return;
    }

    int character = json_get_int(msg, "character");
    if (character < 0) {
        send_null_result(id);
        return;
    }

    Doc *d = doc_find(uri);
    if (!d || !d->text) {
        send_null_result(id);
        return;
    }

    /* Walk to the requested 0-based line */
    const char *p = d->text;
    for (int ln = 0; ln < line; ln++) {
        p = strchr(p, '\n');
        if (!p) { send_null_result(id); return; }
        p++;
    }

    /* Advance to the cursor column, then walk back to the start of the token.
     * This ensures we match the word under the cursor, not the first word on
     * the line. */
    const char *line_start = p;
    for (int col = 0; col < character && *p && *p != '\n'; col++)
        p++;
    while (p > line_start && (isalnum((unsigned char)p[-1]) || p[-1] == '_' || p[-1] == '#'))
        p--;

    /* Check for a CPP directive or C keyword at the token start.
     * CPP directives: longer prefixes are checked before shorter ones to avoid
     * false prefix matches (e.g. #ifdef before #if).
     * C keywords: use kw() to require a word boundary after the keyword. */
#define kw(s) (strncmp(p, s, sizeof(s)-1) == 0 && \
               !isalnum((unsigned char)p[sizeof(s)-1]) && \
               p[sizeof(s)-1] != '_')
    if (strncmp(p, "#include", 8) == 0)
        send_hover_result(id, INCLUDE_DOC);
    else if (strncmp(p, "#ifndef", 7) == 0)
        send_hover_result(id, IFNDEF_DOC);
    else if (strncmp(p, "#ifdef", 6) == 0)
        send_hover_result(id, IFDEF_DOC);
    else if (strncmp(p, "#endif", 6) == 0)
        send_hover_result(id, ENDIF_DOC);
    else if (strncmp(p, "#elif", 5) == 0)
        send_hover_result(id, ELIF_DOC);
    else if (strncmp(p, "#else", 5) == 0)
        send_hover_result(id, ELSE_DOC);
    else if (strncmp(p, "#define", 7) == 0)
        send_hover_result(id, DEFINE_DOC);
    else if (strncmp(p, "#undef", 6) == 0)
        send_hover_result(id, UNDEF_DOC);
    else if (strncmp(p, "#if", 3) == 0)
        send_hover_result(id, IF_DOC);
    else if (strncmp(p, "#error", 6) == 0)
        send_hover_result(id, ERROR_DOC);
    else if (strncmp(p, "#warning", 8) == 0)
        send_hover_result(id, WARNING_DOC);
    else if (strncmp(p, "#pragma", 7) == 0)
        send_hover_result(id, PRAGMA_DOC);
    /* C keywords — longer keywords before shorter to avoid prefix matches */
    else if (kw("continue"))
        send_hover_result(id, CONTINUE_DOC);
    else if (kw("default"))
        send_hover_result(id, DEFAULT_DOC);
    else if (kw("double"))
        send_hover_result(id, DOUBLE_DOC);
    else if (kw("extern"))
        send_hover_result(id, EXTERN_DOC);
    else if (kw("inline"))
        send_hover_result(id, INLINE_DOC);
    else if (kw("register"))
        send_hover_result(id, REGISTER_DOC);
    else if (kw("restrict"))
        send_hover_result(id, RESTRICT_DOC);
    else if (kw("return"))
        send_hover_result(id, RETURN_DOC);
    else if (kw("signed"))
        send_hover_result(id, SIGNED_DOC);
    else if (kw("sizeof"))
        send_hover_result(id, SIZEOF_DOC);
    else if (kw("static"))
        send_hover_result(id, STATIC_DOC);
    else if (kw("struct"))
        send_hover_result(id, STRUCT_DOC);
    else if (kw("switch"))
        send_hover_result(id, SWITCH_DOC);
    else if (kw("typedef"))
        send_hover_result(id, TYPEDEF_DOC);
    else if (kw("union"))
        send_hover_result(id, UNION_DOC);
    else if (kw("unsigned"))
        send_hover_result(id, UNSIGNED_DOC);
    else if (kw("volatile"))
        send_hover_result(id, VOLATILE_DOC);
    else if (kw("while"))
        send_hover_result(id, WHILE_DOC);
    else if (kw("break"))
        send_hover_result(id, BREAK_DOC);
    else if (kw("case"))
        send_hover_result(id, CASE_DOC);
    else if (kw("char"))
        send_hover_result(id, CHAR_DOC);
    else if (kw("const"))
        send_hover_result(id, CONST_DOC);
    else if (kw("else"))
        send_hover_result(id, ELSE_KW_DOC);
    else if (kw("enum"))
        send_hover_result(id, ENUM_DOC);
    else if (kw("float"))
        send_hover_result(id, FLOAT_DOC);
    else if (kw("goto"))
        send_hover_result(id, GOTO_DOC);
    else if (kw("long"))
        send_hover_result(id, LONG_DOC);
    else if (kw("short"))
        send_hover_result(id, SHORT_DOC);
    else if (kw("void"))
        send_hover_result(id, VOID_DOC);
    else if (kw("auto"))
        send_hover_result(id, "## `auto`\n\n`auto` is the default storage class for variables declared inside a block. It gives the variable automatic storage duration — the variable exists from its declaration to the end of the enclosing block. The `auto` keyword is almost never written explicitly in C.");
    else if (kw("do"))
        send_hover_result(id, DO_DOC);
    else if (kw("for"))
        send_hover_result(id, FOR_DOC);
    else if (kw("if"))
        send_hover_result(id, IF_KW_DOC);
    else if (kw("int"))
        send_hover_result(id, INT_DOC);
    else
        send_null_result(id);
#undef kw
}

/* ---------- dispatch ---------- */

static int handle_message(const char *msg)
{
    char method[64];
    if (!json_get_string(msg, "method", method, sizeof(method)))
        return 1;

    char id[64] = "null";
    json_get_id(msg, id, sizeof(id));

    if (strcmp(method, "initialize") == 0) {
        send_initialize_result(id);
    } else if (strcmp(method, "textDocument/didOpen") == 0) {
        handle_did_open(msg);
    } else if (strcmp(method, "textDocument/hover") == 0) {
        handle_hover(msg, id);
    } else if (strcmp(method, "shutdown") == 0) {
        send_null_result(id);
    } else if (strcmp(method, "exit") == 0) {
        return 0;
    }
    return 1;
}

int main(void)
{
    char *msg;
    while ((msg = read_message()) != NULL) {
        int keep = handle_message(msg);
        free(msg);
        if (!keep) break;
    }
    return 0;
}
