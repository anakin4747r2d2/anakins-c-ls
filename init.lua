
vim.lsp.config("anakins-c-ls", {
    cmd = { vim.fn.getcwd() .. "/anakins-c-ls" },
    filetypes = { "c" },
    root_markers = { ".git" },
})

vim.lsp.enable("anakins-c-ls")
