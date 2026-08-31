#################################################################
# official mkDerivation attrs:
  # https://nix.dev/tutorials/callpackage.html
  # https://nixos.org/manual/nixpkgs/stable/#sec-stdenv-phases
  # https://nixos.org/manual/nixpkgs/stable/#var-stdenv-phases
# Default Phase order: 1 unpack, 2 patch, 3 configure, 4 build,
# 5 check, 6 install, 7 fixup, 8 installCheck, 9 dist
# where {$var} is the phase name there is a 
# `pre{$var}`, `{$var}Phase`, and `post{$var}` for each phase
#################################################################

{ lib, stdenv, makeheaders, minimus }:

stdenv.mkDerivation (rec {
  pname = "PIthon";
  version = "1.0.0";
  
  src = ./.;
  
  nativeBuildInputs = [ makeheaders ];
  buildInputs = [ minimus ];

  buildPhase = ''
    makeheaders -local ./pargs.c
    makeheaders -local ./FileIO.c
    $CC -m64 -O2 -std=gnu99 -I${minimus}/include -include annexb.h -include NuClear.h pargs.c -L${minimus}/lib -lNuClear -o pargs
    $CC -m64 -O2 -std=gnu99 -Wno-incompatible-pointer-types -I${minimus}/include FileIO.c -L${minimus}/lib -lNuClear -o FileIO
  '';

  installPhase = ''
    runHook preInstall
    mkdir -p $out/bin
    install -Dm755 pargs $out/bin/pargs
    install -Dm755 FileIO $out/bin/FileIO
    runHook postInstall
  '';

  preFixup = ''
    patchelf --set-rpath "${minimus}/lib" $out/bin/pargs
    patchelf --set-rpath "${minimus}/lib" $out/bin/FileIO
  '';

  outputs = [ "out" ];

  meta = {
    description = "PIthon pargs utility";
    homepage = "https://github.com/Viruliant/PIthon";
    license = lib.licenses.bsd2;
    platforms = lib.platforms.unix;
    maintainers = [ lib.maintainers.GlassGhost ];
    mainProgram = "pargs";
  };
})
