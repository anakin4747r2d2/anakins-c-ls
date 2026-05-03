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

          tryout = pkgs.writeShellApplication {
            name = "tryout";
            runtimeInputs = with pkgs; [ neovim gnused gnugrep self.packages.${system}.default ];
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
              vim.opt.swapfile = false
              vim.lsp.config("anakins-c-ls", {
                cmd = { "anakins-c-ls" },
                filetypes = { "c" },
                root_dir = "$kernel_root",
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
