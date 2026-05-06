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

        vscode-extension = pkgs.buildNpmPackage {
          pname = "anakins-c-ls-vscode";
          version = "0.0.1";
          src = ./vscode-extension;
          npmDepsHash = "sha256-vaR7rUhsbn36TZVXI+NgKq/NO9IUxnyBwzBIvpNxzmc=";
          nativeBuildInputs = [ pkgs.esbuild pkgs.zip ];
          buildPhase = ''
            esbuild src/extension.ts \
              --bundle \
              --outfile=out/extension.js \
              --external:vscode \
              --format=cjs \
              --platform=node
          '';
          installPhase = ''
            mkdir -p vsix/extension/out
            cp out/extension.js vsix/extension/out/
            cp package.json vsix/extension/
            cat > vsix/"[Content_Types].xml" << 'XMLEOF'
<?xml version="1.0" encoding="utf-8"?><Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types"><Default Extension="json" ContentType="application/json"/><Default Extension="js" ContentType="application/javascript"/><Default Extension="vsixmanifest" ContentType="text/xml"/></Types>
XMLEOF
            cat > vsix/extension.vsixmanifest << 'MEOF'
<?xml version="1.0" encoding="utf-8"?>
<PackageManifest Version="2.0.0" xmlns="http://schemas.microsoft.com/developer/vsx-schema/2011">
  <Metadata>
    <Identity Language="en-US" Id="anakins-c-ls" Version="0.0.1" Publisher="anakin4747"/>
    <DisplayName>anakins-c-ls</DisplayName>
    <Description>C/C++ Language Server</Description>
    <Tags>c,cpp,lsp</Tags>
  </Metadata>
  <Installation><InstallationTarget Id="Microsoft.VisualStudio.Code"/></Installation>
  <Assets><Asset Type="Microsoft.VisualStudio.Code.Manifest" Path="extension/package.json" Addressable="true"/></Assets>
</PackageManifest>
MEOF
            mkdir -p $out
            (cd vsix && zip -r $out/anakins-c-ls.vsix .)
          '';
        };

        tryout-vscode = pkgs.writeShellApplication {
          name = "tryout-vscode";
          runtimeInputs = [ pkgs.vscodium self.packages.${system}.default ];
          checkPhase = "";
          text = ''
            set +e +u +o pipefail
            kernel_root="''${1:-$(pwd)}"
            c_files="$(find "$kernel_root/drivers" \( -name '*.c' -o -name '*.h' \) 2>/dev/null | shuf -n 10 || true)"
            if [[ -z "$c_files" ]]; then
              echo "tryout-vscode: no .c or .h files found under $kernel_root/drivers" >&2
              echo "Run this from the root of a Linux kernel source tree." >&2
              exit 1
            fi

            ext_vsix="${vscode-extension}/anakins-c-ls.vsix"
            profile_dir="$(mktemp -d)"
            mkdir -p "$profile_dir/data/User"
            printf '{"security.workspace.trust.enabled":false}' > "$profile_dir/data/User/settings.json"

            codium \
              --extensions-dir "$profile_dir/extensions" \
              --install-extension "$ext_vsix"

            bin_dir="$(mktemp -d)"
            ln -s "$(command -v anakins-c-ls)" "$bin_dir/anakins-c-ls"
            ANAKINS_C_LS_BIN="$bin_dir/anakins-c-ls"
            export ANAKINS_C_LS_BIN
            PATH="$bin_dir:$PATH" codium \
              --extensions-dir "$profile_dir/extensions" \
              --user-data-dir "$profile_dir/data" \
              --disable-workspace-trust \
              --wait \
              $c_files || true
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

              c_files="$(find "$kernel_root/drivers" \( -name '*.c' -o -name '*.h' \) 2>/dev/null | shuf -n 10 || true)"
              if [[ -z "$c_files" ]]; then
                echo "tryout: no .c files found under $kernel_root/drivers" >&2
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

              exec nvim -u "$nvim_config/init.lua" $c_files
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
