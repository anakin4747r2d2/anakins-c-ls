# anakins-c-ls

A lightweight C language server for Neovim, built with tree-sitter and cscope.

## Try it out

Run from inside a Linux kernel source tree:

```sh
nix run github:anakin4747r2d2/anakins-c-ls#tryout
```

### Keybindings

| Key | Action |
|-----|--------|
| `gd` | Go to definition |
| `gr` | Find all references |
| `K` | Hover documentation |
| `<leader>r` | Rename symbol |
| `<leader>s` | List document symbols |

## Features

- Hover documentation for all C keywords and preprocessor directives
- Go-to-definition (local scope → file scope → included headers)
- Document symbols (functions, structs, macros, typedefs)
- Call hierarchy (incoming and outgoing calls)
- Find all references (via cscope)
- Rename symbol (via cscope)
- Workspace symbol search
- Document formatting (via clang-format)
- Completion (identifier prefix matching)

## Development

```sh
# Run tests
make all

# Run tests without cqfd/docker
make test
```
