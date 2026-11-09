# Right-half idle power and reliability

The user prioritizes runtime of the TrackPoint/right half while retaining
immediate TrackPoint wake. This batch preserves sensor power, UART reception,
80 Hz reporting, +8 dBm, 1M PHY, split latency 4, queue sizes, and Studio.

## Changes

- On USB, display updates continue through idle so the full Bad Apple clip
  can finish. Playback remains once per USB connection.
- On battery, ZMK's normal 30-second inactivity event stops the 10 ms
  periodic display task. The nice!view remains powered with its last picture.
- Battery/connection/layer canvas changes request a one-shot display flush
  while idle. Key or pointer activity restarts periodic updates immediately.
  The sensor, receiver, BLE link, and key scanning are not suspended.
- WPM collection is disabled because this screen does not render a WPM widget.
- The TrackPoint packet timeout work item is initialized before the driver
  can enable its callback. Previously the callback could receive a byte and
  touch that work item before it was initialized.

The power decision is in Rust (`widgets/display_power.rs`); the existing
Zephyr C/LVGL interface applies it. The adapter controls `display_timer`
exported by the pinned ZMK v0.3 display implementation and uses a dedicated
one-shot LVGL flush for status changes.
Review this adapter when updating ZMK. All UI changes remain on its dedicated
display work queue. The LCD driver and its COM-inversion arrangement are
unchanged; only LVGL polling stops when there is nothing to redraw.

This removes 100 periodic display-task submissions per idle second on battery,
plus unused WPM work. It is not a measured reduction in total current or an
equivalent multiplier for battery runtime. Radio/controller and sensor power
can dominate the remaining load.

## Audit findings and limits

The compiled baseline already has `CONFIG_UART_0_NRF_ASYNC_LOW_POWER=y`.
Enabling that same flag again would not establish a new saving. The PS/2
receiver remains enabled for immediate input; safely suspending it would
require proving wake timing and first-packet integrity on hardware.

The v2 PCB connects right TrackPoint connector J6 pin 1 and the controller VCC
pin to the same `vcc_r` net. Turning off that supply is incompatible with the
requested immediate sensor wake unless the specific sensor/wake design is
validated. No external-power switching or sensor sleep commands are added.

The sensor's actual model and idle current remain unverified. A current
measurement at rest and while pointing is needed to rank native sensor idle
modes against controller/radio costs. Keep active report rate and the known
working radio settings fixed for that comparison.

## Verification

Native tests cover USB idle through the video duration, completion without
activity-triggered replay, battery idle, pointer/key wake, and USB replug,
alongside full video decoding and corrupt-stream handling. Firmware CI must
show WPM collection disabled, the local corrected input driver and power
adapter compiled, and the existing pointing/Studio/radio settings retained.

Hardware acceptance must check:

1. Startup typing, TrackPoint, and status widgets, including movement during
   startup and the previous three-cold-start check.
2. USB idle for the complete clip: no premature pause and no looping.
3. Right half on battery and connected to a host: after at least 30 seconds
   without input, the first pointer movement and first key still work.
4. Battery/connection status updates remain visible while idle; reconnecting
   USB rearms the video exactly once.

Actual current and full-discharge runtime measurements remain separate from
the scheduling and functional checks above.
