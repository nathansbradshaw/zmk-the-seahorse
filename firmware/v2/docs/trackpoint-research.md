# TrackPoint feel and right-half battery research

Research date: 2026-11-09. These are options to evaluate, not changes to the
working TrackPoint configuration. Distinguish published experiments,
manufacturer specifications, community measurements, and our own source
inspection. None establishes the current Seahorse's measured power use.

## Making the TrackPoint easier to use

| Idea | Evidence and implication | Seahorse next experiment |
| --- | --- | --- |
| Tune negative inertia | IBM's [CHI 1995 experiment](https://chi1995.acm.org/proceedings/shortppr/rcb_bdy.htm) reported a 7.8 ± 3.5% pointing-performance improvement. The filter emphasizes changes in force, helping starts and stops without simply increasing cruising speed. Nine participants supplied quantitative results; this is evidence for the technique, not a predicted gain on our hardware. | The driver exposes `tp-neg-inertia`; our config leaves it at the device default. Compare a few values against the baseline before changing sensitivity. |
| Treat sensitivity and maximum speed separately | The [driver's tuning interface](https://github.com/badjeff/kb_zmk_ps2_mouse_trackpoint_driver/tree/05b309d995bf9a7edcdc9cf641d5a2f20ce83fa1) supports sensitivity, negative inertia, and upper plateau speed. | Current sensitivity is 100; `tp-val6-upper-speed` is not overridden. For less force, trial a modest sensitivity increase while preserving controllable small-target motion. Keep host pointer settings fixed during comparison. |
| Separate precision and scrolling modes | [ZMK input processors](https://zmk.dev/docs/keymaps/input-processors) support scaling, scrolling, and automatic mouse layers. This is a practical configuration technique rather than a controlled usability study. | Keep normal movement quick, use a held precision mode for small targets, and use a separate held scroll mode. Current configuration warrants an audit before adding more tuning; see below. |
| Check cap height, grip, and mounting | [infused-kim's TrackPoint extension project](https://github.com/infused-kim/kb_trackpoint_extension) provides adjustable extension geometry for ergonomic boards. It demonstrates community hardware work, not a measured usability gain. Mechanically, a taller lever can reduce required finger force for a given bending moment. | Check cap-to-key clearance, looseness, finger contact, and mounting flex. A cap or height change may help more than software gain, but excessive height can magnify accidental inputs. |
| Avoid indiscriminate smoothing | In the same IBM study, smoothing increased perceived sluggishness; excessive directional amplification caused weaving and small-motion jumps. | Avoid adding averaging as a universal fix. First identify whether the actual problem is jitter, effort, overshoot, or cursor drift. |

### A source finding worth testing separately

In `boards/shields/seahorse_v2/seahorse_v2_tpoint0_ps2.dtsi`, the `precise`
and `scroll` overrides both match `MOUSE_TP`. Precision is declared first.
[ZMK's documented rule](https://zmk.dev/docs/keymaps/input-processors/usage)
is that the first matching override wins unless `process-next` is set. The
pinned [ZMK v0.3 implementation](https://github.com/zmkfirmware/zmk/blob/edf5c0814fd3ea202e43aad2d68fd32e882a518c/app/src/pointing/input_listener.c)
confirms that early-return behavior. This suggests the later scroll override
can be unreachable. Both
overrides also bypass the parent's temporary-layer processor by default.
This is an audit finding, not a claim that user-confirmed pointing is broken.
No input processors or key bindings are changed by the display update.

## Better battery life on the TrackPoint half

Prioritize the sensor and receiver over the already low-power screen.
The [driver documentation](https://github.com/badjeff/kb_zmk_ps2_mouse_trackpoint_driver/tree/05b309d995bf9a7edcdc9cf641d5a2f20ce83fa1#43-if-you-get-bad-battery-life)
summarizes community measurements around 3–3.85 mA for TrackPoints. Those
reports are not measurements of this keyboard. For comparison, the
[nice!view manufacturer](https://nicekeyboards.com/nice-view/) specifies
about 10 µA typical display consumption. MCU rendering and SPI traffic are
additional costs, so that figure is not the complete cost of animation.

| Option | Expected benefit and trade-off | What is known here |
| --- | --- | --- |
| Verify the sensor's own idle mode | Potentially substantial savings while retaining movement wakeup, if this exact controller supports it. | [Sprintek's SK8707-06](https://www.sprintek.com/en/products/pointing_stick/SK8707-06.aspx) specifies 890 µA idle versus 3.21 mA operating. That is one model's specification; the Seahorse sensor identity and idle behavior must be checked before applying any command or promising savings. |
| Suspend the PS/2 receiver safely during inactivity | May save controller power as well as sensor power. Reliable wake and restoring partial PS/2 packets are necessary. | The pinned [`ps2_uart.c`](https://github.com/badjeff/kb_zmk_ps2_mouse_trackpoint_driver/blob/05b309d995bf9a7edcdc9cf641d5a2f20ce83fa1/src/drivers/ps2/ps2_uart.c) enables RX during initialization and has temporary RX-off code for command transmission, but no general activity-driven power-management path. The input driver likewise has no PM callback. Extra idle current is an engineering suspicion to measure, not a known amount. |
| Cut sensor power during deep sleep | Potentially the largest idle saving, at the cost of losing TrackPoint wake until a key wakes the board. | Requires a controllable sensor power rail/load switch and correct pin states to avoid back-powering over data/clock. Then restore power, reset, and settings after wake. Firmware alone cannot switch an unswitchable supply. Do not assume `RST` or disabling reports removes sensor power. |
| Keep diagnostics off | Avoids unnecessary logging and USB/CPU work. | Check generated `.config`, not just source fragments. The README's “disable logging” example misleadingly shows `CONFIG_ZMK_USB_LOGGING=y`; that enables logging. USB CDC used by Studio is a separate feature and must remain enabled. |
| Trial a lower report rate on battery | Could reduce event-processing and radio work while moving; it does not imply proportional sensor savings. | Current rate is 80 Hz (12.5 ms). Trying 60 Hz (16.7 ms) is a reasonable A/B experiment; 40 Hz (25 ms) may feel less smooth. Do not lower the rate just to claim a battery improvement. |
| Idle blanking / stop continuous animation | Reduces redraw work, but may be a smaller win than sensor/receiver changes. | Existing display configuration leaves idle blanking disabled. This update runs Bad Apple only with USB power and pauses its timer after one play or on unplug. It does not change the existing global display policy. |

[ZMK's sleep and external-power settings](https://zmk.dev/docs/config/power)
help only when the board wiring and peripheral drivers participate in power
management. Deep sleep is distinct from idle display blanking. The left
half's new latency setting affects its own listening schedule; loosening it
would undo part of the Bluetooth reliability work rather than target the
TrackPoint's power cost. Leave radio tuning unchanged while investigating.

## Useful measurement order

1. Identify the TrackPoint controller/model and inspect its actual supply
   wiring. Record the right battery's capacity and condition.
2. With the right half on battery, measure current at rest, while pointing,
   and while typing; keep the BLE host and left-half connection consistent.
3. Compare a separate experimental build with the sensor rail physically
   isolated, then receiver-only changes, to separate sensor and MCU costs.
   Do not disconnect powered signal wires ad hoc.
4. Measure idle-mode and wake behavior before enabling power management in
   the normal build. Check the first cursor movement, held keys, reconnect,
   Studio, and TrackPoint initialization after wake.
5. Estimate runtime from measured average current, then verify a real full
   discharge cycle. Battery percentage alone is too coarse for short tests.

For perspective only: a 500 mAh battery divided by 3.5 mA is about 143 hours
for that load alone, before controller/radio costs, conversion loss, usable
capacity, and battery aging. This is arithmetic, not a Seahorse runtime claim.
