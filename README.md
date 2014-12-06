imgcat
======

Outputs ASCII art when given an image as input, for viewing images on the command line.
Similar in use to "cat" for text files. The use of the name "cat" here isn't correct, as
it doesn't concatenate anything; it just has a similar use case. When you want to quickly
view the contents of a text file, you use "cat". When you want to view the contents of an
image, you use "imgcat". This could be used when you're connected remotely and don't have
a way to view images graphically.

## Issues
* Only supports 24-bit BMP files, and nothing complicated like palettes. More formats
might be added if I have time.
* Low contrast images don't show up well, though this is a limitation of the ASCII art
medium itself. A different ASCII palette might help.
* I wrote this for fun in a few hours with very little prior experience using C.
