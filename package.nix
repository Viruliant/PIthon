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

{ lib, stdenv, makeheaders, minimus, ncurses, PiChudnov }:

stdenv.mkDerivation (rec {
  pname = "PIthon";
  version = "1.0.0";
  
  src = ./.;
  
  nativeBuildInputs = [ makeheaders ];
  buildInputs = [ minimus ncurses PiChudnov ];

buildPhase = ''
    # All auto-generated headers come from `filter-makeheaders`, the
    # macro-aware makeheaders wrapper — never from makeheaders directly.
    # filter-makeheaders -I${minimus}/include ./pargs.c
    filter-makeheaders -I${minimus}/include ./FileIO.c
    filter-makeheaders -I${minimus}/include ./testFileIO.c
#     filter-makeheaders -I${minimus}/include ./PIthon.c

    $CC -c -m64 -O2 -std=gnu99 -I${minimus}/include -include annexb.h -include NuClear.h FileIO.c -o FileIO.o
    $CC -m64 -O2 -std=gnu99 -Wno-incompatible-pointer-types -I${minimus}/include testFileIO.c FileIO.o -L${minimus}/lib -lNuClear -o testFileIO
    $CC -m64 -O2 -std=gnu99 -Wno-error=return-mismatch -I${minimus}/include -I${ncurses}/include -I${PiChudnov}/include -include annexb.h -include NuClear.h PIthon.c -L${minimus}/lib -lNuClear -lncurses -L${PiChudnov}/lib -lpichudnov -o PIthon
    # $CC -m64 -O2 -std=gnu99 -I${minimus}/include -include annexb.h -include NuClear.h pargs.c -L${minimus}/lib -lNuClear -o pargs
  '';

  installPhase = ''
    runHook preInstall
    mkdir -p $out/bin
#     cp PIthon.h $out/include/

    # install -Dm755 pargs $out/bin/pargs
    install -Dm755 testFileIO $out/bin/testFileIO
    install -Dm755 PIthon $out/bin/PIthon
    runHook postInstall
  '';

  preFixup = ''
    # patchelf --set-rpath "${minimus}/lib" $out/bin/pargs
    patchelf --set-rpath "${minimus}/lib" $out/bin/testFileIO
    patchelf --set-rpath "${minimus}/lib:${ncurses}/lib:${PiChudnov}/lib" $out/bin/PIthon
  '';

  outputs = [ "out" ];

  meta = {
    description = "PIthon pargs utility";
    homepage = "https://github.com/Viruliant/PIthon";
    license = lib.licenses.bsd2;
    platforms = lib.platforms.unix;
    maintainers = [ lib.maintainers.GlassGhost ];
    mainProgram = "PIthon";
  };
})
