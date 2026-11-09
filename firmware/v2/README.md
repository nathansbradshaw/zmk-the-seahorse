# Seahorse v2.1.0 firmware

This is the active v2 profile in the maintained Seahorse firmware repository.
The repository combines the original
`zmk-the-seahorse` history with the deployed v2.1.0 configuration from
[`zmk-seahorse-config`](https://github.com/nathansbradshaw/zmk-seahorse-config)
at `419d09b184cee938bf4dffc09b12a96de93c367b`. Both Git histories are preserved.
The [keyboard hardware](https://github.com/nathansbradshaw/the-seahorse) remains
in its hardware repository.

## Configuration and features

The active keymap is
[`boards/shields/seahorse_v2/seahorse_v2.keymap`](boards/shields/seahorse_v2/seahorse_v2.keymap).
The imported v2.1.0 keymap, physical layout, and TrackPoint setup are retained:

- Alpha, Nav, Num, Mouse, Games, G-Right, G-Left, and misc layers, plus two
  reserved layers for Studio.
- Home-row modifiers, Hyper keys, gaming toggle, Bluetooth profile controls,
  and the existing Studio Unlock binding.
- Studio over USB on the central/right half, with saved keymap support and
  the 34-key physical layout.
- The right-side nice!view adapter and nice!view gem custom status screen.
- The unused middle panel plays silent Bad Apple once per USB power
  connection, stopping on unplug and clearing after playback. It stays
  inactive on battery. See the [display implementation and regeneration guide](boards/shields/nice_view_gem/BAD_APPLE.md).
- UART TrackPoint on the original pins, sensitivity 100, 80 Hz sampling,
  inverted X, disabled stick-clicking, temporary mouse-layer activation, input
  scaling, and scrolling processors.

The old `config/seahorse_v2.keymap` and its include files have been removed so
they cannot override the imported shield keymap. The `config` directory owns
the dependency manifest and the shared Bluetooth settings in
`config/seahorse_v2.conf`; shield-specific settings and input processing live
under `boards/shields/seahorse_v2`.

## Bluetooth reliability

Both halves use +8 dBm transmit power and the experimental connection setting,
with `CONFIG_BT_CTLR_PHY_2M=n` explicitly disabling 2 Mbps on both controllers.
The imported v2.1.0 source requested these settings in a generic shield `.conf`
that was not loaded by the suffixed left/right builds. The first consolidated
build therefore still used 0 dBm and enabled 2 Mbps. Shared settings now live
in the user config file that ZMK actually loads for both halves.

The additional tuning increases the left outgoing key-state queue from 10 to
64 and the right incoming transition queue from 5 to 64. Peripheral latency is
reduced from 30 to 4 while retaining the 7.5 ms connection interval and 4 s
supervision timeout. A healthy idle link can therefore listen every 37.5 ms
rather than 232.5 ms. This increases left-half idle power consumption; actual
battery runtime must be measured. The right side can stay USB-powered.

Queues absorb short processing stalls. They cannot preserve events through a
sustained radio outage. Compare typing at fixed distances and orientations,
including quick taps, chords, held-key releases, and TrackPoint movement.
Build success does not establish physical range, input reliability, or battery
runtime. See [ZMK's connection guidance](https://zmk.dev/docs/troubleshooting/connection-issues).

## Build and flash

The build uses official [ZMK v0.3.0](https://github.com/zmkfirmware/zmk/releases/tag/v0.3.0).
ZMK, the original badjeff PS/2 driver, and zmk-helpers are pinned
in [`config/west.yml`](config/west.yml). The nice!view gem source is maintained
inside this profile, including its original license and pinned provenance.
The small `ZMK_MOUSE` compatibility
symbol maps the original PS/2 driver's legacy selection to ZMK's native pointing
subsystem without replacing that driver's multi-device/settings implementation.
The local reusable workflow preserves the pinned recipe for the release's
Zephyr 3.5 board model and adds Rust 1.85.1 for the display's `no_std` decoder.

Run **Build ZMK firmware** in GitHub Actions. The existing two-half matrix
includes the Studio USB snippet and nice!view shields on the right half.
Download its `firmware` artifact and flash the matching image to each
nice!nano v2 controller using its bootloader:

- Left: `seahorse_v2_left-nice_nano_v2-zmk.uf2`
- Right: `seahorse_v2_right-nice_nano_v2-zmk.uf2`

Keep your known-working firmware. Start with the saved settings and bonds;
do not erase settings as part of this update. Studio remaps stored on the
device take precedence over the stock keymap. Avoid **Restore Stock Settings**
unless you intend to discard those saved remaps.

## Connect Studio

Connect the right half over USB and open [ZMK Studio](https://zmk.studio/) in a
supported browser or its native app. Use the existing unlock binding: with the
stock keymap, hold the Tab and Backspace thumb keys to activate **misc**, then
press the physical **Z** key (leftmost key on the bottom letter row). The saved
keymap may place that binding elsewhere if it was remapped in Studio.

Verify the complete layer set, key positions, saved changes across a reconnect,
TrackPoint controls, and the nice!view screen on hardware after updating.

## TrackPoint research

[TrackPoint feel and battery research](docs/trackpoint-research.md) separates
published findings, community reports, and source-level observations. The
display update preserves current TrackPoint tuning and Bluetooth settings.
