
vim.opt.swapfile = false

vim.lsp.log.set_level("trace")
vim.lsp.log._set_filename(vim.fn.getcwd() .. "/lsp.log")

vim.lsp.config("anakins-c-ls", {
    cmd = { vim.fn.getcwd() .. "/out/anakins-c-ls" },
    filetypes = { "c" },
    root_markers = { ".git" },
})

vim.lsp.enable("anakins-c-ls")
