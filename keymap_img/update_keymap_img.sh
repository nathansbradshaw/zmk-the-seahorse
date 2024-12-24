#!/bin/sh

echo 'Parsing ZMK keymap for Ferris Sweep...'
keymap -c keymap_config.yaml parse -z ../config/seahorse_v2.keymap > keymap.yaml

echo '\n\nDrawing keymap for Ferris Sweep...'
keymap -c keymap_config.yaml draw --qmk-keyboard ferris/sweep --qmk-layout LAYOUT_ferris_sweep keymap.yaml > keymap.svg

echo 'Keymap SVG for Ferris Sweep has been generated as keymap.svg'
