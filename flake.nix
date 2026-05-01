{
  description = "Anakin's C language server";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        grammars = pkgs.tree-sitter.withPlugins (p: [ p.tree-sitter-c ]);
      in
      {
        packages = {
          default = pkgs.stdenv.mkDerivation {
            pname = "anakins-c-ls";
            version = "0.2.0";
            src = ./.;

            nativeBuildInputs = [ pkgs.makeWrapper ];
            buildInputs = [ pkgs.tree-sitter grammars ];

            buildPhase = ''
              mkdir -p out
              cc -Wall -Wextra \
                -Isrc \
                -I${pkgs.tree-sitter}/include \
                -o out/anakins-c-ls src/main.c \
                -L${pkgs.tree-sitter}/lib \
                -L${grammars} \
                -ltree-sitter \
                -l:c.so
            '';

            installPhase = ''
              mkdir -p $out/bin
              cp out/anakins-c-ls $out/bin/
              wrapProgram $out/bin/anakins-c-ls \
                --prefix LD_LIBRARY_PATH : ${pkgs.tree-sitter}/lib:${grammars}
            '';
          };

          tryout = pkgs.writeShellScriptBin "anakins-c-ls-tryout" ''
            set -e
            if [ -z "$1" ]; then
              echo "Usage: nix run github:anakin4747r2d2/anakins-c-ls#tryout -- /path/to/linux"
              echo ""
              echo "Opens neovim with anakins-c-ls configured, rooted at the given Linux source tree."
              echo "Run 'make cscope' in the Linux tree first for textDocument/references support."
              exit 1
            fi
            LINUX_DIR="$(realpath "$1")"
            if [ ! -d "$LINUX_DIR" ]; then
              echo "Error: '$LINUX_DIR' is not a directory" >&2
              exit 1
            fi
            INIT_LUA="$(mktemp --suffix=.lua)"
            trap 'rm -f "$INIT_LUA"' EXIT
            cat > "$INIT_LUA" << 'EOF'
            vim.opt.swapfile = false
            vim.lsp.config("anakins-c-ls", {
              cmd = { "${pkgs.lib.getExe self.packages.${system}.default}" },
              filetypes = { "c" },
              root_markers = { ".git", "Makefile", "Kconfig" },
            })
            vim.lsp.enable("anakins-c-ls")
            vim.keymap.set("n", "gd", vim.lsp.buf.definition, { desc = "Go to definition" })
            vim.keymap.set("n", "gr", vim.lsp.buf.references, { desc = "Find references" })
            vim.keymap.set("n", "K",  vim.lsp.buf.hover,      { desc = "Hover docs" })
            vim.keymap.set("n", "<leader>r", vim.lsp.buf.rename,     { desc = "Rename symbol" })
            vim.keymap.set("n", "<leader>s", vim.lsp.buf.document_symbol, { desc = "Document symbols" })
            print("anakins-c-ls ready — gd=definition  gr=references  K=hover  <leader>r=rename")
            EOF
            cd "$LINUX_DIR"
            exec ${pkgs.lib.getExe pkgs.neovim} -u "$INIT_LUA" "$@"
          '';
        };

        apps.tryout = {
          type = "app";
          program = "${self.packages.${system}.tryout}/bin/anakins-c-ls-tryout";
        };

        devShells.default = pkgs.mkShell {
          name = "anakins-c-ls";

          packages = with pkgs; [
            bats
            clang-tools
            cscope
            diffutils
            jq
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
