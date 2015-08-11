imgcat
======

Outputs ASCII art when given an image as input, for viewing images on the command line.

## Options
* `-w NUM`: Sets the number of characters wide the output will be.
* `FILE`: The last argument that is not another option will be used as the input file.
Otherwise it will read from stdin.

## Issues
* Only supports 24-bit BMP files, JPEGs, and PNGs. More formats might be added if I have time.
* Low contrast images don't show up well, though this is a limitation of the ASCII art
medium itself. A different ASCII palette might help.
* I wrote this for fun in a few hours with very little prior experience using C.
