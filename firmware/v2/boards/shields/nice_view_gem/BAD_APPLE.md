# Seahorse display and Bad Apple

The local shield is imported from `nathansbradshaw/nice-view-gem` at
`3e94b8c3e462e070a56d022329d471cff7428f0e`. Its MIT license is retained in
`LICENSE`. Keeping the display in this firmware profile makes the complete
v2 source export include the customization, rather than requiring another
mutable display repository. The original battery, connection, profile, and
layer widgets are retained.

The central/right middle panel plays the full silent Bad Apple silhouette
video, centered at 64×48 pixels, at 12 frames per second. Playback starts
when USB power becomes available, runs once, then clears the panel. Unplugging
stops and clears it; the next connection starts from the beginning. Starting
the keyboard while already plugged in also counts as a connection. USB
enumeration, host suspend/resume, battery updates, and Studio connections
do not restart a completed playback while power stays present. Power-only
chargers count too, provided the controller detects USB power.

The video has 2,629 frames (about 3 minutes 39 seconds), compressed into
265,551 bytes of flash. An allocation-free Rust LZSS decoder uses a 4 KiB history and one
384-byte frame. Display callbacks decode at most three frames per tick to
bound catch-up work. The animation timer is paused on battery and after
completion. This does not change the existing display's global refresh or
idle policy. No audio, USB streaming, logging transport, or keymap bindings
are added. All LVGL calls run on ZMK's display queue.

## Asset provenance and regeneration

Original shadow-art video: **あにら / Anira**,
[Bad Apple!! 影絵](https://www.nicovideo.jp/watch/sm8628149).
Touhou characters and original composition are by ZUN / Team Shanghai Alice;
the familiar arrangement is by Alstroemeria Records, featuring nomico.
Audio is discarded. These credits identify the media; the shield's MIT
license does not relicense the original video.

Conversion input is the MP4 from
[`C-mykk/Bad-Apple-Starter`](https://github.com/C-mykk/Bad-Apple-Starter/blob/7b04ba8eb92b778c14ddc378620ada59da0e5aed/BadApple!!.mp4),
commit `7b04ba8eb92b778c14ddc378620ada59da0e5aed`.
Its SHA-256 is
`d8f21232313506da4f8d631ca289793bcf2f0a33440d68ccb02f4ac52e963cd0`.
The generated compressed stream's SHA-256 is
`e41c01ed9e085f95786d43604386fd2ec9401654f36e418eca9392d0c2a79edc`.
The source MP4 is not needed when building firmware.

From the v2 profile directory, with Rust and FFmpeg installed:

```sh
rustc --edition=2021 -O tools/pack_bad_apple.rs -o /private/tmp/pack_bad_apple
ffmpeg -v error -i bad-apple.mp4 -an \
  -vf 'fps=12,scale=64:48:flags=lanczos' -pix_fmt gray -f rawvideo \
  /private/tmp/bad-apple.gray
/private/tmp/pack_bad_apple boards/shields/nice_view_gem/assets < /private/tmp/bad-apple.gray
```

The converter thresholds pixels at 128 and verifies every decoded pixel
against its uncompressed frames before writing the asset. Only
`assets/bad_apple_data.inc` is versioned; `bad_apple.bin` is a disposable
verification output. Keep the documented input and FFmpeg version when
comparing byte-for-byte regeneration (initial conversion: FFmpeg 8.1.2).

## Native verification

These Rust tests exercise the actual production Rust codec and session controller.
From the v2 profile directory:

```sh
task_tmp=$(mktemp -d)
trap 'rm -rf "$task_tmp"' EXIT
rustc --edition=2021 --test boards/shields/nice_view_gem/widgets/bad_apple_core.rs \
  -o "$task_tmp/player-tests"
"$task_tmp/player-tests"
rustc --edition=2021 --test tools/pack_bad_apple.rs -o "$task_tmp/pack-tests"
"$task_tmp/pack-tests"
```

The checks cover the complete embedded stream, overlapping references across
frame boundaries, malformed/truncated input, uptime wrap, and once-per-USB
connection behavior. Firmware CI also builds both halves. Hardware acceptance
must still check orientation, playback, unplug/replug, and normal typing,
TrackPoint, display indicators, and Studio while the animation runs.

## Firmware toolchain

Firmware CI uses the local copy of the previously pinned ZMK v0.3 build recipe
and installs Rust 1.85.1 with `thumbv7em-none-eabi` for the right build. The
root and standalone-profile recipes are identical and checked by CI. On a
local ZMK development system, install that target with
`rustup target add thumbv7em-none-eabi`; CMake invokes `rustc` directly to link
the `no_std` static library. No Cargo dependencies or runtime allocation are
needed. The existing C widget provides the LVGL drawing/event interface.
