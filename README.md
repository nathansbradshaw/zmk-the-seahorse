# zmk-the-seahorse
Firmware for the sea horse pcb based on the Ferris Sweep
The board can be found here: https://github.com/nathansbradshaw/the-seahorse

V2 has a trackpoint and therefore needs special setup if building locally

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