# Seahorse firmware

One maintained firmware repository for the Seahorse family. The former
`zmk-seahorse-config` history is merged here; build and develop firmware here.
The [hardware repository](https://github.com/nathansbradshaw/the-seahorse)
continues to hold PCB and mechanical designs.

| Generation | Project | Status | Source and instructions |
| --- | --- | --- | --- |
| v1 | Original Seahorse | Frozen legacy v1.1.0; no active users | [firmware/v1](firmware/v1/README.md) |
| v2 | Seahorse v2.1.0 | Active/default build, Studio + TrackPoint + nice!view | [firmware/v2](firmware/v2/README.md) |
| v3 | Loce Alda | Experimental development | [firmware/v3](firmware/v3/README.md) |

Each generation is a standalone ZMK project containing its own `boards`,
`config/west.yml`, `build.yaml`, `zephyr/module.yml`, and build workflow.
See the [version guide](docs/firmware-versions.md) for source provenance,
export commands, and the standard for future generations.

## Default build

Run **Build ZMK firmware** in GitHub Actions. It builds the active **v2** left
and right halves, including Studio and the right-side nice!view gem screen.
The root `config` and `build.yaml` links select `firmware/v2`; the root module
metadata points Zephyr at that version's boards. These entry points preserve
the existing ZMK build workflow while keeping versioned source separate.

Use the instructions in [firmware/v2](firmware/v2/README.md) to flash and test.
The current v2 keymap, its Studio unlock key, and all eight populated plus two
reserved layers come from the actual v2.1.0 config. Shared radio settings are
now loaded from the active config, with the Bluetooth queue/latency tuning.

Legacy v1 and experimental v3 are excluded from the default build. Their
source and histories remain available for independent use and development.
