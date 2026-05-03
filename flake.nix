{
  description = "Anakin's C language server";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    neovim-nightly.url = "github:nix-community/neovim-nightly-overlay";
  };

  outputs = { self, nixpkgs, flake-utils, neovim-nightly }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        nvim = neovim-nightly.packages.${system}.default;
        grammars = pkgs.tree-sitter.withPlugins (p: [ p.tree-sitter-c ]);
      in
      {
        packages = {
          default = pkgs.stdenv.mkDerivation {
            pname = "anakins-c-ls";
            version = "0.2.0";
            src = ./.;

            nativeBuildInputs = [ pkgs.makeWrapper pkgs.pkg-config ];
            buildInputs = [ pkgs.tree-sitter grammars ];

            buildPhase = ''make build TREE_SITTER_GRAMMARS=${grammars}'';

            installPhase = ''
              mkdir -p $out/bin
              cp out/anakins-c-ls $out/bin/
              wrapProgram $out/bin/anakins-c-ls \
                --prefix LD_LIBRARY_PATH : ${pkgs.tree-sitter}/lib:${grammars}
            '';
          };

          tryout = pkgs.writeShellApplication {
            name = "tryout";
            runtimeInputs = [ pkgs.gnused pkgs.gnugrep nvim self.packages.${system}.default ];
            checkPhase = "";
            text = ''
              kernel_root="$(pwd)"

              c_file="$(find "$kernel_root/kernel" -name '*.c' -print -quit 2>/dev/null)"
              if [[ -z "$c_file" ]]; then
                echo "tryout: no .c files found under $kernel_root/kernel" >&2
                echo "Run this from the root of a Linux kernel source tree." >&2
                exit 1
              fi

              nvim_config=$(mktemp -d)
              cat > "$nvim_config/init.lua" << EOF
              vim.cmd("filetype plugin on")
              vim.opt.swapfile = false
              vim.lsp.config("anakins-c-ls", {
                cmd = { "anakins-c-ls" },
                filetypes = { "c" },
                root_markers = { ".git", "Makefile", "Kconfig" },
              })
              vim.lsp.enable("anakins-c-ls")
              vim.keymap.set("n", "gd", vim.lsp.buf.definition,        { desc = "Go to definition" })
              vim.keymap.set("n", "gr", vim.lsp.buf.references,        { desc = "Find references" })
              vim.keymap.set("n", "K",  vim.lsp.buf.hover,             { desc = "Hover docs" })
              vim.keymap.set("n", "<leader>r", vim.lsp.buf.rename,     { desc = "Rename symbol" })
              vim.keymap.set("n", "<leader>s", vim.lsp.buf.document_symbol, { desc = "Document symbols" })
              print("anakins-c-ls ready — gd=definition  gr=references  K=hover  <leader>r=rename")
              EOF

              exec nvim -u "$nvim_config/init.lua" "$c_file"
            '';
          };
        };

        apps.tryout = {
          type = "app";
          program = "${self.packages.${system}.tryout}/bin/tryout";
        };

        devShells.default = pkgs.mkShell {
          name = "anakins-c-ls";

          packages = with pkgs; [
            bats
            clang-tools
            cscope
            diffutils
            jq
            pkg-config
            shellcheck
            tree-sitter
            self.packages.${system}.default
          ];

          buildInputs = [ pkgs.tree-sitter grammars ];

          NIX_LDFLAGS = "-L${pkgs.tree-sitter}/lib -L${grammars}";
          LD_LIBRARY_PATH = "${pkgs.tree-sitter}/lib:${grammars}";
        };
      });
}
