{
  description = "A Nix-flake-based C/C++ development environment";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    rust-overlay = {
      url = "github:oxalica/rust-overlay";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    inputs:
    let
      supportedSystems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];

      forEachSupportedSystem =
        f:
        inputs.nixpkgs.lib.genAttrs supportedSystems (
          system:
          f {
            pkgs = import inputs.nixpkgs {
              inherit system;
              overlays = [
                inputs.rust-overlay.overlays.default
                inputs.self.overlays.default
              ];
            };
          }
        );
    in
    {
      overlays.default = final: prev: {
        rustToolchain =
          let
            rust = prev.rust-bin;
          in
          if builtins.pathExists ./rust-toolchain.toml then
            rust.fromRustupToolchainFile ./rust-toolchain.toml
          else if builtins.pathExists ./rust-toolchain then
            rust.fromRustupToolchainFile ./rust-toolchain
          else
            rust.stable.latest.default.override {
              extensions = [
                "rust-src"
                "rustfmt"
              ];
            };
      };

      devShells = forEachSupportedSystem (
        { pkgs }:
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
                  emscripten

                  # correct lsp support
                  bear

                  # rust + wgpu-native
                  rustToolchain
                  openssl
                  pkg-config
                  cargo-deny
                  cargo-edit
                  cargo-watch
                  rust-analyzer
                  wgpu-native
                  wayland-scanner
                  wayland
                  wayland-protocols
                  libxkbcommon
                  xorg.libX11
                  xorg.libXrandr
                  xorg.libXinerama
                  xorg.libXcursor
                  xorg.libXi

                  vulkan-loader
                  vulkan-tools
                  vulkan-headers
                ];

                LIBCLANG_PATH = pkgs.lib.makeLibraryPath [ pkgs.llvmPackages_latest.libclang.lib ];
                shellHook = ''
                  export PATH=$PATH:''${CARGO_HOME:-~/.cargo}/bin
                  export PATH=$PATH:''${RUSTUP_HOME:-~/.rustup}/toolchains/$RUSTC_VERSION-x86_64-unknown-linux-gnu/bin/
                  if [ ! -d $(pwd)/.emscripten_cache ]; then
                    cp -R ${pkgs.emscripten}/share/emscripten/cache/ $(pwd)/.emscripten_cache
                    chmod u+rwX -R $(pwd)/.emscripten_cache
                  fi
                  export EM_CACHE="$(pwd)/.emscripten_cache"
                '';
                # Add precompiled library to rustc search path
                RUSTFLAGS = (
                  builtins.map (a: ''-L ${a}/lib'') [
                    # add libraries here (e.g. pkgs.libvmi)
                  ]
                );
                LD_LIBRARY_PATH =
                  with pkgs;
                  lib.makeLibraryPath [
                    # load external libraries that you need in your rust project here
                    wayland
                    wayland-protocols
                    libxkbcommon
                    xorg.libX11
                    xorg.libXrandr
                    xorg.libXinerama
                    xorg.libXcursor
                    xorg.libXi
                    vulkan-loader
                    vulkan-tools
                    vulkan-headers
                  ];

                # Add glibc, clang, glib, and other headers to bindgen search path

                BINDGEN_EXTRA_CLANG_ARGS =
                  # Includes normal include path
                  let
                    clangMajorVersion = builtins.head (builtins.match "([0-9]+).*" pkgs.libclang.version);
                  in
                  (builtins.map (a: ''-I"${a}/include"'') [
                    # add dev libraries here (e.g. pkgs.libvmi.dev)
                    pkgs.glibc.dev
                  ])
                  # Includes with special directory paths
                  ++ [
                    ''-I"${pkgs.libclang.lib}/lib/clang/${clangMajorVersion}/include"''
                    ''-I"${pkgs.glib.dev}/include/glib-2.0"''
                    ''-I${pkgs.glib.out}/lib/glib-2.0/include/''
                    ''-I${pkgs.wgpu-native.dev}/include/webgpu''
                  ];
              };
        }
      );
    };
}
