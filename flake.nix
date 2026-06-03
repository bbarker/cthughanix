{
  description = "CthughaNix - An Oscilloscope on Acid (audio visualization)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.11";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        isDarwin = pkgs.stdenv.isDarwin;

        cthughanix = pkgs.stdenv.mkDerivation {
          pname = "cthughanix";
          version = "1.5";

          src = ./.;

          nativeBuildInputs = with pkgs; [
            pkg-config
          ];

          buildInputs = with pkgs; [
            ncurses
          ] ++ pkgs.lib.optionals (!isDarwin) [
            libGL
            libGLU
            freeglut
            xorg.libX11
            xorg.libXext
            xorg.libXt
            xorg.libXaw
            xorg.libXmu
            xorg.libXpm
            xorg.libXi
          ] ++ pkgs.lib.optionals isDarwin [
            darwin.apple_sdk.frameworks.OpenGL
            darwin.apple_sdk.frameworks.GLUT
          ];

          configureFlags = [
            "--disable-svga"
            "--without-dsp"
            "--without-cdrom"
            "--without-mixer"
            "--without-network"
            "--without-mpg123"
          ] ++ pkgs.lib.optionals isDarwin [
            "--disable-xwin"
            "--disable-serv"
          ];

          preConfigure = ''
            chmod +x configure config.guess config.sub install-sh mkinstalldirs missing depcomp
            substituteInPlace Makefile.in --replace "SUBDIRS = src tab doc" "SUBDIRS = src tab"
          '' + pkgs.lib.optionalString isDarwin ''
            # Provide GL/ compatibility headers that forward to macOS frameworks
            mkdir -p gl-compat/GL
            echo '#include <OpenGL/gl.h>' > gl-compat/GL/gl.h
            echo '#include <OpenGL/glu.h>' > gl-compat/GL/glu.h
            echo '#include <GLUT/glut.h>'  > gl-compat/GL/glut.h
            cat > gl-compat/GL/glx.h <<'EOF'
            #ifndef __glx_h__
            #define __glx_h__
            #endif
            EOF
            export CPPFLAGS="''${CPPFLAGS:-} -I$(pwd)/gl-compat"
          '';

          preInstall = ''
            substituteInPlace src/Makefile --replace "-o root -m 4775" ""
          '';

          postInstall = ''
            mkdir -p $out/lib/cthughanix/{map,pcx,tab}
            cp map/*.map $out/lib/cthughanix/map/ || true
            cp pcx/*.pcx.gz $out/lib/cthughanix/pcx/ || true
            cp tab/*.cmd $out/lib/cthughanix/tab/ || true
          '';

          meta = with pkgs.lib; {
            description = "An Oscilloscope on Acid - audio visualization tool";
            homepage = "https://cthughanix.sourceforge.net/";
            license = licenses.lgpl2;
            platforms = platforms.unix;
          };
        };
      in
      {
        packages = {
          default = cthughanix;
          cthughanix = cthughanix;
        };

        apps.default = {
          type = "app";
          program =
            if isDarwin
            then "${cthughanix}/bin/glcthugha"
            else "${cthughanix}/bin/xcthugha";
        };

        apps.glcthugha = {
          type = "app";
          program = "${cthughanix}/bin/glcthugha";
        };

        devShells.default = pkgs.mkShell {
          inputsFrom = [ cthughanix ];
        };
      }
    );
}
