
vim.opt.swapfile = false

vim.lsp.set_log_level("trace")
vim.lsp.log._set_filename(vim.fn.getcwd() .. "/lsp.log")

vim.lsp.config("anakins-c-ls", {
    cmd = { "anakins-c-ls" },
    filetypes = { "c" },
    root_markers = { ".git" },
})

vim.lsp.enable("anakins-c-ls")
