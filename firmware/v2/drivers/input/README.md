# PS/2 TrackPoint startup reset correction

`input_mouse_ps2.c` is copied from
[`badjeff/kb_zmk_ps2_mouse_trackpoint_driver`](https://github.com/badjeff/kb_zmk_ps2_mouse_trackpoint_driver/blob/05b309d995bf9a7edcdc9cf641d5a2f20ce83fa1/src/drivers/input/input_mouse_ps2.c)
at commit `05b309d995bf9a7edcdc9cf641d5a2f20ce83fa1`. Its original copyright
and Apache-2.0 SPDX header are retained; the license is in `Apache-2.0.txt`.

The reset correction changes three `DT_INST_NODE_HAS_PROP(n, rst_gpio)` checks
to `rst_gpios`, matching the actual `rst-gpios` binding/property. The old
checks selected a null reset port even though the Seahorse config supplied
one, so the power-on-reset function returned without driving the pin.

The packet timeout is also initialized in the device initializer, before
starting the initialization thread. Previously it was initialized after
enabling callbacks/reporting, leaving a window where an early byte could
cancel or schedule an uninitialized work item.

The v2 module runs after the pinned PS/2 module and replaces only that input
source in the application target. CMake fails if the expected upstream source
is absent. Transport, settings, behaviors, reset polarity/duration, UART pins,
report rate, sensitivity, Bluetooth, Studio, and the display are retained.
This remains a native Zephyr C driver; no conversion helpers are added.

When updating the upstream driver, compare this file with its new version and
review whether the override is still needed. Standalone v2 exports include
the correction and module ordering.

Validation requires both firmware builds, verification that the local input
source compiled in the right image, and repeated physical cold starts on the
affected keyboard. A build proves reset support is present, not that a
particular sensor/wiring starts reliably on every power cycle.
