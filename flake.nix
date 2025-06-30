{
  description = "A Nix-flake-based C/C++ development environment for wgpu-native applications";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  };
  inputs.self.submodules = true;

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
                inputs.self.overlays.default
              ];
            };
          }
        );

    in
    {
      overlays.default = final: prev: {
        wgpu-native = prev.wgpu-native.overrideAttrs (oldAttrs: {
          postInstall = (oldAttrs.postInstall or "") + ''
            mkdir -p $dev/lib/pkgconfig

            cat > $dev/lib/pkgconfig/wgpu-native.pc <<EOF
            prefix=$dev
            exec_prefix=$prefix
            libdir=$exec_prefix/lib
            includedir=$prefix/include

            Name: wgpu-native
            Description: Native WebGPU implementation based on wgpu-core
            Version: ${oldAttrs.version}
            Libs: -L$out -lwgpu_native
            Cflags: -I$dev/include/webgpu
            EOF
          '';
        });
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
                  cmake

                  # makes lsp work
                  bear

                  # rendering things
                  ## general
                  glew
                  emscripten
                  glfw
                  wgpu-native
                  vulkan-loader
                  vulkan-tools
                  vulkan-headers
                  ## linux
                  wayland-scanner
                  wayland
                  wayland-protocols
                  libxkbcommon
                  xorg.libX11
                  xorg.libXrandr
                  xorg.libXinerama
                  xorg.libXcursor
                  xorg.libXi
                  ## windows
                  # TODO
                  ## mac os
                  # TODO

                  # wgsl
                  wgsl-analyzer

                  # python scripts
                  python313
                  python313Packages.python-lsp-server
                  python313Packages.tkinter
                ];

                shellHook = ''
                  if [ ! -d $(pwd)/.emscripten_cache ]; then
                    cp -R ${pkgs.emscripten}/share/emscripten/cache/ $(pwd)/.emscripten_cache
                    chmod u+rwX -R $(pwd)/.emscripten_cache
                  fi
                  export EM_CACHE="$(pwd)/.emscripten_cache"
                '';

                LD_LIBRARY_PATH =
                  with pkgs;
                  lib.makeLibraryPath [
                    vulkan-loader
                  ];
              };
        }
      );

      packages = forEachSupportedSystem (
        { pkgs }:
        let
          pname = "moshading";
          version = "0.1.0";
        in {
          default = pkgs.clangStdenv.mkDerivation {
            inherit pname version;

            src = ./.;

            nativeBuildInputs = with pkgs; [
              python3
              meson
              ninja
              pkg-config
              makeWrapper
            ];

            buildInputs = with pkgs; [
              wgpu-native
              glfw
              glew
              wayland
              vulkan-loader
              xorg.libX11
              xorg.libXrandr
              xorg.libXinerama
              xorg.libXcursor
              xorg.libXi
            ];

            configurePhase = ''
                meson setup build
            '';
                buildPhase = ''
                meson compile -C build
            '';
            installPhase = ''
  mkdir -p $out/bin
  cp build/moshading $out/bin/moshading-raw

  makeWrapper $out/bin/moshading-raw $out/bin/moshading \
    --set LD_LIBRARY_PATH ${with pkgs; lib.makeLibraryPath [
      vulkan-loader
      glfw
      wayland
      xorg.libX11
      xorg.libXrandr
      xorg.libXinerama
      xorg.libXcursor
      xorg.libXi
    ]}
'';

            meta = {
              description = "C++ project using wgpu-native";
              license = pkgs.lib.licenses.mit;
              platforms = pkgs.lib.platforms.unix;
            };
          };
        }
      );
    };
}
