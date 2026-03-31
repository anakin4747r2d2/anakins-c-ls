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
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "anakins-c-ls";
          version = "0.1.0";
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

        devShells.default = pkgs.mkShell {
          name = "anakins-c-ls";

          packages = with pkgs; [
            bats
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
