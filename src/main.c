#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Documentation for the #include directive, sourced from
 * tests/gcc/gcc/doc/cpp.texi (Include Syntax section). */
#define INCLUDE_DOC \
    "## `#include`\\n" \
    "\\n" \
    "Both user and system header files are included using the preprocessing directive `#include`. It has two variants:\\n" \
    "\\n" \
    "- `#include <file>` \\u2014 Used for system header files. Searches for a file named *file* in a standard list of system directories. You can prepend directories to this list with the `-I` option.\\n" \
    "\\n" \
    "- `#include \"file\"` \\u2014 Used for header files of your own program. Searches for a file named *file* first in the directory containing the current file, then in the quote directories and then the same directories used for `<file>`. You can prepend directories to the list of quote directories with the `-iquote` option.\\n" \
    "\\n" \
    "The argument of `#include`, whether delimited with quote marks or angle brackets, behaves like a string constant in that comments are not recognized, and macro names are not expanded.\\n" \
    "\\n" \
    "It is an error if there is anything (other than comments) on the line after the file name."

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
             "\"result\":{\"capabilities\":{\"hoverProvider\":true}}}", id);
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

    /* Skip leading whitespace then check for #include */
    while (*p == ' ' || *p == '\t') p++;
    if (strncmp(p, "#include", 8) != 0) {
        send_null_result(id);
        return;
    }

    send_hover_result(id, INCLUDE_DOC);
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
