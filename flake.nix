{
  description = "A Nix-flake-based C/C++ development environment";

  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";

  outputs =
    { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
    in
    {
          default =
            pkgs.mkShell.override
              {
                # Override stdenv in order to change compiler:
                stdenv = pkgs.clangStdenv;
              }
              rec {
                packages = with pkgs; [
                  # c/c++
                  clang-tools
                  gdb
                  valgrind
                  meson
                  ninja
                  pkg-config
                  glew
                  cmake

                  # imgui
                  imgui
                  wgpu-native
                  emscripten

                  # correct lsp support
                  bear

                ];

                # TODO make this cleaner
                shellHook = ''
                  if [ ! -d $(pwd)/.emscripten_cache ]; then
                    cp -R ${pkgs.emscripten}/share/emscripten/cache/ $(pwd)/.emscripten_cache
                    chmod u+rwX -R $(pwd)/.emscripten_cache
                  fi
                  export EM_CACHE=$(pwd)/.emscripten_cache
                  export CFLAGS="-I${pkgs.wgpu-native.dev}/include/webgpu";
                  export LDFLAGS="-L${pkgs.wgpu-native.out}/lib -lwgpu_native";
                '';
              };
    };
}
