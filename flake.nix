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
        treeSitterC = pkgs.tree-sitter-grammars.tree-sitter-c;
      in
      {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "anakins-c-ls";
          version = "0.1.0";
          src = ./.;

          nativeBuildInputs = [ pkgs.makeWrapper ];
          buildInputs = [ pkgs.tree-sitter treeSitterC ];

          buildPhase = ''
            mkdir -p out
            cc -Wall -Wextra \
              -Isrc \
              -I${pkgs.tree-sitter}/include \
              -o out/anakins-c-ls src/main.c \
              -L${pkgs.tree-sitter}/lib \
              -L${treeSitterC} \
              -ltree-sitter \
              -l:parser
          '';

          installPhase = ''
            mkdir -p $out/bin
            cp out/anakins-c-ls $out/bin/
            wrapProgram $out/bin/anakins-c-ls \
              --prefix LD_LIBRARY_PATH : ${pkgs.tree-sitter}/lib:${treeSitterC}
          '';
        };

        devShells.default = pkgs.mkShell {
          name = "anakins-c-ls";

          packages = with pkgs; [
            bats
            diffutils
            jq
            shellcheck
            tree-sitter
            treeSitterC
            self.packages.${system}.default
          ];

          # nixpkgs tree-sitter-c grammar ships its shared library as
          # "parser" (no lib prefix) in the package root.  Create a
          # conventional symlink so "-ltree-sitter-c" resolves correctly
          # when building from within the dev shell, and expose the
          # directory on LD_LIBRARY_PATH so the binary can load it at
          # runtime.
          shellHook = ''
            mkdir -p "$PWD/.nix-dev"
            ln -sf ${treeSitterC}/parser "$PWD/.nix-dev/libtree-sitter-c.so"
            export NIX_LDFLAGS="-L$PWD/.nix-dev $NIX_LDFLAGS"
            export LD_LIBRARY_PATH="$PWD/.nix-dev:${pkgs.tree-sitter}/lib''${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
          '';
        };
      });
}
