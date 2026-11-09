#!/bin/sh

echo 'Parsing ZMK keymap for Seahorse V2...'
keymap -c keymap_config.yaml parse \
  -z ../boards/shields/seahorse_v2/seahorse_v2.keymap > keymap.yaml

echo '\n\nDrawing keymap for Seahorse V2...'
keymap -c keymap_config.yaml draw \
  --qmk-keyboard ferris/sweep \
  --qmk-layout LAYOUT_split_3x5_2 \
  keymap.yaml > keymap.svg

echo 'Keymap SVG for Seahorse V2 has been generated as keymap.svg'
