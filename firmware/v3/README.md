# Seahorse v3 — Loce Alda

Experimental firmware imported from `zmk-seahorse-config/loce_alda`, commit
`11092ff0062122e227b5a58070b4332ba5865b23`. Loce Alda is the v3 project name;
this source is a starting point for ongoing hardware/firmware development.
The profile name `v3` identifies the Seahorse hardware generation; the existing
`LoceAlda v1.0.0` device label is its separate project/firmware label and is
retained without inventing a new release.

The Loce Alda shields, keymap, layout, and input configuration are retained.
The build matrix contains only its left/right XIAO BLE targets; the source
branch's unrelated v2 build rows and v2 shield copies are excluded. The
original manifest and workflow are retained without claiming compatibility
with today's upstream revisions.

This is a standalone ZMK project, following the
[version layout standard](https://github.com/nathansbradshaw/zmk-the-seahorse/blob/main/docs/firmware-versions.md). It is excluded from
the repository's default v2 build and has not been built or hardware-validated
as part of this consolidation. Pin dependencies and verify the combined
features before promoting it to a released profile or default build.
