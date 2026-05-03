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
              printf 'vim.lsp.set_log_level("off")\n' > "$nvim_config/init.lua"
              printf 'vim.api.nvim_create_autocmd({ "BufRead", "BufNewFile" }, {\n' >> "$nvim_config/init.lua"
              printf '    pattern = { "*.c", "*.h" },\n' >> "$nvim_config/init.lua"
              printf '    callback = function()\n' >> "$nvim_config/init.lua"
              printf '        vim.lsp.start({\n' >> "$nvim_config/init.lua"
              printf '            name = "anakins-c-ls",\n' >> "$nvim_config/init.lua"
              printf '            cmd = { "anakins-c-ls" },\n' >> "$nvim_config/init.lua"
              printf '            root_dir = "%s",\n' "$kernel_root" >> "$nvim_config/init.lua"
              printf '            filetypes = { "c" },\n' >> "$nvim_config/init.lua"
              printf '        })\n' >> "$nvim_config/init.lua"
              printf '    end,\n' >> "$nvim_config/init.lua"
              printf '})\n' >> "$nvim_config/init.lua"

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
