# zmk-the-seahorse
Firmware for the sea horse pcb based on the Ferris Sweep
The board can be found here: https://github.com/nathansbradshaw/the-seahorse

V2 has a trackpoint and therefore needs special setup if building locally

## Note:
This is ZMK for V2, check the releases for v1 releases if you a re using the original Seahorse.

# Flashing
See [Flashing Guide](https://zmk.dev/docs/development/build-flash)
## Local
Run the following command in the zmk/app path
```
west build --pristine -b nice_nano_v2 -- -DSHIELD=blaster_pad -DZMK_EXTRA_MODULES="absolute/path/to/this/repo"
```
alternativly the `-s /path/to/zmk/app` flag can be added to be run anywhere
```shell
west build --pristine -b nice_nano_v2 -- -DSHIELD=blaster_pad -DZMK_EXTRA_MODULES="absolute/path/to/this/repo" -s /path/to/zmk/app
```
the above command generates a file in `zmk/app/build/zephyr/zmk.uf2`
## Github
fork this repo

## TRACKPOINT
red trackpoint
[Red Track point pinout](Red_Track_Point.png)

## Split Bluetooth reliability

The right half is the central and owns the TrackPoint. With the right half
connected to the computer over USB, left-side keys still travel over Bluetooth
between the halves. If only left-side keys drop as the halves move apart, test
this split link first.

The firmware favors reliability while retaining some idle power saving:

| Setting | Left half | Right half |
| --- | --- | --- |
| Radio transmit power | +8 dBm (already enabled previously) | +8 dBm (already enabled previously) |
| 2 Mbps PHY | Disabled; use 1 Mbps | Disabled; use 1 Mbps |
| Split key-event queue | 64 outgoing snapshots (previously 10) | 64 incoming transitions (previously 5) |
| Split connection parameters | Negotiated by the right half | 7.5 ms interval, latency 4 (previously 30), 4 s supervision timeout |

The lower peripheral latency lets the left half skip fewer idle connection
events. It increases idle radio use, so measure battery life on the left half;
no battery-runtime or distance improvement is guaranteed. Larger queues use
additional RAM and help with short processing stalls, not sustained radio
outages. Disabling 2 Mbps mode follows the radio-mode change in
[ZMK's connection-stability option](https://zmk.dev/docs/config/bluetooth),
without enabling its other experimental changes. See also
[ZMK's weak-connection guidance](https://zmk.dev/docs/troubleshooting/connection-issues#unreliableweak-connection).

ZMK and the PS/2 driver are pinned to the existing compatible upstream commits;
this change does not migrate the TrackPoint to a newer ZMK pointing stack.
The reusable GitHub build workflow is also pinned to support this fork's
Zephyr 3.5 board model. Newer workflow revisions assume newer board metadata
and can fail after compilation, before uploading the firmware.

### Flash and compare

1. Download the `firmware` artifact from a successful GitHub Actions build of
   this revision. Keep a copy of your known-working left and right firmware.
2. Flash `seahorse_v2_left-nice_nano_v2-zmk.uf2` to the **left** half and
   `seahorse_v2_right-nice_nano_v2-zmk.uf2` to the **right** half, using each
   controller's bootloader. Both halves need the update. These filenames are
   the defaults from the checked-in build matrix.
3. Start with the existing bonds. A settings reset is not normally needed for
   radio and queue changes. The `settings_reset` image erases saved settings;
   use it only if following ZMK's pairing-recovery instructions.
4. Keep the right half connected to the computer over USB and select USB output
   if necessary. At a fixed separation, compare left-only typing with the old
   firmware, then the new firmware. Include quick taps, chords, and held keys
   followed by releases. Check for missing, repeated, or stuck keys, both while
   the TrackPoint is idle and while it is moving.
5. Repeat at increasing distances with the same orientation and surroundings,
   and check reconnection after briefly blocking the link. Record left-half
   runtime through a normal charge cycle before judging the battery tradeoff.

Build success confirms firmware compatibility, not physical wireless range.
If distance-dependent drops remain, compare antenna orientation and clearance
from batteries, metal mounts, and nearby USB 3 equipment. Radio configuration
cannot repair an obstructed or damaged antenna. A failure only during TrackPoint
motion also warrants checking PS/2 interrupt load and wiring interference.
