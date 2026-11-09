# zmk-the-seahorse
Firmware for the sea horse pcb based on the Ferris Sweep
The board can be found here: https://github.com/nathansbradshaw/the-seahorse

V2 has a TrackPoint and uses the PS/2 UART driver module pinned in
[`config/west.yml`](config/west.yml).

## Firmware baseline

The firmware uses official [ZMK v0.3.0](https://github.com/zmkfirmware/zmk/releases/tag/v0.3.0)
at `edf5c0814fd3ea202e43aad2d68fd32e882a518c`, replacing the old
`infused-kim` mouse/urob fork. The PS/2 module uses the
[tails-dev adaptation](https://github.com/tails-dev/kb_zmk_ps2_mouse_trackpoint_driver)
at `68acb29332fd6446181374227df4a8ea1b5d47a8` for compatibility with ZMK's
official pointing APIs. It retains the existing UART TrackPoint wiring,
axis inversion, movement-triggered mouse layer, and sensitivity controls.

The reusable GitHub workflow is pinned to a revision compatible with v0.3's
Zephyr 3.5 board model. Updating the workflow to current development `main`
can fail its newer board-metadata check after successfully compiling.

This upgrade establishes a baseline before Bluetooth tuning. The existing
+8 dBm transmit power is retained. Split latency, queues, and PHY settings
use the release defaults. Hardware testing is required to establish typing,
TrackPoint, split-link, and battery behavior.

## Note:
This is ZMK for V2, check the releases for v1 releases if you a re using the original Seahorse.

# Flashing
See [Flashing Guide](https://zmk.dev/docs/development/build-flash)
## Local
Run the following command in the zmk/app path
```
west build --pristine -b nice_nano_v2 -- -DSHIELD=seahorse_v2_right -DZMK_CONFIG="absolute/path/to/this/repo/config" -DZMK_EXTRA_MODULES="absolute/path/to/this/repo"
```
alternativly the `-s /path/to/zmk/app` flag can be added to be run anywhere
```shell
west build --pristine -s /path/to/zmk/app -b nice_nano_v2 -- -DSHIELD=seahorse_v2_left -DZMK_CONFIG="absolute/path/to/this/repo/config" -DZMK_EXTRA_MODULES="absolute/path/to/this/repo"
```
the above command generates a file in `zmk/app/build/zephyr/zmk.uf2`
## Github
Run the existing **build.yml** workflow from GitHub Actions. Download its
`firmware` artifact after all three builds succeed.

### Update both halves

Flash `seahorse_v2_left-nice_nano_v2-zmk.uf2` to the left controller and
`seahorse_v2_right-nice_nano_v2-zmk.uf2` to the right controller using their
bootloaders. Keep copies of your known-working firmware before upgrading.
Start with the existing bonds; use the settings-reset image only if required
by ZMK's pairing-recovery procedure, because it erases saved settings.

With the right half connected to the computer over USB, check:

- All 34 keys, held-key releases, layers, and home-row modifiers.
- TrackPoint direction, automatic mouse-layer activation and timeout, mouse
  buttons, keyboard scrolling, and the existing sensitivity-up/down bindings.
- Left-half typing at a fixed separation, first while the TrackPoint is idle
  and then while moving it; reconnecting after restarting either half.

Record that baseline before applying Bluetooth-specific configuration changes.
Successful compilation does not establish physical wireless range or battery
runtime.

## TRACKPOINT
red trackpoint
[Red Track point pinout](Red_Track_Point.png)
