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

        vscode-extension = pkgs.stdenv.mkDerivation {
          pname = "anakins-c-ls-vscode";
          version = "0.0.1";
          src = ./vscode-extension;
          installPhase = ''
            mkdir -p $out
            cp -r out package.json $out/
          '';
        };

        tryout-vscode = pkgs.writeShellApplication {
          name = "tryout-vscode";
          runtimeInputs = [ pkgs.vscodium self.packages.${system}.default ];
          checkPhase = "";
          text = ''
            set +e +u +o pipefail
            kernel_root="''${1:-$(pwd)}"
            c_file="$(find "$kernel_root/drivers" \( -name '*.c' -o -name '*.h' \) 2>/dev/null | shuf -n 1 || true)"
            if [[ -z "$c_file" ]]; then
              echo "tryout-vscode: no .c or .h files found under $kernel_root/drivers" >&2
              echo "Run this from the root of a Linux kernel source tree." >&2
              exit 1
            fi

            ext_dir="${vscode-extension}"
            profile_dir="$(mktemp -d)"

            codium \
              --extensions-dir "$profile_dir/extensions" \
              --install-extension "$ext_dir" \
              --wait \
              "$c_file" || true
          '';
        };
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
              vim.api.nvim_create_autocmd({ "BufRead", "BufNewFile" }, {
                pattern = { "*.c", "*.h" },
                callback = function()
                  vim.lsp.start({
                    name = "anakins-c-ls",
                    cmd = { "anakins-c-ls" },
                    root_dir = "$kernel_root",
                  })
                end,
              })
              EOF

              exec nvim -u "$nvim_config/init.lua" "$c_file"
            '';
          };

          vscode-extension = vscode-extension;

          tryout-vscode = tryout-vscode;
        };

        apps.tryout = {
          type = "app";
          program = "${self.packages.${system}.tryout}/bin/tryout";
        };

        apps.tryout-vscode = {
          type = "app";
          program = "${self.packages.${system}.tryout-vscode}/bin/tryout-vscode";
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
