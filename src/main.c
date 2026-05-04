#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <tree_sitter/api.h>
#include <tree_sitter/tree-sitter-c.h>

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

/* implement-c.texi: Storage duration and linkage */
#define AUTO_DOC \
    "## `auto`\n" \
    "\n" \
    "`auto` is a storage-class specifier that gives a block-scope variable automatic storage duration — the variable is allocated on entry to the block and deallocated on exit. It is the default for block-scope variables and the keyword is rarely written explicitly.\n" \
    "\n" \
    "In C23, `auto` gains a second meaning as a type deduced from the initializer expression, similar to `auto` in C++."

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

#define MAX_DOCS     64
#define MAX_URI      1024
#define MAX_MSG      (1 << 20)  /* 1 MiB */
#define MAX_INCLUDES 64
#define MAX_PATH     2048

/* ---------- document store ---------- */

static TSParser *parser;
static char workspace_root[MAX_PATH];

typedef struct {
    char    uri[MAX_URI];
    char   *text;      /* heap-allocated */
    TSTree *tree;      /* syntax tree, heap-allocated */
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
        snprintf(d->uri, MAX_URI, "%s", uri);
        d->text = NULL;
        d->tree = NULL;
    }
    free(d->text);
    ts_tree_delete(d->tree);
    d->text = strdup(text);
    d->tree = ts_parser_parse_string(parser, NULL, d->text, strlen(d->text));
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

/* Spawn cscope -bRq in dir as a background process. */
static void send_initialize_result(const char *id); /* forward decl */
static void build_cscope_db(const char *dir)
{
    if (!dir || !dir[0]) return;

    /* Check if cscope is available */
    if (access("/usr/bin/cscope", X_OK) != 0 &&
        system("command -v cscope > /dev/null 2>&1") != 0)
        return;

    pid_t pid = fork();
    if (pid < 0) return;
    if (pid == 0) {
        /* Child: run cscope in the workspace root */
        if (chdir(dir) != 0) _exit(1);
        /* Redirect stdout/stderr to /dev/null */
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0) {
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }
        execl("/usr/bin/env", "env", "cscope", "-bRq", (char *)NULL);
        /* execl failed — try PATH */
        execlp("cscope", "cscope", "-bRq", (char *)NULL);
        _exit(1);
    }
    /* Parent: don't wait — let it run in background */
    (void)pid;
}

static void handle_initialize(const char *msg, const char *id)
{
    /* Extract rootUri or rootPath to discover the workspace directory */
    char root_uri[MAX_URI] = "";
    char root_path[2048] = "";

    if (json_get_string(msg, "rootUri", root_uri, sizeof(root_uri))) {
        /* Strip file:// prefix */
        const char *p = root_uri;
        if (strncmp(p, "file://", 7) == 0) p += 7;
        snprintf(workspace_root, sizeof(workspace_root), "%s", p);
    } else if (json_get_string(msg, "rootPath", root_path, sizeof(root_path))) {
        snprintf(workspace_root, sizeof(workspace_root), "%s", root_path);
    }

    /* Kick off a background cscope build if no database exists yet
     * and the root looks like a real source tree (has a Makefile or
     * Kconfig — not a test harness directory). */
    if (workspace_root[0]) {
        char check_path[MAX_PATH + 16];
        snprintf(check_path, sizeof(check_path), "%s/cscope.out", workspace_root);
        int no_db = (access(check_path, F_OK) != 0);
        snprintf(check_path, sizeof(check_path), "%s/Makefile", workspace_root);
        if (no_db && access(check_path, F_OK) == 0)
            build_cscope_db(workspace_root);
    }

    send_initialize_result(id);
}

static void send_initialize_result(const char *id)
{
    char body[1024];
    snprintf(body, sizeof(body),
             "{\"jsonrpc\":\"2.0\",\"id\":%s,"
             "\"result\":{\"capabilities\":{"
             "\"hoverProvider\":true,"
             "\"definitionProvider\":true,"
             "\"documentSymbolProvider\":true,"
             "\"callHierarchyProvider\":true,"
             "\"referencesProvider\":true,"
             "\"workspaceSymbolProvider\":true,"
             "\"renameProvider\":true,"
             "\"documentFormattingProvider\":true,"
             "\"completionProvider\":{},"
             "\"textDocumentSync\":{\"openClose\":true,\"change\":2}"
             "}}}", id);
    send_message(body);
}

/* Send a textDocument/definition response with a JSON array of Location
 * objects.  locs points to an array of nlocs Location structs encoded as a
 * JSON array string that has already been built by the caller. */
static void send_definition_result(const char *id, const char *locs_json)
{
    size_t bufsz = strlen(locs_json) + 128;
    char *body = malloc(bufsz);
    if (!body) return;
    snprintf(body, bufsz,
             "{\"jsonrpc\":\"2.0\",\"id\":%s,\"result\":%s}",
             id, locs_json);
    send_message(body);
    free(body);
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

/* ---------- definition helpers ---------- */

/* Append a Location JSON object to buf (capacity bufsz) at byte offset s in
 * src, with identifier length ident_len, for the given uri. */
static void append_location(char *buf, size_t bufsz, int *count,
                            const char *uri, const char *src, uint32_t s,
                            uint32_t ident_len)
{
    uint32_t line = 0, character = 0;
    for (uint32_t i = 0; i < s; i++) {
        if (src[i] == '\n') { line++; character = 0; }
        else                  character++;
    }
    size_t cur = strlen(buf);
    snprintf(buf + cur, bufsz - cur,
             "%s{\"uri\":\"%s\","
             "\"range\":{\"start\":{\"line\":%u,\"character\":%u},"
             "\"end\":{\"line\":%u,\"character\":%u}}}",
             *count > 0 ? "," : "",
             uri, line, character, line, character + ident_len);
    (*count)++;
}

/* Find the enclosing function_definition node for a given byte offset.
 * Returns a null node if no enclosing function_definition exists. */
static TSNode find_enclosing_function(TSNode root, uint32_t byte)
{
    TSNode node = ts_node_named_descendant_for_byte_range(root, byte, byte);
    while (!ts_node_is_null(node)) {
        if (strcmp(ts_node_type(node), "function_definition") == 0)
            return node;
        node = ts_node_parent(node);
    }
    return ts_node_child(root, -1u); /* return a null node */
}

/* Walk an AST subtree and collect definition sites for an identifier.
 *
 * ident/ident_len — the name being searched.
 * node_kind       — "identifier" or "type_identifier" or "statement_identifier".
 * For "statement_identifier" we look inside labeled_statement nodes.
 * For "identifier" we match declarator / function_declarator / preproc_def.
 * For "type_identifier" we match struct/union/enum_specifier and type_definition.
 */
static void collect_definitions(TSNode node, const char *src,
                                const char *ident, uint32_t ident_len,
                                const char *kind,
                                const char *uri,
                                char *buf, size_t bufsz, int *count)
{
    if (ts_node_is_null(node)) return;

    const char *ntype = ts_node_type(node);

    if (strcmp(kind, "statement_identifier") == 0) {
        /* goto labels: labeled_statement's first named child */
        if (strcmp(ntype, "labeled_statement") == 0) {
            TSNode id = ts_node_named_child(node, 0);
            if (!ts_node_is_null(id) &&
                strcmp(ts_node_type(id), "statement_identifier") == 0) {
                uint32_t s = ts_node_start_byte(id);
                uint32_t e = ts_node_end_byte(id);
                if (e - s == ident_len &&
                    strncmp(src + s, ident, ident_len) == 0)
                    append_location(buf, bufsz, count, uri, src, s, ident_len);
            }
        }
    } else if (strcmp(kind, "identifier") == 0) {
        /* Variable / function / macro definitions.
         * Match an "identifier" child in one of the declaration forms. */
        if (strcmp(ntype, "declaration") == 0 ||
            strcmp(ntype, "parameter_declaration") == 0) {
            /* Walk all named children looking for declarator identifiers. */
            for (uint32_t i = 0; i < ts_node_named_child_count(node); i++) {
                TSNode ch = ts_node_named_child(node, i);
                const char *ct = ts_node_type(ch);
                /* declarator / pointer_declarator / init_declarator all
                 * eventually contain an identifier. */
                if (strcmp(ct, "identifier") == 0 ||
                    strcmp(ct, "declarator") == 0 ||
                    strcmp(ct, "init_declarator") == 0 ||
                    strcmp(ct, "pointer_declarator") == 0 ||
                    strcmp(ct, "array_declarator") == 0 ||
                    strcmp(ct, "function_declarator") == 0) {
                    /* find the inner identifier */
                    TSNode inner = ts_node_named_descendant_for_byte_range(
                        ch,
                        ts_node_start_byte(ch), ts_node_start_byte(ch));
                    while (!ts_node_is_null(inner) &&
                           strcmp(ts_node_type(inner), "identifier") != 0)
                        inner = ts_node_named_child(inner, 0);
                    if (!ts_node_is_null(inner) &&
                        strcmp(ts_node_type(inner), "identifier") == 0) {
                        uint32_t s = ts_node_start_byte(inner);
                        uint32_t e = ts_node_end_byte(inner);
                        if (e - s == ident_len &&
                            strncmp(src + s, ident, ident_len) == 0)
                            append_location(buf, bufsz, count, uri, src, s, ident_len);
                    }
                }
            }
        } else if (strcmp(ntype, "function_definition") == 0) {
            /* The function name sits in its function_declarator */
            TSNode decl = ts_node_child_by_field_name(node, "declarator", 10);
            /* Unwrap pointer_declarator if present */
            while (!ts_node_is_null(decl) &&
                   strcmp(ts_node_type(decl), "function_declarator") != 0) {
                decl = ts_node_named_child(decl, 0);
            }
            if (!ts_node_is_null(decl) &&
                strcmp(ts_node_type(decl), "function_declarator") == 0) {
                TSNode name = ts_node_named_child(decl, 0);
                if (!ts_node_is_null(name) &&
                    strcmp(ts_node_type(name), "identifier") == 0) {
                    uint32_t s = ts_node_start_byte(name);
                    uint32_t e = ts_node_end_byte(name);
                    if (e - s == ident_len &&
                        strncmp(src + s, ident, ident_len) == 0)
                        append_location(buf, bufsz, count, uri, src, s, ident_len);
                }
            }
        } else if (strcmp(ntype, "preproc_def") == 0 ||
                   strcmp(ntype, "preproc_function_def") == 0) {
            TSNode name = ts_node_child_by_field_name(node, "name", 4);
            if (!ts_node_is_null(name) &&
                strcmp(ts_node_type(name), "identifier") == 0) {
                uint32_t s = ts_node_start_byte(name);
                uint32_t e = ts_node_end_byte(name);
                if (e - s == ident_len &&
                    strncmp(src + s, ident, ident_len) == 0)
                    append_location(buf, bufsz, count, uri, src, s, ident_len);
            }
        }
    } else if (strcmp(kind, "macro") == 0) {
        /* Only match #define / #define-function */
        if (strcmp(ntype, "preproc_def") == 0 ||
            strcmp(ntype, "preproc_function_def") == 0) {
            TSNode name = ts_node_child_by_field_name(node, "name", 4);
            if (!ts_node_is_null(name) &&
                strcmp(ts_node_type(name), "identifier") == 0) {
                uint32_t s = ts_node_start_byte(name);
                uint32_t e = ts_node_end_byte(name);
                if (e - s == ident_len &&
                    strncmp(src + s, ident, ident_len) == 0)
                    append_location(buf, bufsz, count, uri, src, s, ident_len);
            }
        }
    } else if (strcmp(kind, "type_identifier") == 0) {
        /* struct/union/enum definitions and typedefs */
        if ((strcmp(ntype, "struct_specifier") == 0 ||
             strcmp(ntype, "union_specifier") == 0 ||
             strcmp(ntype, "enum_specifier") == 0) &&
            /* Only match top-level definitions (has a body) */
            !ts_node_is_null(ts_node_child_by_field_name(node, "body", 4))) {
            TSNode name = ts_node_child_by_field_name(node, "name", 4);
            if (!ts_node_is_null(name) &&
                strcmp(ts_node_type(name), "type_identifier") == 0) {
                uint32_t s = ts_node_start_byte(name);
                uint32_t e = ts_node_end_byte(name);
                if (e - s == ident_len &&
                    strncmp(src + s, ident, ident_len) == 0)
                    append_location(buf, bufsz, count, uri, src, s, ident_len);
            }
        } else if (strcmp(ntype, "type_definition") == 0) {
            /* typedef: the alias name is the last declarator */
            uint32_t nc = ts_node_named_child_count(node);
            if (nc > 0) {
                TSNode last = ts_node_named_child(node, nc - 1);
                if (!ts_node_is_null(last) &&
                    strcmp(ts_node_type(last), "type_identifier") == 0) {
                    uint32_t s = ts_node_start_byte(last);
                    uint32_t e = ts_node_end_byte(last);
                    if (e - s == ident_len &&
                        strncmp(src + s, ident, ident_len) == 0)
                        append_location(buf, bufsz, count, uri, src, s, ident_len);
                }
            }
        }
    }

    /* Recurse into all named children */
    for (uint32_t i = 0; i < ts_node_named_child_count(node); i++)
        collect_definitions(ts_node_named_child(node, i), src, ident, ident_len,
                            kind, uri, buf, bufsz, count);
}

/* Walk upward from file_path until a directory with an "include" subdirectory
 * is found; write it into out (capacity outsz).  This is the workspace root
 * used for resolving angle-bracket includes. */
static void derive_workspace_root(const char *file_path, char *out, size_t outsz)
{
    char cur[MAX_PATH];
    strncpy(cur, file_path, sizeof(cur) - 1);
    cur[sizeof(cur) - 1] = '\0';
    char *sl = strrchr(cur, '/');
    if (sl) *sl = '\0';

    out[0] = '\0';
    for (int up = 0; up < 8; up++) {
        char probe[MAX_PATH + 9];
        snprintf(probe, sizeof(probe), "%s/include", cur);
        if (access(probe, F_OK) == 0) {
            strncpy(out, cur, outsz - 1);
            out[outsz - 1] = '\0';
            return;
        }
        char *s2 = strrchr(cur, '/');
        if (!s2) break;
        *s2 = '\0';
    }
}

/* Resolve a <linux/foo.h> or "foo.h" include path relative to a workspace
 * root and the directory of the file that contains the #include.
 * Writes the resolved absolute path into out (capacity outsz).
 * Returns 1 on success. */
static int resolve_include(const char *workspace_root,
                           const char *including_file_path,
                           const char *include_text,
                           char *out, size_t outsz)
{
    /* Strip angle brackets or quotes */
    const char *p = include_text;
    int is_system = (*p == '<');
    if (*p == '<' || *p == '"') p++;
    size_t plen = strlen(p);
    if (plen > 0 && (p[plen-1] == '>' || p[plen-1] == '"'))
        plen--;

    if (is_system) {
        /* Try workspace_root/include/<path> */
        snprintf(out, outsz, "%s/include/%.*s", workspace_root, (int)plen, p);
        if (access(out, R_OK) == 0) return 1;
        /* Try workspace_root/<path> */
        snprintf(out, outsz, "%s/%.*s", workspace_root, (int)plen, p);
        if (access(out, R_OK) == 0) return 1;
    } else {
        /* Quote include: relative to directory of including file */
        char dir[MAX_PATH];
        strncpy(dir, including_file_path, sizeof(dir) - 1);
        dir[sizeof(dir) - 1] = '\0';
        char *slash = strrchr(dir, '/');
        if (slash) *slash = '\0';
        else { dir[0] = '.'; dir[1] = '\0'; }
        snprintf(out, outsz, "%s/%.*s", dir, (int)plen, p);
        if (access(out, R_OK) == 0) return 1;
    }
    return 0;
}

/* Read a file from disk into a freshly malloc'd buffer.
 * Returns NULL on failure. */
static char *read_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0) { fclose(f); return NULL; }
    char *buf = malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return NULL; }
    if ((long)fread(buf, 1, (size_t)sz, f) != sz) {
        free(buf); fclose(f); return NULL;
    }
    buf[sz] = '\0';
    fclose(f);
    return buf;
}

/* Search the AST of a parsed header for definitions of ident, appending
 * Location entries to buf. */
static void search_header(const char *header_path, const char *header_src,
                          TSNode header_root,
                          const char *ident, uint32_t ident_len,
                          const char *kind,
                          char *buf, size_t bufsz, int *count)
{
    /* Build a file:// URI for the header */
    char huri[MAX_PATH + 8];
    snprintf(huri, sizeof(huri), "file://%s", header_path);
    collect_definitions(header_root, header_src, ident, ident_len,
                        kind, huri, buf, bufsz, count);
}

/* Recursively search header files reachable from file_path for definitions
 * of ident/kind, appending Location entries to buf.
 * visited/nvisited tracks already-searched paths to prevent cycles.
 * workspace_root is used for resolving <system> includes. */
static void search_includes(const char *workspace_root,
                            const char *file_path,
                            const char *src,
                            TSNode root,
                            const char *ident, uint32_t ident_len,
                            const char *kind,
                            char *buf, size_t bufsz, int *count,
                            char visited[][MAX_PATH], int *nvisited)
{
    uint32_t nchildren = ts_node_named_child_count(root);
    for (uint32_t i = 0; i < nchildren && *count == 0; i++) {
        TSNode ch = ts_node_named_child(root, i);
        if (strcmp(ts_node_type(ch), "preproc_include") != 0) continue;

        TSNode path_node = ts_node_named_child(ch, 0);
        if (ts_node_is_null(path_node)) continue;

        uint32_t ps = ts_node_start_byte(path_node);
        uint32_t pe = ts_node_end_byte(path_node);
        char include_text[MAX_PATH];
        uint32_t plen = pe - ps < sizeof(include_text) - 1
                        ? pe - ps : sizeof(include_text) - 1;
        strncpy(include_text, src + ps, plen);
        include_text[plen] = '\0';

        char header_path[MAX_PATH];
        if (!resolve_include(workspace_root, file_path,
                             include_text, header_path, sizeof(header_path)))
            continue;

        int already = 0;
        for (int v = 0; v < *nvisited; v++)
            if (strcmp(visited[v], header_path) == 0) { already = 1; break; }
        if (already) continue;
        if (*nvisited < MAX_INCLUDES)
            snprintf(visited[(*nvisited)++], MAX_PATH, "%s", header_path);

        char *hsrc = read_file(header_path);
        if (!hsrc) continue;

        TSTree *htree = ts_parser_parse_string(parser, NULL, hsrc, strlen(hsrc));
        if (htree) {
            TSNode hroot = ts_tree_root_node(htree);
            search_header(header_path, hsrc, hroot,
                          ident, ident_len, kind, buf, bufsz, count);
            if (*count == 0)
                search_includes(workspace_root, header_path, hsrc, hroot,
                                ident, ident_len, kind, buf, bufsz, count,
                                visited, nvisited);
            ts_tree_delete(htree);
        }
        free(hsrc);
    }
}

static void handle_definition(const char *msg, const char *id)
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
    if (!d || !d->text || !d->tree) {
        send_null_result(id);
        return;
    }

    /* Convert (line, character) to a byte offset. */
    uint32_t byte = 0;
    const char *p = d->text;
    for (int ln = 0; ln < line; ln++) {
        const char *nl = strchr(p, '\n');
        if (!nl) { send_null_result(id); return; }
        byte += (uint32_t)(nl - p) + 1;
        p = nl + 1;
    }
    byte += (uint32_t)character;

    TSNode root = ts_tree_root_node(d->tree);

    /* Find the smallest named node at the cursor. */
    TSNode node = ts_node_named_descendant_for_byte_range(root, byte, byte);
    if (ts_node_is_null(node)) {
        send_null_result(id);
        return;
    }

    const char *node_kind = ts_node_type(node);
    TSNode parent = ts_node_parent(node);

    /* Extract the identifier text at the cursor. */
    uint32_t    tok_start = ts_node_start_byte(node);
    uint32_t    tok_end   = ts_node_end_byte(node);
    const char *ident     = d->text + tok_start;
    uint32_t    ident_len = tok_end - tok_start;

    /* Determine the search kind:
     *   statement_identifier in a goto_statement → look for labeled_statement
     *   identifier                               → functions, variables, macros
     *   type_identifier                          → struct/union/enum/typedef
     * Anything else → null. */
    const char *kind = NULL;

    if (strcmp(node_kind, "statement_identifier") == 0 &&
        !ts_node_is_null(parent) &&
        strcmp(ts_node_type(parent), "goto_statement") == 0) {
        kind = "statement_identifier";
    } else if (strcmp(node_kind, "identifier") == 0) {
        kind = "identifier";
    } else if (strcmp(node_kind, "type_identifier") == 0) {
        kind = "type_identifier";
    }

    if (!kind) {
        send_null_result(id);
        return;
    }

    /* Result buffer for JSON Location array entries. */
    size_t bufsz = 16384;
    char *locs = malloc(bufsz);
    if (!locs) { send_null_result(id); return; }
    locs[0] = '\0';
    int count = 0;

    if (strcmp(kind, "statement_identifier") == 0) {
        /* goto labels are function-scoped: search only within the enclosing
         * function_definition. */
        TSNode fn = find_enclosing_function(root, byte);
        if (ts_node_is_null(fn)) {
            free(locs);
            send_null_result(id);
            return;
        }
        collect_definitions(fn, d->text, ident, ident_len,
                            kind, uri, locs, bufsz, &count);
    } else {
        /* For identifiers: search the enclosing function first (catches local
         * variables).  Only if nothing is found there do we fall through to a
         * full-file search (catches function definitions and macros) and then
         * to included headers.
         * For type_identifiers there is no meaningful "enclosing function"
         * scope, so skip straight to the full-file search. */
        if (strcmp(kind, "identifier") == 0) {
            TSNode fn = find_enclosing_function(root, byte);
            if (!ts_node_is_null(fn))
                collect_definitions(fn, d->text, ident, ident_len,
                                    kind, uri, locs, bufsz, &count);
        }

        if (count == 0)
            collect_definitions(root, d->text, ident, ident_len,
                                kind, uri, locs, bufsz, &count);

        /* Extract the file path from the URI (strip "file://") */
        const char *file_path = uri;
        if (strncmp(file_path, "file://", 7) == 0)
            file_path += 7;

        char workspace_root[MAX_PATH];
        derive_workspace_root(file_path, workspace_root, sizeof(workspace_root));

        char visited[MAX_INCLUDES][MAX_PATH];
        int nvisited = 0;
        search_includes(workspace_root, file_path, d->text, root,
                        ident, ident_len, kind, locs, bufsz, &count,
                        visited, &nvisited);
    }

    if (count == 0) {
        free(locs);
        send_null_result(id);
        return;
    }

    /* Build the final JSON array. */
    size_t arrlen = strlen(locs) + 4;
    char *arr = malloc(arrlen);
    if (!arr) { free(locs); send_null_result(id); return; }
    snprintf(arr, arrlen, "[%s]", locs);
    free(locs);

    send_definition_result(id, arr);
    free(arr);
}

/* Convert a byte offset in src to {line, character} LSP position. */
static void byte_to_line_col(const char *src, uint32_t byte,
                              uint32_t *line_out, uint32_t *col_out)
{
    uint32_t line = 0, col = 0;
    for (uint32_t i = 0; i < byte; i++) {
        if (src[i] == '\n') { line++; col = 0; }
        else                   col++;
    }
    *line_out = line;
    *col_out  = col;
}

/* ---------- textDocument/documentSymbol ---------- */

static void collect_symbols(TSNode node, const char *src, const char *uri,
                             char **buf, size_t *bufsz, size_t *len, int *count)
{
    if (ts_node_is_null(node)) return;

    const char *ntype = ts_node_type(node);

    const char *sym_name = NULL;
    int sym_kind = 0;
    uint32_t name_start = 0, name_end = 0;

    if (strcmp(ntype, "function_definition") == 0) {
        TSNode decl = ts_node_child_by_field_name(node, "declarator", 10);
        while (!ts_node_is_null(decl) &&
               strcmp(ts_node_type(decl), "function_declarator") != 0) {
            if (ts_node_named_child_count(decl) == 0) { decl = ts_node_child(node, -1u); break; }
            decl = ts_node_named_child(decl, 0);
        }
        if (!ts_node_is_null(decl) &&
            strcmp(ts_node_type(decl), "function_declarator") == 0) {
            TSNode name_node = ts_node_named_child(decl, 0);
            if (!ts_node_is_null(name_node)) {
                name_start = ts_node_start_byte(name_node);
                name_end   = ts_node_end_byte(name_node);
                sym_name   = src + name_start;
                sym_kind   = 12;
            }
        }
    } else if (strcmp(ntype, "struct_specifier") == 0 ||
               strcmp(ntype, "union_specifier") == 0) {
        TSNode name_node = ts_node_child_by_field_name(node, "name", 4);
        if (!ts_node_is_null(name_node)) {
            name_start = ts_node_start_byte(name_node);
            name_end   = ts_node_end_byte(name_node);
            sym_name   = src + name_start;
            sym_kind   = 23;
        }
    } else if (strcmp(ntype, "enum_specifier") == 0) {
        TSNode name_node = ts_node_child_by_field_name(node, "name", 4);
        if (!ts_node_is_null(name_node)) {
            name_start = ts_node_start_byte(name_node);
            name_end   = ts_node_end_byte(name_node);
            sym_name   = src + name_start;
            sym_kind   = 10;
        }
    } else if (strcmp(ntype, "type_definition") == 0) {
        uint32_t nc = ts_node_named_child_count(node);
        if (nc > 0) {
            TSNode last = ts_node_named_child(node, nc - 1);
            if (!ts_node_is_null(last)) {
                name_start = ts_node_start_byte(last);
                name_end   = ts_node_end_byte(last);
                sym_name   = src + name_start;
                sym_kind   = 26;
            }
        }
    } else if (strcmp(ntype, "preproc_def") == 0 ||
               strcmp(ntype, "preproc_function_def") == 0) {
        TSNode name_node = ts_node_child_by_field_name(node, "name", 4);
        if (!ts_node_is_null(name_node)) {
            name_start = ts_node_start_byte(name_node);
            name_end   = ts_node_end_byte(name_node);
            sym_name   = src + name_start;
            sym_kind   = 14;
        }
    } else if (strcmp(ntype, "declaration") == 0) {
        TSNode par = ts_node_parent(node);
        if (!ts_node_is_null(par) &&
            strcmp(ts_node_type(par), "translation_unit") == 0) {
            TSNode inner = ts_node_named_descendant_for_byte_range(
                node, ts_node_start_byte(node), ts_node_start_byte(node));
            while (!ts_node_is_null(inner) &&
                   strcmp(ts_node_type(inner), "identifier") != 0)
                inner = ts_node_named_child(inner, 0);
            if (!ts_node_is_null(inner) &&
                strcmp(ts_node_type(inner), "identifier") == 0) {
                name_start = ts_node_start_byte(inner);
                name_end   = ts_node_end_byte(inner);
                sym_name   = src + name_start;
                sym_kind   = 13;
            }
        }
    }

    if (sym_name && sym_kind > 0) {
        uint32_t n_sl, n_sc, n_el, n_ec;
        byte_to_line_col(src, ts_node_start_byte(node), &n_sl, &n_sc);
        byte_to_line_col(src, ts_node_end_byte(node),   &n_el, &n_ec);
        uint32_t name_len = name_end - name_start;

        size_t needed = name_len + 512;
        if (*len + needed >= *bufsz) {
            *bufsz = (*bufsz) * 2 + needed;
            *buf = realloc(*buf, *bufsz);
            if (!*buf) return;
        }

        char *escaped = malloc(name_len * 2 + 4);
        if (!escaped) return;
        size_t ei = 0;
        for (uint32_t k = 0; k < name_len; k++) {
            char c = sym_name[k];
            if (c == '"')  { escaped[ei++] = '\\'; escaped[ei++] = '"'; }
            else if (c == '\\') { escaped[ei++] = '\\'; escaped[ei++] = '\\'; }
            else escaped[ei++] = c;
        }
        escaped[ei] = '\0';

        int written = snprintf(*buf + *len, *bufsz - *len,
                 "%s{\"name\":\"%s\",\"kind\":%d,"
                 "\"location\":{\"uri\":\"%s\","
                 "\"range\":{\"start\":{\"line\":%u,\"character\":%u},"
                 "\"end\":{\"line\":%u,\"character\":%u}}}}",
                 *count > 0 ? "," : "",
                 escaped, sym_kind, uri,
                 n_sl, n_sc, n_el, n_ec);
        free(escaped);
        if (written > 0) *len += (size_t)written;
        (*count)++;
    }

    for (uint32_t i = 0; i < ts_node_named_child_count(node); i++)
        collect_symbols(ts_node_named_child(node, i), src, uri,
                        buf, bufsz, len, count);
}

static void handle_document_symbol(const char *msg, const char *id)
{
    char uri[MAX_URI];
    if (!json_get_string(msg, "uri", uri, sizeof(uri))) {
        send_null_result(id);
        return;
    }
    Doc *d = doc_find(uri);
    if (!d || !d->text || !d->tree) {
        send_null_result(id);
        return;
    }

    TSNode root = ts_tree_root_node(d->tree);

    size_t bufsz = 4096;
    char *syms = malloc(bufsz);
    if (!syms) { send_null_result(id); return; }
    syms[0] = '\0';
    size_t len = 0;
    int count = 0;

    collect_symbols(root, d->text, uri, &syms, &bufsz, &len, &count);

    size_t arrlen = len + 8;
    char *arr = malloc(arrlen);
    if (!arr) { free(syms); send_null_result(id); return; }
    snprintf(arr, arrlen, "[%s]", syms);
    free(syms);

    size_t bodysz = arrlen + 128;
    char *body = malloc(bodysz);
    if (!body) { free(arr); send_null_result(id); return; }
    snprintf(body, bodysz,
             "{\"jsonrpc\":\"2.0\",\"id\":%s,\"result\":%s}",
             id, arr);
    free(arr);
    send_message(body);
    free(body);
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
    if (!d || !d->text || !d->tree) {
        send_null_result(id);
        return;
    }

    /* Convert (line, character) to a byte offset into the document text. */
    uint32_t byte = 0;
    const char *p = d->text;
    for (int ln = 0; ln < line; ln++) {
        const char *nl = strchr(p, '\n');
        if (!nl) { send_null_result(id); return; }
        byte += (uint32_t)(nl - p) + 1;
        p = nl + 1;
    }
    byte += (uint32_t)character;

    /* Find the smallest named node covering that byte offset. */
    TSNode node = ts_node_named_descendant_for_byte_range(
        ts_tree_root_node(d->tree), byte, byte);

    if (ts_node_is_null(node)) {
        send_null_result(id);
        return;
    }

    /* Extract the token text for nodes whose type is shared by multiple
     * keywords (e.g. storage_class_specifier covers static/extern/etc.). */
    const char *node_type = ts_node_type(node);
    uint32_t    tok_start = ts_node_start_byte(node);
    uint32_t    tok_end   = ts_node_end_byte(node);
    const char *tok       = d->text + tok_start;
    uint32_t    tok_len   = tok_end - tok_start;

    /* Match node type first, then token text where needed. */
#define tok_is(s) (tok_len == sizeof(s)-1 && strncmp(tok, s, sizeof(s)-1) == 0)
#define type_is(s) (strcmp(node_type, s) == 0)

    if (type_is("storage_class_specifier")) {
        if      (tok_is("static"))   send_hover_result(id, STATIC_DOC);
        else if (tok_is("extern"))   send_hover_result(id, EXTERN_DOC);
        else if (tok_is("register")) send_hover_result(id, REGISTER_DOC);
        else if (tok_is("auto"))     send_hover_result(id, AUTO_DOC);
        else if (tok_is("inline"))   send_hover_result(id, INLINE_DOC);
        else                         send_null_result(id);
    } else if (type_is("type_qualifier")) {
        if      (tok_is("const"))    send_hover_result(id, CONST_DOC);
        else if (tok_is("volatile")) send_hover_result(id, VOLATILE_DOC);
        else if (tok_is("restrict")) send_hover_result(id, RESTRICT_DOC);
        else                         send_null_result(id);
    } else if (type_is("primitive_type")) {
        if      (tok_is("int"))    send_hover_result(id, INT_DOC);
        else if (tok_is("char"))   send_hover_result(id, CHAR_DOC);
        else if (tok_is("float"))  send_hover_result(id, FLOAT_DOC);
        else if (tok_is("double")) send_hover_result(id, DOUBLE_DOC);
        else if (tok_is("void"))   send_hover_result(id, VOID_DOC);
        else                       send_null_result(id);
    } else if (type_is("sized_type_specifier")) {
        /* The named node spans the whole specifier (e.g. "unsigned int").
         * Get the unnamed leaf directly under the cursor to find the keyword. */
        TSNode leaf = ts_node_descendant_for_byte_range(
            ts_tree_root_node(d->tree), byte, byte);
        const char *ltok = d->text + ts_node_start_byte(leaf);
        uint32_t    llen = ts_node_end_byte(leaf) - ts_node_start_byte(leaf);
#define ltok_is(s) (llen == sizeof(s)-1 && strncmp(ltok, s, sizeof(s)-1) == 0)
        if      (ltok_is("short"))    send_hover_result(id, SHORT_DOC);
        else if (ltok_is("long"))     send_hover_result(id, LONG_DOC);
        else if (ltok_is("signed"))   send_hover_result(id, SIGNED_DOC);
        else if (ltok_is("unsigned")) send_hover_result(id, UNSIGNED_DOC);
        else                          send_null_result(id);
#undef ltok_is
    } else if (type_is("type_definition")) {
        /* typedef keyword sits inside a type_definition node */
        send_hover_result(id, TYPEDEF_DOC);
    } else if (type_is("struct_specifier")) {
        send_hover_result(id, STRUCT_DOC);
    } else if (type_is("union_specifier")) {
        send_hover_result(id, UNION_DOC);
    } else if (type_is("enum_specifier")) {
        send_hover_result(id, ENUM_DOC);
    } else if (type_is("sizeof_expression")) {
        send_hover_result(id, SIZEOF_DOC);
    } else if (type_is("return_statement")) {
        send_hover_result(id, RETURN_DOC);
    } else if (type_is("if_statement")) {
        send_hover_result(id, IF_KW_DOC);
    } else if (type_is("else_clause")) {
        send_hover_result(id, ELSE_KW_DOC);
    } else if (type_is("for_statement")) {
        send_hover_result(id, FOR_DOC);
    } else if (type_is("while_statement")) {
        send_hover_result(id, WHILE_DOC);
    } else if (type_is("do_statement")) {
        send_hover_result(id, DO_DOC);
    } else if (type_is("switch_statement")) {
        send_hover_result(id, SWITCH_DOC);
    } else if (type_is("case_statement")) {
        /* The named node spans the whole case body; use the leaf token
         * to distinguish "case" from "default". */
        TSNode leaf = ts_node_descendant_for_byte_range(
            ts_tree_root_node(d->tree), byte, byte);
        const char *ltok = d->text + ts_node_start_byte(leaf);
        uint32_t    llen = ts_node_end_byte(leaf) - ts_node_start_byte(leaf);
#define ltok_is(s) (llen == sizeof(s)-1 && strncmp(ltok, s, sizeof(s)-1) == 0)
        if (ltok_is("default")) send_hover_result(id, DEFAULT_DOC);
        else                    send_hover_result(id, CASE_DOC);
#undef ltok_is
    } else if (type_is("break_statement")) {
        send_hover_result(id, BREAK_DOC);
    } else if (type_is("continue_statement")) {
        send_hover_result(id, CONTINUE_DOC);
    } else if (type_is("goto_statement")) {
        send_hover_result(id, GOTO_DOC);
    } else if (type_is("preproc_include")) {
        send_hover_result(id, INCLUDE_DOC);
    } else if (type_is("preproc_def") || type_is("preproc_function_def")) {
        send_hover_result(id, DEFINE_DOC);
    } else if (type_is("preproc_ifdef")) {
        /* The named node spans the whole conditional block; use the leaf
         * token to distinguish #ifdef / #ifndef / #endif. */
        TSNode leaf = ts_node_descendant_for_byte_range(
            ts_tree_root_node(d->tree), byte, byte);
        const char *ltok = d->text + ts_node_start_byte(leaf);
        uint32_t    llen = ts_node_end_byte(leaf) - ts_node_start_byte(leaf);
#define ltok_is(s) (llen == sizeof(s)-1 && strncmp(ltok, s, sizeof(s)-1) == 0)
        if      (ltok_is("#ifndef")) send_hover_result(id, IFNDEF_DOC);
        else if (ltok_is("#endif"))  send_hover_result(id, ENDIF_DOC);
        else                         send_hover_result(id, IFDEF_DOC);
#undef ltok_is
    } else if (type_is("preproc_if")) {
        send_hover_result(id, IF_DOC);
    } else if (type_is("preproc_elif")) {
        send_hover_result(id, ELIF_DOC);
    } else if (type_is("preproc_else")) {
        send_hover_result(id, ELSE_DOC);
    } else if (type_is("preproc_directive")) {
        if      (tok_is("#undef"))   send_hover_result(id, UNDEF_DOC);
        else if (tok_is("#error"))   send_hover_result(id, ERROR_DOC);
        else if (tok_is("#warning")) send_hover_result(id, WARNING_DOC);
        else if (tok_is("#pragma"))  send_hover_result(id, PRAGMA_DOC);
        else if (tok_is("#endif"))   send_hover_result(id, ENDIF_DOC);
        else                         send_null_result(id);
    } else if (type_is("type_identifier")) {
        /* User-defined type: find the definition and show the token name
         * as a code-block hover. */
        char hover_buf[4096];
        hover_buf[0] = '\0';
        int hcount = 0;
        size_t hbufsz = sizeof(hover_buf);

        collect_definitions(ts_tree_root_node(d->tree), d->text,
                            tok, tok_len, "type_identifier", uri,
                            hover_buf, hbufsz, &hcount);

        if (hcount == 0) {
            const char *file_path = uri;
            if (strncmp(file_path, "file://", 7) == 0) file_path += 7;
            char workspace_root[MAX_PATH];
            derive_workspace_root(file_path, workspace_root, sizeof(workspace_root));
            char visited[MAX_INCLUDES][MAX_PATH];
            int nvisited = 0;
            search_includes(workspace_root, file_path, d->text,
                            ts_tree_root_node(d->tree),
                            tok, tok_len, "type_identifier",
                            hover_buf, hbufsz, &hcount,
                            visited, &nvisited);
        }

        if (hcount == 0)
            send_null_result(id);
        else {
            char md[512];
            snprintf(md, sizeof(md), "```c\n%.*s\n```", (int)tok_len, tok);
            send_hover_result(id, md);
        }
    } else if (type_is("identifier")) {
        /* Macro call: only provide hover if a macro definition is found. */
        char hover_buf[4096];
        hover_buf[0] = '\0';
        int hcount = 0;
        size_t hbufsz = sizeof(hover_buf);

        collect_definitions(ts_tree_root_node(d->tree), d->text,
                            tok, tok_len, "macro", uri,
                            hover_buf, hbufsz, &hcount);

        if (hcount == 0) {
            const char *file_path = uri;
            if (strncmp(file_path, "file://", 7) == 0) file_path += 7;
            char workspace_root[MAX_PATH];
            derive_workspace_root(file_path, workspace_root, sizeof(workspace_root));
            char visited[MAX_INCLUDES][MAX_PATH];
            int nvisited = 0;
            search_includes(workspace_root, file_path, d->text,
                            ts_tree_root_node(d->tree),
                            tok, tok_len, "macro",
                            hover_buf, hbufsz, &hcount,
                            visited, &nvisited);
        }

        if (hcount == 0)
            send_null_result(id);
        else {
            char md[512];
            snprintf(md, sizeof(md), "```c\n%.*s\n```", (int)tok_len, tok);
            send_hover_result(id, md);
        }
    } else {
        send_null_result(id);
    }

#undef tok_is
#undef type_is
}

/* ---------- dispatch ---------- */

/* --- callHierarchy helpers --- */

/* Build a JSON CallHierarchyItem into buf (caller ensures space).
 * fn_node: the function_definition node.
 * name_node: the identifier node for the function name.
 * Returns the number of bytes written (not including NUL). */
static int build_call_hierarchy_item(char *buf, size_t bufsz,
                                      const char *src, const char *uri,
                                      TSNode fn_node, TSNode name_node)
{
    uint32_t fn_sl, fn_sc, fn_el, fn_ec;
    uint32_t nm_sl, nm_sc, nm_el, nm_ec;
    byte_to_line_col(src, ts_node_start_byte(fn_node),   &fn_sl, &fn_sc);
    byte_to_line_col(src, ts_node_end_byte(fn_node),     &fn_el, &fn_ec);
    byte_to_line_col(src, ts_node_start_byte(name_node), &nm_sl, &nm_sc);
    byte_to_line_col(src, ts_node_end_byte(name_node),   &nm_el, &nm_ec);

    uint32_t nlen = ts_node_end_byte(name_node) - ts_node_start_byte(name_node);
    char name[256];
    if (nlen >= sizeof(name)) nlen = sizeof(name) - 1;
    strncpy(name, src + ts_node_start_byte(name_node), nlen);
    name[nlen] = '\0';

    return snprintf(buf, bufsz,
        "{\"name\":\"%s\",\"kind\":12,"
        "\"uri\":\"%s\","
        "\"range\":{\"start\":{\"line\":%u,\"character\":%u},"
                    "\"end\":{\"line\":%u,\"character\":%u}},"
        "\"selectionRange\":{\"start\":{\"line\":%u,\"character\":%u},"
                             "\"end\":{\"line\":%u,\"character\":%u}},"
        "\"data\":{\"uri\":\"%s\",\"name\":\"%s\"}}",
        name, uri,
        fn_sl, fn_sc, fn_el, fn_ec,
        nm_sl, nm_sc, nm_el, nm_ec,
        uri, name);
}

/* Find the function_definition whose declarator identifier matches `name`.
 * Returns a null node if not found. */
static TSNode find_function_def(TSNode root, const char *src,
                                 const char *name, uint32_t nlen)
{
    TSNode null_node;
    memset(&null_node, 0, sizeof(null_node));
    if (ts_node_is_null(root)) return null_node;
    uint32_t nc = ts_node_child_count(root);
    for (uint32_t i = 0; i < nc; i++) {
        TSNode ch = ts_node_child(root, i);
        if (strcmp(ts_node_type(ch), "function_definition") != 0) continue;
        TSNode decl = ts_node_child_by_field_name(ch, "declarator", 10);
        while (!ts_node_is_null(decl) &&
               strcmp(ts_node_type(decl), "function_declarator") != 0)
            decl = ts_node_child_by_field_name(decl, "declarator", 10);
        if (ts_node_is_null(decl)) continue;
        TSNode ident = ts_node_child_by_field_name(decl, "declarator", 10);
        if (ts_node_is_null(ident)) continue;
        uint32_t ilen = ts_node_end_byte(ident) - ts_node_start_byte(ident);
        if (ilen == nlen && strncmp(src + ts_node_start_byte(ident), name, nlen) == 0)
            return ch;
    }
    TSNode null_node2;
    memset(&null_node2, 0, sizeof(null_node2));
    return null_node2;
}

/* --- callHierarchy/prepare --- */

static void handle_call_hierarchy_prepare(const char *msg, const char *id)
{
    char uri[MAX_URI];
    if (!json_get_string(msg, "uri", uri, sizeof(uri))) {
        send_null_result(id);
        return;
    }
    int line = json_get_int(msg, "line");
    int character = json_get_int(msg, "character");
    if (line < 0 || character < 0) { send_null_result(id); return; }

    Doc *d = doc_find(uri);
    if (!d || !d->text || !d->tree) { send_null_result(id); return; }

    uint32_t byte = 0;
    const char *p = d->text;
    for (int ln = 0; ln < line; ln++) {
        const char *nl = strchr(p, '\n');
        if (!nl) { send_null_result(id); return; }
        byte += (uint32_t)(nl - p) + 1;
        p = nl + 1;
    }
    byte += (uint32_t)character;

    TSNode root = ts_tree_root_node(d->tree);
    TSNode node = ts_node_named_descendant_for_byte_range(root, byte, byte);
    if (ts_node_is_null(node)) { send_null_result(id); return; }

    /* Must be an identifier that is the name of a function_definition */
    if (strcmp(ts_node_type(node), "identifier") != 0) {
        send_null_result(id);
        return;
    }
    TSNode parent = ts_node_parent(node);
    /* Walk up through pointer/parameter declarators to function_declarator */
    while (!ts_node_is_null(parent) &&
           strcmp(ts_node_type(parent), "function_declarator") != 0)
        parent = ts_node_parent(parent);
    if (ts_node_is_null(parent)) { send_null_result(id); return; }
    TSNode fn = ts_node_parent(parent);
    if (ts_node_is_null(fn) ||
        strcmp(ts_node_type(fn), "function_definition") != 0) {
        send_null_result(id);
        return;
    }

    char item[2048];
    build_call_hierarchy_item(item, sizeof(item), d->text, uri, fn, node);

    size_t blen = strlen(item) + 64;
    char *body = malloc(blen);
    if (!body) { send_null_result(id); return; }
    snprintf(body, blen,
             "{\"jsonrpc\":\"2.0\",\"id\":%s,\"result\":[%s]}", id, item);
    send_message(body);
    free(body);
}

/* --- callHierarchy/incomingCalls --- */

/* Walk tree collecting call_expression nodes that call `name`.
 * Appends entries to *buf, updating *len and *count. */
static void collect_incoming_calls(TSNode node, const char *src,
                                    const char *call_name, uint32_t call_nlen,
                                    const char *uri,
                                    char **buf, size_t *bufsz,
                                    size_t *len, int *count)
{
    if (ts_node_is_null(node)) return;
    if (strcmp(ts_node_type(node), "call_expression") == 0) {
        TSNode fn_child = ts_node_child_by_field_name(node, "function", 8);
        if (!ts_node_is_null(fn_child)) {
            uint32_t flen = ts_node_end_byte(fn_child) - ts_node_start_byte(fn_child);
            if (flen == call_nlen &&
                strncmp(src + ts_node_start_byte(fn_child), call_name, call_nlen) == 0) {
                /* Found a call site. Find enclosing function. */
                /* Find enclosing function by walking up the parent chain */
                TSNode cur = ts_node_parent(fn_child);
                TSNode enclosing;
                memset(&enclosing, 0, sizeof(enclosing));
                while (!ts_node_is_null(cur)) {
                    if (strcmp(ts_node_type(cur), "function_definition") == 0) {
                        enclosing = cur;
                        break;
                    }
                    cur = ts_node_parent(cur);
                }
                uint32_t call_sl, call_sc, call_el, call_ec;
                byte_to_line_col(src, ts_node_start_byte(fn_child), &call_sl, &call_sc);
                byte_to_line_col(src, ts_node_end_byte(fn_child),   &call_el, &call_ec);

                char from_item[2048];
                if (!ts_node_is_null(enclosing)) {
                    /* Get name of enclosing function */
                    TSNode edecl = ts_node_child_by_field_name(enclosing, "declarator", 10);
                    while (!ts_node_is_null(edecl) &&
                           strcmp(ts_node_type(edecl), "function_declarator") != 0)
                        edecl = ts_node_child_by_field_name(edecl, "declarator", 10);
                    TSNode eident = ts_node_is_null(edecl) ? enclosing :
                        ts_node_child_by_field_name(edecl, "declarator", 10);
                    build_call_hierarchy_item(from_item, sizeof(from_item),
                                              src, uri, enclosing,
                                              ts_node_is_null(eident) ? enclosing : eident);
                } else {
                    snprintf(from_item, sizeof(from_item),
                        "{\"name\":\"<file>\",\"kind\":1,"
                        "\"uri\":\"%s\","
                        "\"range\":{\"start\":{\"line\":0,\"character\":0},"
                                   "\"end\":{\"line\":0,\"character\":0}},"
                        "\"selectionRange\":{\"start\":{\"line\":0,\"character\":0},"
                                           "\"end\":{\"line\":0,\"character\":0}},"
                        "\"data\":{\"uri\":\"%s\",\"name\":\"<file>\"}}",
                        uri, uri);
                }

                char entry[4096];
                int elen = snprintf(entry, sizeof(entry),
                    "{\"from\":%s,"
                    "\"fromRanges\":[{\"start\":{\"line\":%u,\"character\":%u},"
                                    "\"end\":{\"line\":%u,\"character\":%u}}]}",
                    from_item, call_sl, call_sc, call_el, call_ec);

                if (*len + (size_t)elen + 4 > *bufsz) {
                    *bufsz *= 2;
                    *buf = realloc(*buf, *bufsz);
                    if (!*buf) return;
                }
                if (*count > 0) { (*buf)[(*len)++] = ','; }
                memcpy(*buf + *len, entry, elen);
                *len += elen;
                (*buf)[*len] = '\0';
                (*count)++;
            }
        }
    }
    uint32_t nc = ts_node_named_child_count(node);
    for (uint32_t i = 0; i < nc; i++)
        collect_incoming_calls(ts_node_named_child(node, i), src,
                               call_name, call_nlen, uri, buf, bufsz, len, count);
}

static void handle_incoming_calls(const char *msg, const char *id)
{
    char name[256];
    if (!json_get_string(msg, "name", name, sizeof(name))) {
        send_null_result(id);
        return;
    }
    uint32_t nlen = (uint32_t)strlen(name);

    size_t bufsz = 16384;
    char *buf = malloc(bufsz);
    if (!buf) { send_null_result(id); return; }
    buf[0] = '\0';
    size_t len = 0;
    int count = 0;

    for (int i = 0; i < MAX_DOCS; i++) {
        if (!docs[i].uri[0] || !docs[i].text || !docs[i].tree) continue;
        TSNode root = ts_tree_root_node(docs[i].tree);
        collect_incoming_calls(root, docs[i].text, name, nlen, docs[i].uri,
                               &buf, &bufsz, &len, &count);
    }

    size_t arrlen = len + 4;
    char *arr = malloc(arrlen);
    if (!arr) { free(buf); send_null_result(id); return; }
    snprintf(arr, arrlen, "[%s]", buf);
    free(buf);

    size_t blen = strlen(arr) + 64;
    char *body = malloc(blen);
    if (!body) { free(arr); send_null_result(id); return; }
    snprintf(body, blen,
             "{\"jsonrpc\":\"2.0\",\"id\":%s,\"result\":%s}", id, arr);
    free(arr);
    send_message(body);
    free(body);
}

/* --- callHierarchy/outgoingCalls --- */

static void collect_outgoing_calls(TSNode node, TSNode doc_root,
                                    const char *src,
                                    const char *uri,
                                    char **buf, size_t *bufsz,
                                    size_t *len, int *count)
{
    if (ts_node_is_null(node)) return;
    if (strcmp(ts_node_type(node), "call_expression") == 0) {
        TSNode fn_child = ts_node_child_by_field_name(node, "function", 8);
        if (!ts_node_is_null(fn_child) &&
            strcmp(ts_node_type(fn_child), "identifier") == 0) {
            uint32_t flen = ts_node_end_byte(fn_child) - ts_node_start_byte(fn_child);
            char callee_name[256];
            if (flen >= sizeof(callee_name)) flen = sizeof(callee_name) - 1;
            strncpy(callee_name, src + ts_node_start_byte(fn_child), flen);
            callee_name[flen] = '\0';

            uint32_t call_sl, call_sc, call_el, call_ec;
            byte_to_line_col(src, ts_node_start_byte(fn_child), &call_sl, &call_sc);
            byte_to_line_col(src, ts_node_end_byte(fn_child),   &call_el, &call_ec);

            /* Try to find the callee's function_definition in the same doc */
            char to_item[2048];
            TSNode callee_fn = find_function_def(doc_root, src, callee_name, (uint32_t)strlen(callee_name));
            if (!ts_node_is_null(callee_fn)) {
                TSNode cdecl = ts_node_child_by_field_name(callee_fn, "declarator", 10);
                while (!ts_node_is_null(cdecl) &&
                       strcmp(ts_node_type(cdecl), "function_declarator") != 0)
                    cdecl = ts_node_child_by_field_name(cdecl, "declarator", 10);
                TSNode cident = ts_node_is_null(cdecl) ? callee_fn :
                    ts_node_child_by_field_name(cdecl, "declarator", 10);
                build_call_hierarchy_item(to_item, sizeof(to_item),
                                          src, uri, callee_fn,
                                          ts_node_is_null(cident) ? callee_fn : cident);
            } else {
                snprintf(to_item, sizeof(to_item),
                    "{\"name\":\"%s\",\"kind\":12,"
                    "\"uri\":\"%s\","
                    "\"range\":{\"start\":{\"line\":%u,\"character\":%u},"
                               "\"end\":{\"line\":%u,\"character\":%u}},"
                    "\"selectionRange\":{\"start\":{\"line\":%u,\"character\":%u},"
                                       "\"end\":{\"line\":%u,\"character\":%u}},"
                    "\"data\":{\"uri\":\"%s\",\"name\":\"%s\"}}",
                    callee_name, uri,
                    call_sl, call_sc, call_el, call_ec,
                    call_sl, call_sc, call_el, call_ec,
                    uri, callee_name);
            }

            char entry[4096];
            int elen = snprintf(entry, sizeof(entry),
                "{\"to\":%s,"
                "\"fromRanges\":[{\"start\":{\"line\":%u,\"character\":%u},"
                                "\"end\":{\"line\":%u,\"character\":%u}}]}",
                to_item, call_sl, call_sc, call_el, call_ec);

            if (*len + (size_t)elen + 4 > *bufsz) {
                *bufsz *= 2;
                *buf = realloc(*buf, *bufsz);
                if (!*buf) return;
            }
            if (*count > 0) { (*buf)[(*len)++] = ','; }
            memcpy(*buf + *len, entry, elen);
            *len += elen;
            (*buf)[*len] = '\0';
            (*count)++;
        }
    }
    uint32_t nc = ts_node_named_child_count(node);
    for (uint32_t i = 0; i < nc; i++)
        collect_outgoing_calls(ts_node_named_child(node, i), doc_root, src, uri,
                               buf, bufsz, len, count);
}

static void handle_outgoing_calls(const char *msg, const char *id)
{
    char name[256];
    char uri[MAX_URI];
    if (!json_get_string(msg, "name", name, sizeof(name)) ||
        !json_get_string(msg, "uri", uri, sizeof(uri))) {
        send_null_result(id);
        return;
    }
    uint32_t nlen = (uint32_t)strlen(name);

    Doc *d = doc_find(uri);
    if (!d || !d->text || !d->tree) { send_null_result(id); return; }

    TSNode root = ts_tree_root_node(d->tree);
    TSNode fn = find_function_def(root, d->text, name, nlen);
    if (ts_node_is_null(fn)) {
        /* Return empty array */
        char body[128];
        snprintf(body, sizeof(body),
                 "{\"jsonrpc\":\"2.0\",\"id\":%s,\"result\":[]}", id);
        send_message(body);
        return;
    }

    size_t bufsz = 16384;
    char *buf = malloc(bufsz);
    if (!buf) { send_null_result(id); return; }
    buf[0] = '\0';
    size_t len = 0;
    int count = 0;

    collect_outgoing_calls(fn, root, d->text, uri, &buf, &bufsz, &len, &count);

    size_t arrlen = len + 4;
    char *arr = malloc(arrlen);
    if (!arr) { free(buf); send_null_result(id); return; }
    snprintf(arr, arrlen, "[%s]", buf);
    free(buf);

    size_t blen = strlen(arr) + 64;
    char *body = malloc(blen);
    if (!body) { free(arr); send_null_result(id); return; }
    snprintf(body, blen,
             "{\"jsonrpc\":\"2.0\",\"id\":%s,\"result\":%s}", id, arr);
    free(arr);
    send_message(body);
    free(body);
}

/* ---------- textDocument/references (via cscope) ---------- */

/* Walk upward from file_path looking for cscope.out */
static void find_cscope_db(const char *file_path,
                            char *db_dir, size_t db_dir_sz)
{
    char dir[MAX_PATH];
    snprintf(dir, MAX_PATH, "%s", file_path);
    char *slash = strrchr(dir, '/');
    if (slash) *slash = '\0';
    else       { strncpy(db_dir, ".", db_dir_sz); return; }

    char candidate[MAX_PATH + 16];
    char best[MAX_PATH];
    best[0] = '\0';
    char cur[MAX_PATH];
    snprintf(cur, MAX_PATH, "%s", dir);
    for (;;) {
        snprintf(candidate, sizeof(candidate), "%s/cscope.out", cur);
        if (access(candidate, F_OK) == 0)
            snprintf(best, MAX_PATH, "%s", cur);
        char *up = strrchr(cur, '/');
        if (!up || up == cur) break;
        *up = '\0';
    }
    if (best[0]) strncpy(db_dir, best, db_dir_sz);
    else         strncpy(db_dir, dir,  db_dir_sz);
}

static void handle_references(const char *msg, const char *id)
{
    char uri[MAX_URI];
    if (!json_get_string(msg, "uri", uri, sizeof(uri))) {
        send_null_result(id); return;
    }
    int line      = json_get_int(msg, "line");
    int character = json_get_int(msg, "character");
    if (line < 0 || character < 0) { send_null_result(id); return; }

    Doc *d = doc_find(uri);
    if (!d || !d->text || !d->tree) { send_null_result(id); return; }

    uint32_t byte = 0;
    const char *p = d->text;
    for (int ln = 0; ln < line; ln++) {
        const char *nl = strchr(p, '\n');
        if (!nl) { send_null_result(id); return; }
        byte += (uint32_t)(nl - p) + 1;
        p = nl + 1;
    }
    byte += (uint32_t)character;

    TSNode root = ts_tree_root_node(d->tree);
    TSNode node = ts_node_named_descendant_for_byte_range(root, byte, byte);
    if (ts_node_is_null(node)) { send_null_result(id); return; }

    const char *ntype = ts_node_type(node);
    if (strcmp(ntype, "identifier")       != 0 &&
        strcmp(ntype, "type_identifier")  != 0 &&
        strcmp(ntype, "field_identifier") != 0) {
        send_null_result(id); return;
    }

    uint32_t tok_start = ts_node_start_byte(node);
    uint32_t tok_end   = ts_node_end_byte(node);
    uint32_t tok_len   = tok_end - tok_start;
    char ident[256];
    if (tok_len >= sizeof(ident)) tok_len = sizeof(ident) - 1;
    strncpy(ident, d->text + tok_start, tok_len);
    ident[tok_len] = '\0';

    const char *file_path = uri;
    if (strncmp(file_path, "file://", 7) == 0) file_path += 7;

    char db_dir[MAX_PATH];
    find_cscope_db(file_path, db_dir, sizeof(db_dir));

    char cscope_out[MAX_PATH];
    snprintf(cscope_out, sizeof(cscope_out), "%s/cscope.out", db_dir);
    if (access(cscope_out, F_OK) != 0) {
        char body[128];
        snprintf(body, sizeof(body),
                 "{\"jsonrpc\":\"2.0\",\"id\":%s,\"result\":[]}", id);
        send_message(body);
        return;
    }

    /* cscope query type 3 = find all references */
    char cmd[MAX_PATH * 2 + 256];
    snprintf(cmd, sizeof(cmd),
             "cscope -dL -f '%s/cscope.out' -3 '%s' 2>/dev/null",
             db_dir, ident);

    FILE *fp = popen(cmd, "r");
    if (!fp) {
        char body[128];
        snprintf(body, sizeof(body),
                 "{\"jsonrpc\":\"2.0\",\"id\":%s,\"result\":[]}", id);
        send_message(body);
        return;
    }

    size_t bufsz = 16384;
    char *locs = malloc(bufsz);
    if (!locs) { pclose(fp); send_null_result(id); return; }
    locs[0] = '\0';
    size_t len = 0;
    int count = 0;

    char line_buf[4096];
    while (fgets(line_buf, sizeof(line_buf), fp)) {
        char ref_file[MAX_PATH], ref_func[256], ref_text[1024];
        int  ref_line = 0;
        if (sscanf(line_buf, "%s %s %d %[^\n]",
                   ref_file, ref_func, &ref_line, ref_text) < 3)
            continue;

        char ref_uri[MAX_PATH + 8];
        if (ref_file[0] == '/')
            snprintf(ref_uri, sizeof(ref_uri), "file://%s", ref_file);
        else
            snprintf(ref_uri, sizeof(ref_uri), "file://%s/%s", db_dir, ref_file);

        int lsp_line = ref_line > 0 ? ref_line - 1 : 0;

        char entry[MAX_PATH + 8 + 256];
        int elen = snprintf(entry, sizeof(entry),
            "{\"uri\":\"%s\","
            "\"range\":{\"start\":{\"line\":%d,\"character\":0},"
                       "\"end\":{\"line\":%d,\"character\":0}}}",
            ref_uri, lsp_line, lsp_line);

        if (len + (size_t)elen + 4 > bufsz) {
            bufsz *= 2;
            locs = realloc(locs, bufsz);
            if (!locs) { pclose(fp); return; }
        }
        if (count > 0) { locs[len++] = ','; }
        memcpy(locs + len, entry, elen);
        len += elen;
        locs[len] = '\0';
        count++;
    }
    pclose(fp);

    size_t arrlen = len + 4;
    char *arr = malloc(arrlen);
    if (!arr) { free(locs); send_null_result(id); return; }
    snprintf(arr, arrlen, "[%s]", locs);
    free(locs);

    size_t blen = strlen(arr) + 64;
    char *body = malloc(blen);
    if (!body) { free(arr); send_null_result(id); return; }
    snprintf(body, blen,
             "{\"jsonrpc\":\"2.0\",\"id\":%s,\"result\":%s}", id, arr);
    free(arr);
    send_message(body);
    free(body);
}

/* ---------- workspace/symbol ---------- */

static void handle_workspace_symbol(const char *msg, const char *id)
{
    char query[256] = "";
    json_get_string(msg, "query", query, sizeof(query));

    /* Lowercase query for case-insensitive match */
    char lquery[256];
    for (int i = 0; query[i]; i++)
        lquery[i] = (char)tolower((unsigned char)query[i]);
    lquery[strlen(query)] = '\0';

    size_t bufsz = 32768;
    char *buf = malloc(bufsz);
    if (!buf) { send_null_result(id); return; }
    buf[0] = '\0';
    size_t len = 0;
    int count = 0;

    for (int i = 0; i < MAX_DOCS; i++) {
        if (!docs[i].uri[0] || !docs[i].text || !docs[i].tree) continue;
        TSNode root = ts_tree_root_node(docs[i].tree);
        /* Reuse collect_symbols but filter here */
        size_t sym_bufsz = 16384;
        char *sym_buf = malloc(sym_bufsz);
        if (!sym_buf) continue;
        sym_buf[0] = '\0';
        size_t sym_len = 0;
        int sym_count = 0;
        collect_symbols(root, docs[i].text, docs[i].uri,
                        &sym_buf, &sym_bufsz, &sym_len, &sym_count);

        /* sym_buf is a comma-separated list of SymbolInformation JSON objects.
         * We need to filter by name. Parse each object naively. */
        char *p = sym_buf;
        while (*p) {
            /* Find start of object */
            if (*p != '{') { p++; continue; }
            /* Find matching end brace (no nesting beyond one level in our output) */
            char *start = p;
            int depth = 0;
            while (*p) {
                if (*p == '{') depth++;
                else if (*p == '}') { depth--; if (depth == 0) { p++; break; } }
                p++;
            }
            size_t obj_len = (size_t)(p - start);
            /* Skip comma */
            if (*p == ',') p++;

            /* Extract name from this object */
            char obj[4096];
            if (obj_len >= sizeof(obj)) obj_len = sizeof(obj) - 1;
            strncpy(obj, start, obj_len);
            obj[obj_len] = '\0';

            char sym_name[256] = "";
            json_get_string(obj, "name", sym_name, sizeof(sym_name));

            /* Filter: if query non-empty, name must contain query (case-insensitive) */
            int match = 1;
            if (lquery[0]) {
                char lname[256];
                for (int j = 0; sym_name[j]; j++)
                    lname[j] = (char)tolower((unsigned char)sym_name[j]);
                lname[strlen(sym_name)] = '\0';
                match = (strstr(lname, lquery) != NULL);
            }

            if (match) {
                if (len + obj_len + 4 > bufsz) {
                    bufsz *= 2;
                    buf = realloc(buf, bufsz);
                    if (!buf) { free(sym_buf); return; }
                }
                if (count > 0) { buf[len++] = ','; }
                memcpy(buf + len, obj, obj_len);
                len += obj_len;
                buf[len] = '\0';
                count++;
            }
        }
        free(sym_buf);
    }

    size_t arrlen = len + 4;
    char *arr = malloc(arrlen);
    if (!arr) { free(buf); send_null_result(id); return; }
    snprintf(arr, arrlen, "[%s]", buf);
    free(buf);

    size_t blen = strlen(arr) + 64;
    char *body = malloc(blen);
    if (!body) { free(arr); send_null_result(id); return; }
    snprintf(body, blen,
             "{\"jsonrpc\":\"2.0\",\"id\":%s,\"result\":%s}", id, arr);
    free(arr);
    send_message(body);
    free(body);
}

/* ---------- textDocument/rename ---------- */

static void handle_rename(const char *msg, const char *id)
{
    char uri[MAX_URI];
    if (!json_get_string(msg, "uri", uri, sizeof(uri))) {
        send_null_result(id); return;
    }
    int line      = json_get_int(msg, "line");
    int character = json_get_int(msg, "character");
    char new_name[256] = "";
    if (!json_get_string(msg, "newName", new_name, sizeof(new_name)) ||
        line < 0 || character < 0) {
        send_null_result(id); return;
    }

    Doc *d = doc_find(uri);
    if (!d || !d->text || !d->tree) { send_null_result(id); return; }

    /* Extract identifier at cursor */
    uint32_t byte = 0;
    const char *p = d->text;
    for (int ln = 0; ln < line; ln++) {
        const char *nl = strchr(p, '\n');
        if (!nl) { send_null_result(id); return; }
        byte += (uint32_t)(nl - p) + 1;
        p = nl + 1;
    }
    byte += (uint32_t)character;

    TSNode root = ts_tree_root_node(d->tree);
    TSNode node = ts_node_named_descendant_for_byte_range(root, byte, byte);
    if (ts_node_is_null(node)) { send_null_result(id); return; }

    const char *ntype = ts_node_type(node);
    if (strcmp(ntype, "identifier")       != 0 &&
        strcmp(ntype, "type_identifier")  != 0 &&
        strcmp(ntype, "field_identifier") != 0) {
        send_null_result(id); return;
    }

    uint32_t tok_start = ts_node_start_byte(node);
    uint32_t tok_end   = ts_node_end_byte(node);
    uint32_t tok_len   = tok_end - tok_start;
    char ident[256];
    if (tok_len >= sizeof(ident)) tok_len = sizeof(ident) - 1;
    strncpy(ident, d->text + tok_start, tok_len);
    ident[tok_len] = '\0';

    const char *file_path = uri;
    if (strncmp(file_path, "file://", 7) == 0) file_path += 7;

    char db_dir[MAX_PATH];
    find_cscope_db(file_path, db_dir, sizeof(db_dir));

    /* Build list of locations from cscope, then group by URI for WorkspaceEdit */
    char cmd[MAX_PATH * 2 + 256];
    snprintf(cmd, sizeof(cmd),
             "cscope -dL -f '%s/cscope.out' -3 '%s' 2>/dev/null",
             db_dir, ident);

    /* We'll build the WorkspaceEdit changes object.
     * Format: {"uri1":[edits...],"uri2":[edits...]} */
    size_t changes_bufsz = 65536;
    char *changes = malloc(changes_bufsz);
    if (!changes) { send_null_result(id); return; }
    changes[0] = '\0';
    size_t changes_len = 0;
    int changes_count = 0;

    /* Accumulate edits per file using a simple array */
    typedef struct { char uri[MAX_PATH + 8]; int lines[512]; int count; } FileEdits;
    FileEdits *files = calloc(256, sizeof(FileEdits));
    if (!files) { free(changes); send_null_result(id); return; }
    int nfiles = 0;

    char cscope_out_path[MAX_PATH];
    snprintf(cscope_out_path, sizeof(cscope_out_path), "%s/cscope.out", db_dir);
    if (access(cscope_out_path, F_OK) == 0) {
        FILE *fp = popen(cmd, "r");
        if (fp) {
            char line_buf[4096];
            while (fgets(line_buf, sizeof(line_buf), fp)) {
                char ref_file[MAX_PATH], ref_func[256], ref_text[1024];
                int  ref_line = 0;
                if (sscanf(line_buf, "%s %s %d %[^\n]",
                           ref_file, ref_func, &ref_line, ref_text) < 3)
                    continue;
                char ref_uri[MAX_PATH + 8];
                if (ref_file[0] == '/')
                    snprintf(ref_uri, sizeof(ref_uri), "file://%s", ref_file);
                else
                    snprintf(ref_uri, sizeof(ref_uri), "file://%s/%s", db_dir, ref_file);

                /* Find or create file entry */
                int fi = -1;
                for (int k = 0; k < nfiles; k++)
                    if (strcmp(files[k].uri, ref_uri) == 0) { fi = k; break; }
                if (fi < 0 && nfiles < 256) {
                    fi = nfiles++;
                    snprintf(files[fi].uri, MAX_PATH + 8, "%s", ref_uri);
                }
                if (fi >= 0 && files[fi].count < 512)
                    files[fi].lines[files[fi].count++] = ref_line - 1;
            }
            pclose(fp);
        }
    }

    /* Build changes JSON: for each file, for each line, create a TextEdit.
     * We use character 0 to end-of-line as range and rely on the new_name
     * being a simple identifier replacement. Actually we need to find the
     * exact column. Read each referenced file line and find the identifier. */
    size_t new_len __attribute__((unused)) = strlen(new_name);
    size_t id_len  = strlen(ident);

    for (int fi = 0; fi < nfiles; fi++) {
        /* Read the file */
        const char *fpath = files[fi].uri;
        if (strncmp(fpath, "file://", 7) == 0) fpath += 7;
        char *fsrc = read_file(fpath);

        char edits_buf[65536];
        size_t edits_len = 0;
        int edits_count = 0;

        for (int ei = 0; ei < files[fi].count; ei++) {
            int lsp_line = files[fi].lines[ei];
            /* Find the identifier on this line */
            int col = 0;
            if (fsrc) {
                const char *lp = fsrc;
                for (int ln = 0; ln < lsp_line && *lp; ln++) {
                    const char *nl = strchr(lp, '\n');
                    if (!nl) { lp = NULL; break; }
                    lp = nl + 1;
                }
                if (lp) {
                    const char *found = strstr(lp, ident);
                    if (found) col = (int)(found - lp);
                }
            }

            char edit[1024];
            int elen = snprintf(edit, sizeof(edit),
                "{\"range\":{\"start\":{\"line\":%d,\"character\":%d},"
                            "\"end\":{\"line\":%d,\"character\":%d}},"
                "\"newText\":\"%s\"}",
                lsp_line, col,
                lsp_line, col + (int)id_len,
                new_name);

            if (edits_len + (size_t)elen + 4 < sizeof(edits_buf)) {
                if (edits_count > 0) edits_buf[edits_len++] = ',';
                memcpy(edits_buf + edits_len, edit, elen);
                edits_len += elen;
                edits_buf[edits_len] = '\0';
                edits_count++;
            }
        }
        free(fsrc);

        /* Append to changes */
        char entry[MAX_URI + 70000];
        int elen2 = snprintf(entry, sizeof(entry),
            "\"%s\":[%s]", files[fi].uri, edits_buf);
        if (changes_len + (size_t)elen2 + 4 > changes_bufsz) {
            changes_bufsz *= 2;
            changes = realloc(changes, changes_bufsz);
            if (!changes) { free(files); return; }
        }
        if (changes_count > 0) { changes[changes_len++] = ','; }
        memcpy(changes + changes_len, entry, elen2);
        changes_len += elen2;
        changes[changes_len] = '\0';
        changes_count++;
    }
    free(files);

    size_t blen = changes_len + 128;
    char *body = malloc(blen);
    if (!body) { free(changes); send_null_result(id); return; }
    snprintf(body, blen,
             "{\"jsonrpc\":\"2.0\",\"id\":%s,"
             "\"result\":{\"changes\":{%s}}}",
             id, changes);
    free(changes);
    send_message(body);
    free(body);
}

/* ---------- textDocument/formatting ---------- */

static void handle_formatting(const char *msg, const char *id)
{
    char uri[MAX_URI];
    if (!json_get_string(msg, "uri", uri, sizeof(uri))) {
        send_null_result(id); return;
    }

    Doc *d = doc_find(uri);
    if (!d || !d->text) { send_null_result(id); return; }

    const char *file_path = uri;
    if (strncmp(file_path, "file://", 7) == 0) file_path += 7;

    /* Get just the filename for --assume-filename */
    const char *fname = strrchr(file_path, '/');
    fname = fname ? fname + 1 : file_path;

    /* Pipe text through clang-format */
    char cmd[MAX_PATH + 64];
    snprintf(cmd, sizeof(cmd),
             "clang-format --assume-filename='%s' 2>/dev/null", fname);

    FILE *fp = popen(cmd, "r+");
    if (!fp) {
        /* clang-format not available — return empty array */
        char body[128];
        snprintf(body, sizeof(body),
                 "{\"jsonrpc\":\"2.0\",\"id\":%s,\"result\":[]}", id);
        send_message(body);
        return;
    }
    /* We can't do r+ with popen — use a temp file approach */
    pclose(fp);

    /* Write doc text to a temp file, run clang-format on it */
    char tmpfile[64];
    snprintf(tmpfile, sizeof(tmpfile), "/tmp/anakins-c-ls-fmt-XXXXXX");
    int tmpfd = mkstemp(tmpfile);
    if (tmpfd < 0) {
        char body[128];
        snprintf(body, sizeof(body),
                 "{\"jsonrpc\":\"2.0\",\"id\":%s,\"result\":[]}", id);
        send_message(body);
        return;
    }
    size_t src_len = strlen(d->text);
    if (write(tmpfd, d->text, src_len) < 0) { /* best-effort */ }
    close(tmpfd);

    snprintf(cmd, sizeof(cmd),
             "clang-format --assume-filename='%s' '%s' 2>/dev/null",
             fname, tmpfile);
    fp = popen(cmd, "r");
    unlink(tmpfile);
    if (!fp) {
        char body[128];
        snprintf(body, sizeof(body),
                 "{\"jsonrpc\":\"2.0\",\"id\":%s,\"result\":[]}", id);
        send_message(body);
        return;
    }

    /* Read formatted output */
    size_t out_bufsz = src_len + 65536;
    char *out = malloc(out_bufsz);
    if (!out) { pclose(fp); send_null_result(id); return; }
    size_t out_len = 0;
    char fbuf[4096];
    size_t n;
    while ((n = fread(fbuf, 1, sizeof(fbuf), fp)) > 0) {
        if (out_len + n + 1 > out_bufsz) {
            out_bufsz *= 2;
            out = realloc(out, out_bufsz);
            if (!out) { pclose(fp); return; }
        }
        memcpy(out + out_len, fbuf, n);
        out_len += n;
    }
    out[out_len] = '\0';
    pclose(fp);

    if (out_len == 0) {
        free(out);
        char body[128];
        snprintf(body, sizeof(body),
                 "{\"jsonrpc\":\"2.0\",\"id\":%s,\"result\":[]}", id);
        send_message(body);
        return;
    }

    /* Count lines in original to build the end position */
    uint32_t end_line = 0;
    uint32_t end_char = 0;
    for (size_t i = 0; i < src_len; i++) {
        if (d->text[i] == '\n') { end_line++; end_char = 0; }
        else end_char++;
    }

    /* JSON-escape the formatted output */
    size_t esc_bufsz = out_len * 2 + 64;
    char *esc = malloc(esc_bufsz);
    if (!esc) { free(out); send_null_result(id); return; }
    size_t esc_len = 0;
    for (size_t i = 0; i < out_len && esc_len + 4 < esc_bufsz; i++) {
        unsigned char ch = (unsigned char)out[i];
        if (ch == '"')       { esc[esc_len++] = '\\'; esc[esc_len++] = '"'; }
        else if (ch == '\\') { esc[esc_len++] = '\\'; esc[esc_len++] = '\\'; }
        else if (ch == '\n') { esc[esc_len++] = '\\'; esc[esc_len++] = 'n'; }
        else if (ch == '\r') { esc[esc_len++] = '\\'; esc[esc_len++] = 'r'; }
        else if (ch == '\t') { esc[esc_len++] = '\\'; esc[esc_len++] = 't'; }
        else                  { esc[esc_len++] = (char)ch; }
    }
    esc[esc_len] = '\0';
    free(out);

    size_t blen = esc_len + 256;
    char *body = malloc(blen);
    if (!body) { free(esc); send_null_result(id); return; }
    snprintf(body, blen,
        "{\"jsonrpc\":\"2.0\",\"id\":%s,\"result\":[{"
        "\"range\":{\"start\":{\"line\":0,\"character\":0},"
                   "\"end\":{\"line\":%u,\"character\":%u}},"
        "\"newText\":\"%s\""
        "}]}",
        id, end_line, end_char, esc);
    free(esc);
    send_message(body);
    free(body);
}

/* ---------- textDocument/completion ---------- */

typedef struct {
    char  name[256];
    int   kind;  /* LSP CompletionItemKind */
} CompletionItem;

static int collect_completions(TSNode node, const char *src,
                                const char *prefix, size_t prefix_len,
                                CompletionItem *items, int max_items, int *count)
{
    if (ts_node_is_null(node) || *count >= max_items) return *count;
    const char *ntype = ts_node_type(node);

    int kind = 0;
    TSNode name_node;
    memset(&name_node, 0, sizeof(name_node));

    if (strcmp(ntype, "function_definition") == 0) {
        kind = 3; /* Function */
        TSNode decl = ts_node_child_by_field_name(node, "declarator", 10);
        while (!ts_node_is_null(decl) &&
               strcmp(ts_node_type(decl), "function_declarator") != 0)
            decl = ts_node_child_by_field_name(decl, "declarator", 10);
        if (!ts_node_is_null(decl))
            name_node = ts_node_child_by_field_name(decl, "declarator", 10);
    } else if (strcmp(ntype, "declaration") == 0) {
        kind = 6; /* Variable */
        /* Look for the declarator identifier */
        TSNode decl = ts_node_child_by_field_name(node, "declarator", 10);
        while (!ts_node_is_null(decl) &&
               strcmp(ts_node_type(decl), "identifier") != 0) {
            TSNode inner = ts_node_child_by_field_name(decl, "declarator", 10);
            if (ts_node_is_null(inner)) break;
            decl = inner;
        }
        if (!ts_node_is_null(decl) &&
            strcmp(ts_node_type(decl), "identifier") == 0)
            name_node = decl;
    } else if (strcmp(ntype, "preproc_def") == 0 ||
               strcmp(ntype, "preproc_function_def") == 0) {
        kind = 21; /* Constant */
        name_node = ts_node_child_by_field_name(node, "name", 4);
    } else if (strcmp(ntype, "type_definition") == 0) {
        kind = 25; /* TypeParameter */
        /* Find the last declarator which holds the alias name */
        uint32_t nc = ts_node_named_child_count(node);
        if (nc > 0) name_node = ts_node_named_child(node, nc - 1);
    }

    if (kind && !ts_node_is_null(name_node) &&
        (strcmp(ts_node_type(name_node), "identifier") == 0 ||
         strcmp(ts_node_type(name_node), "type_identifier") == 0)) {
        uint32_t ns = ts_node_start_byte(name_node);
        uint32_t ne = ts_node_end_byte(name_node);
        uint32_t nlen = ne - ns;
        if (nlen >= prefix_len &&
            strncmp(src + ns, prefix, prefix_len) == 0) {
            /* Dedup: skip if already in items */
            int dup = 0;
            char cname[256];
            if (nlen >= sizeof(cname)) nlen = sizeof(cname) - 1;
            strncpy(cname, src + ns, nlen);
            cname[nlen] = '\0';
            for (int k = 0; k < *count; k++)
                if (strcmp(items[k].name, cname) == 0) { dup = 1; break; }
            if (!dup && *count < max_items) {
                snprintf(items[*count].name, 256, "%s", cname);
                items[*count].kind = kind;
                (*count)++;
            }
        }
    }

    uint32_t nc = ts_node_named_child_count(node);
    for (uint32_t i = 0; i < nc && *count < max_items; i++)
        collect_completions(ts_node_named_child(node, i), src,
                            prefix, prefix_len, items, max_items, count);
    return *count;
}

static void handle_completion(const char *msg, const char *id)
{
    char uri[MAX_URI];
    if (!json_get_string(msg, "uri", uri, sizeof(uri))) {
        send_null_result(id); return;
    }
    int line      = json_get_int(msg, "line");
    int character = json_get_int(msg, "character");
    if (line < 0 || character < 0) { send_null_result(id); return; }

    Doc *d = doc_find(uri);
    if (!d || !d->text || !d->tree) { send_null_result(id); return; }

    /* Find partial identifier: walk back from cursor */
    uint32_t byte = 0;
    const char *p = d->text;
    for (int ln = 0; ln < line; ln++) {
        const char *nl = strchr(p, '\n');
        if (!nl) { send_null_result(id); return; }
        byte += (uint32_t)(nl - p) + 1;
        p = nl + 1;
    }
    byte += (uint32_t)character;

    uint32_t prefix_start = byte;
    while (prefix_start > 0) {
        unsigned char ch = (unsigned char)d->text[prefix_start - 1];
        if (!isalnum(ch) && ch != '_') break;
        prefix_start--;
    }
    uint32_t prefix_len = byte - prefix_start;
    const char *prefix  = d->text + prefix_start;

    CompletionItem *items = calloc(512, sizeof(CompletionItem));
    if (!items) { send_null_result(id); return; }
    int count = 0;

    TSNode root = ts_tree_root_node(d->tree);
    collect_completions(root, d->text, prefix, prefix_len, items, 512, &count);

    /* Build JSON CompletionList */
    size_t bufsz = count * 512 + 256;
    char *buf = malloc(bufsz);
    if (!buf) { free(items); send_null_result(id); return; }
    size_t len = 0;
    buf[0] = '\0';

    for (int i = 0; i < count; i++) {
        char entry[512];
        int elen = snprintf(entry, sizeof(entry),
            "{\"label\":\"%s\",\"kind\":%d}",
            items[i].name, items[i].kind);
        if (len + (size_t)elen + 4 > bufsz) {
            bufsz *= 2;
            buf = realloc(buf, bufsz);
            if (!buf) { free(items); return; }
        }
        if (i > 0) { buf[len++] = ','; }
        memcpy(buf + len, entry, elen);
        len += elen;
        buf[len] = '\0';
    }
    free(items);

    size_t blen = len + 256;
    char *body = malloc(blen);
    if (!body) { free(buf); send_null_result(id); return; }
    snprintf(body, blen,
             "{\"jsonrpc\":\"2.0\",\"id\":%s,"
             "\"result\":{\"isIncomplete\":false,\"items\":[%s]}}",
             id, buf);
    free(buf);
    send_message(body);
    free(body);
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
        handle_initialize(msg, id);
    } else if (strcmp(method, "textDocument/didOpen") == 0) {
        handle_did_open(msg);
    } else if (strcmp(method, "textDocument/hover") == 0) {
        handle_hover(msg, id);
    } else if (strcmp(method, "textDocument/definition") == 0) {
        handle_definition(msg, id);
    } else if (strcmp(method, "textDocument/documentSymbol") == 0) {
        handle_document_symbol(msg, id);
    } else if (strcmp(method, "textDocument/prepareCallHierarchy") == 0) {
        handle_call_hierarchy_prepare(msg, id);
    } else if (strcmp(method, "callHierarchy/incomingCalls") == 0) {
        handle_incoming_calls(msg, id);
    } else if (strcmp(method, "callHierarchy/outgoingCalls") == 0) {
        handle_outgoing_calls(msg, id);
    } else if (strcmp(method, "textDocument/references") == 0) {
        handle_references(msg, id);
    } else if (strcmp(method, "workspace/symbol") == 0) {
        handle_workspace_symbol(msg, id);
    } else if (strcmp(method, "textDocument/rename") == 0) {
        handle_rename(msg, id);
    } else if (strcmp(method, "textDocument/formatting") == 0) {
        handle_formatting(msg, id);
    } else if (strcmp(method, "textDocument/completion") == 0) {
        handle_completion(msg, id);
    } else if (strcmp(method, "shutdown") == 0) {
        send_null_result(id);
    } else if (strcmp(method, "exit") == 0) {
        return 0;
    }
    return 1;
}

int main(void)
{
    parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_c());

    char *msg;
    while ((msg = read_message()) != NULL) {
        int keep = handle_message(msg);
        free(msg);
        if (!keep) break;
    }

    ts_parser_delete(parser);
    return 0;
}
