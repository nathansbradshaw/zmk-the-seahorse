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

The firmware favors split-link reliability, retaining the existing +8 dBm
transmit power on both halves. Hardware testing is required to establish
typing, TrackPoint, split-link, and battery behavior.

## Split Bluetooth reliability

The USB-connected right half is the central and owns the TrackPoint. Left-side
keys still travel over Bluetooth between the halves. If left-side keys drop
as the halves move apart while the right side continues working, test this
split link first.

| Setting | Untuned v0.3.0 baseline | Reliability configuration |
| --- | --- | --- |
| Transmit power, both halves | +8 dBm | +8 dBm (retained) |
| 2 Mbps PHY, both halves | Enabled | Disabled; use 1 Mbps |
| Left outgoing key-state snapshot queue | 10 | 64 |
| Right incoming key-transition queue | 5 | 64 |
| Peripheral latency, negotiated by the right half | 30 | 4 |
| Connection interval / supervision timeout | 7.5 ms / 4 s | 7.5 ms / 4 s (retained) |

Reducing peripheral latency lets the left half skip fewer idle connection
events: at most four rather than thirty. With a healthy link, that corresponds
to idle listening every 37.5 ms rather than 232.5 ms. This increases left-half
idle power consumption; measure actual battery runtime through a normal charge
cycle. Lower idle latency is not a guarantee of faster active typing or longer
range. Larger queues use RAM and absorb short processing/notification stalls;
they do not preserve events through a sustained radio outage or disconnect.

Disabling 2 Mbps mode follows the radio change used by
[ZMK's connection-stability option](https://zmk.dev/docs/config/bluetooth).
The direct controller setting applies to both halves without enabling other
experimental settings. See also
[ZMK's weak-connection guidance](https://zmk.dev/docs/troubleshooting/connection-issues#unreliableweak-connection).

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

Compare this firmware against the untuned ZMK v0.3.0 baseline at the same
separation and orientation. Include quick taps, chords, and held keys followed
by releases; check for missing, repeated, or stuck keys. Repeat at increasing
distances with the TrackPoint idle and moving. Successful compilation does not
establish physical wireless range or battery runtime.

If distance-dependent drops remain, compare antenna orientation and clearance
from batteries, metal mounts, and nearby USB 3 equipment. Radio configuration
cannot repair an obstructed or damaged antenna. A failure only during TrackPoint
motion also warrants checking PS/2 interrupt load and wiring interference.

## TRACKPOINT
red trackpoint
[Red Track point pinout](Red_Track_Point.png)
