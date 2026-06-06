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

        # Wrapper script that captures audio from BlackHole via ffmpeg and pipes to cthugha
        cthughanix-audio = pkgs.writeShellScriptBin "cthughanix-audio" ''
          set -e

          BLACKHOLE_DEVICE="BlackHole 2ch"
          GAIN="20"

          # Parse our own flags before passing rest to cthugha
          ARGS=()
          while [ $# -gt 0 ]; do
            case "$1" in
              --gain) GAIN="$2"; shift 2 ;;
              --gain=*) GAIN="''${1#--gain=}"; shift ;;
              *) ARGS+=("$1"); shift ;;
            esac
          done

          # Check BlackHole is installed
          if ! ${pkgs.ffmpeg}/bin/ffmpeg -f avfoundation -list_devices true -i "" 2>&1 | grep -q "$BLACKHOLE_DEVICE"; then
            echo "ERROR: BlackHole 2ch not found." >&2
            echo "" >&2
            echo "Install it with:  brew install --cask blackhole-2ch" >&2
            echo "" >&2
            echo "Then create a Multi-Output Device in Audio MIDI Setup" >&2
            echo "that sends audio to both your speakers and BlackHole 2ch." >&2
            exit 1
          fi

          # Find BlackHole device index
          BH_INDEX=$(${pkgs.ffmpeg}/bin/ffmpeg -f avfoundation -list_devices true -i "" 2>&1 \
            | grep "audio devices" -A50 | grep "$BLACKHOLE_DEVICE" | grep -o '\[[0-9]*\]' | tr -d '[]')

          # Build ffmpeg filter chain
          AF_OPTS=""
          if [ -n "$GAIN" ]; then
            AF_OPTS="-af volume=$GAIN"
          fi

          # Pipe ffmpeg output directly into cthugha via stdin
          exec ${cthughanix}/bin/glcthugha \
            -D 3 --play /dev/stdin --silent --rate 48000 --stereo --snd-format 3 \
            "''${ARGS[@]}" < <(${pkgs.ffmpeg}/bin/ffmpeg -nostdin \
              -f avfoundation -i ":$BH_INDEX" \
              -probesize 32 -analyzeduration 0 \
              $AF_OPTS -f s16le -acodec pcm_s16le -ar 48000 -ac 2 pipe:1 2>/dev/null)
        '';
      in
      {
        packages = {
          default = cthughanix;
          cthughanix = cthughanix;
          cthughanix-audio = cthughanix-audio;
        };

        apps.default = {
          type = "app";
          program = "${cthughanix-audio}/bin/cthughanix-audio";
        };

        apps.glcthugha = {
          type = "app";
          program = "${cthughanix}/bin/glcthugha";
        };

        apps.no-audio = {
          type = "app";
          program =
            if isDarwin
            then "${cthughanix}/bin/glcthugha"
            else "${cthughanix}/bin/xcthugha";
        };

        devShells.default = pkgs.mkShell {
          inputsFrom = [ cthughanix ];
          packages = [ pkgs.ffmpeg ];
        };
      }
    );
}
