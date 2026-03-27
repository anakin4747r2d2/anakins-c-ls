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
      in
      {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "anakins-c-ls";
          version = "0.1.0";
          src = ./.;
          buildPhase = "cc -Wall -Wextra -o anakins-c-ls src/main.c";
          installPhase = ''
            mkdir -p $out/bin
            cp anakins-c-ls $out/bin/
          '';
        };

        devShells.default = pkgs.mkShell {
          name = "anakins-c-ls";

          packages = with pkgs; [
            bats
            shellcheck
            self.packages.${system}.default
          ];
        };
      });
}
