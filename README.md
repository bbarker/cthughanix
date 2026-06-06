# CthughaNix - An Oscilloscope on Acid

## Relicensing and the Future

This is a relicensing, renaming, and modernization of the Cthugha-L
distribution. Much remains to be done, though if you are using
Linux/x86 you should be able to enjoy Cthugha immediately.
Please see the TODO file for more information.

For details on the new licence see the `COPYING` file.

The application's web site is http://cthughanix.sourceforge.net/

## Installation

### Nix (recommended)

CthughaNix is packaged as a Nix flake. To run directly:

```bash
nix run github:bbarker/cthughanix
```

Or from a local checkout:

```bash
nix run
```

Available apps:

- `nix run` — launches with live audio capture (requires BlackHole on macOS)
- `nix run .#no-audio` — launches with random noise visualization (no audio setup needed)
- `nix run .#glcthugha` — launches the GL version directly without the audio wrapper

To enter a development shell:

```bash
nix develop
```

### Traditional (autotools)

See the `INSTALL` file for details on `./configure && make && make install`.

## macOS Setup (Audio Capture)

On macOS there is no `/dev/dsp` — to visualize audio from Spotify or other apps,
you need a virtual loopback audio device. CthughaNix uses **BlackHole** for this.

### 1. Install BlackHole

```bash
brew install --cask blackhole-2ch
```

### 2. Restart Core Audio (if BlackHole doesn't appear)

If BlackHole doesn't show up in Audio MIDI Setup after installation, restart
the Core Audio daemon:

```bash
sudo launchctl kickstart -kp system/com.apple.audio.coreaudiod
```

or, failing that:

```bash
sudo killall coreaudiod
```

This causes a brief (~1 second) audio interruption.

### 3. Create a Multi-Output Device

1. Open **Audio MIDI Setup** (in `/Applications/Utilities/` or search Spotlight)
2. Click the **+** button at the bottom-left → "Create Multi-Output Device"
3. Check both your speakers/headphones **and** "BlackHole 2ch"
4. Optionally rename it (e.g., "Speakers + BlackHole")

### 4. Set System Audio Output

Go to **System Settings → Sound → Output** and select your new Multi-Output Device.
Audio will now play through your speakers *and* be routed to BlackHole for CthughaNix
to capture.

### 5. Run

```bash
nix run .#cthughanix-audio
```

The wrapper script will detect BlackHole, launch ffmpeg to capture audio from it,
and pipe it into the visualizer.

If your system volume is low, use the `--gain` flag to amplify the signal:

```bash
nix run .#cthughanix-audio -- --gain 50
```

## Keyboard Controls

- `l` — Lock/unlock auto-changing (pause cycling through visualizations)
- `Esc` — Quit
- `F1` — Help screen

## Tips

- `xcthugha` does not react to `-geometry` in the normal way; use the `-D` and
  `--position` options.
- Take a look at the documentation — it's quite long but still useful.
- Joystick: the movement of the display can be controlled by a joystick.
  Use `--no-joystick` for automatic (random) movement.
- Example of full screen options for X:
  ```bash
  ./xcthugha -D 5 --full-screen --play ~/Music/song.mp3
  ```

## Bugs

See the file `TODO`. Known bugs and problems are usually listed there.

If you have problems with the sound (sound reading errors), use the
`--snd-method` option. Setting a value of 3 should work.

Bug tracker: https://sourceforge.net/tracker/?group_id=201465&atid=977671

## Contact

- E-mail: brandon.barker@gmail.com
- Blog: http://brandonbarker.blogspot.com
- Official WWW: http://cthughanix.sourceforge.net/
- Newsgroup: alt.graphics.cthugha
