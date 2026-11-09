# Firmware generations

## Layout standard

Keep each hardware generation in `firmware/vN/` as a standalone ZMK project:

```text
firmware/vN/
  README.md
  build.yaml
  config/west.yml
  boards/shields/<keyboard>/
  zephyr/module.yml
  .github/workflows/build.yml
```

Keep keymaps, layouts, pin assignments, processing rules, build matrices, and
dependencies inside that generation. A profile's `zephyr/module.yml` uses
`board_root: .`, so the profile works after export without the parent repo.
Only the repository-root module wrapper selects `firmware/v2` for the default
build. Moving development to another generation should be an explicit change
to these root entry points after its hardware and firmware are validated.
The root automatic workflow watches only v2 and its build entry points, so
documentation, archived v1, and experimental v3 edits do not spend v2 CI time.
Manual dispatch remains available for the default v2 build.

Released profiles pin compatible dependencies and their reusable build
workflow. Legacy snapshots and experimental profiles retain their original
dependencies until a separate, verified update is made; their build status
must be stated rather than inferred from the active v2 build.

## Source provenance

| Profile | Imported source | Validation scope |
| --- | --- | --- |
| v1 | `v1.1.0`, commit `e953e2302b30b04ecb7fde623354948a98350217` | Historical source retained; not rebuilt in this consolidation |
| v2 | `zmk-seahorse-config/main`, commit `419d09b184cee938bf4dffc09b12a96de93c367b`, plus the ZMK upgrade and Bluetooth changes | Active two-half firmware build; device acceptance is separate |
| v3 | `zmk-seahorse-config/loce_alda`, commit `11092ff0062122e227b5a58070b4332ba5865b23` | Loce Alda source retained as experimental; not built or hardware-validated here |

Both config-repository branches are merged into this Git history. The v1
release tags and the older firmware history are retained. No force-push or
history rewrite is needed to recover an earlier generation.

The v3 profile contains the Loce Alda shields and its own build rows. The
source branch's extra v2 build rows and duplicated v2 shields are excluded,
so development inside v3 cannot accidentally select a stale v2 keymap.

## Export one generation

From this repository, create a new empty destination outside the checkout
and export the desired profile. For example:

```sh
mkdir /private/tmp/seahorse-v1-export
git archive HEAD:firmware/v1 | tar -x -C /private/tmp/seahorse-v1-export
```

Replace `v1` with `v2` or `v3` as needed. The result has a normal standalone
ZMK root; no parent-repository symlinks are required. Its build matrix and
manifest travel with it. If published as its own repository, its included
workflow appears at the normal `.github/workflows` location.

For the exact historical v1 tree, including its original README, use:

```sh
mkdir /private/tmp/seahorse-v1-original
git archive v1.1.0 | tar -x -C /private/tmp/seahorse-v1-original
```

These commands export source, not a promise that historical floating
dependencies still build. Check the profile's README before building or
flashing. Preserve saved settings and known-working firmware when changing
generations. Never flash one generation's image to another generation's
hardware just because the controller type matches.
