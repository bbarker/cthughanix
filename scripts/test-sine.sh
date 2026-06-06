#!/usr/bin/env bash
cd "$(dirname "$0")"
BIN=$(nix build .#cthughanix --print-out-paths 2>/dev/null)/bin/glcthugha

echo "Testing s16le with volume=11 (full scale sine)"

"$BIN" -D 3 -l --flat --no-trans --wave 2 --flame 0 \
  --play /dev/stdin --silent --rate 48000 --stereo --snd-format 3 \
  < <(nix develop --command bash -c '
    ffmpeg -nostdin -f lavfi -i "sine=frequency=440:sample_rate=48000:duration=5" -af "volume=11" -ac 2 -f s16le pipe:1 2>/dev/null
    while true; do dd if=/dev/zero bs=19200 count=1 2>/dev/null; sleep 0.1; done
  ')
